#include "DX11RenderContext.h"
#include "imgui_impl_dx11.h"

#define TEX_BINGING_REC 0
#define INDIRECT_DRAW 0

DX11RenderContext::DX11RenderContext(DX11Context& context, RenderContextDesc& desc)
    : IRenderContext(desc), dxContext(context)
{
}

void DX11RenderContext::init()
{
	if (FAILED(deviceContext->QueryInterface<ID3D11DeviceContext4>(&deviceContext4)))
		throw runtime_error("ID3D11DeviceContext4 not support");
	if (FAILED(dxContext.device5->CreateFence(fenceValue, D3D11_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
		throw runtime_error("Fence create failed");
}

void DX11RenderContext::reset()
{
	clearSRV();
	clearUAV();
	clearRTV();
}

void DX11RenderContext::release()
{
	if (drawInfoBuf)
		drawInfoBuf.Reset();
	if (fence)
		fence.Reset();
	if (deviceContext4)
		deviceContext4.Reset();
    if (deviceContext)
        deviceContext.Reset();
}

void* DX11RenderContext::getDeviceHandle() const
{
	return dxContext.device.Get();
}

void DX11RenderContext::setGPUSignal()
{
	fenceValue++;

	deviceContext4->Signal(fence.Get(), fenceValue);
}

void DX11RenderContext::waitSignalGPU()
{
	deviceContext4->Wait(fence.Get(), fenceValue);
}

void DX11RenderContext::waitSignalCPU()
{
	if (fence->GetCompletedValue() < fenceValue)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		fence->SetEventOnCompletion(fenceValue, eventHandle);
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void DX11RenderContext::clearVertexBindings()
{
	ID3D11Buffer* buffers[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT] = { NULL };
	unsigned int offsets[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT] = { 0 };
	unsigned int strides[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT] = { 0 };
	deviceContext->IASetVertexBuffers(0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT, buffers, strides, offsets);
}

void bindCBToStage(ComPtr<ID3D11DeviceContext> deviceContext, ShaderProgram* program, ComPtr<ID3D11Buffer> buffer, unsigned int index)
{
	for (auto b = program->shaderStages.begin(), e = program->shaderStages.end(); b != e; b++) {
		switch (b->first)
		{
		case Vertex_Shader_Stage:
			deviceContext->VSSetConstantBuffers(index, 1, buffer.GetAddressOf());
			break;
		case Fragment_Shader_Stage:
			deviceContext->PSSetConstantBuffers(index, 1, buffer.GetAddressOf());
			break;
		case Geometry_Shader_Stage:
			deviceContext->GSSetConstantBuffers(index, 1, buffer.GetAddressOf());
			break;
		case Compute_Shader_Stage:
			deviceContext->CSSetConstantBuffers(index, 1, buffer.GetAddressOf());
			break;
		case Tessellation_Control_Shader_Stage:
			deviceContext->HSSetConstantBuffers(index, 1, buffer.GetAddressOf());
			break;
		case Tessellation_Evalution_Shader_Stage:
			deviceContext->DSSetConstantBuffers(index, 1, buffer.GetAddressOf());
			break;
		}
	}
}

void bindSRToStage(ComPtr<ID3D11DeviceContext> deviceContext, ShaderProgram* program, ComPtr<ID3D11ShaderResourceView> srv, unsigned int index)
{
	for (auto b = program->shaderStages.begin(), e = program->shaderStages.end(); b != e; b++) {
		switch (b->first)
		{
		case Vertex_Shader_Stage:
			deviceContext->VSSetShaderResources(index, 1, srv.GetAddressOf());
			break;
		case Fragment_Shader_Stage:
			deviceContext->PSSetShaderResources(index, 1, srv.GetAddressOf());
			break;
		case Geometry_Shader_Stage:
			deviceContext->GSSetShaderResources(index, 1, srv.GetAddressOf());
			break;
		case Compute_Shader_Stage:
			deviceContext->CSSetShaderResources(index, 1, srv.GetAddressOf());
			break;
		case Tessellation_Control_Shader_Stage:
			deviceContext->HSSetShaderResources(index, 1, srv.GetAddressOf());
			break;
		case Tessellation_Evalution_Shader_Stage:
			deviceContext->DSSetShaderResources(index, 1, srv.GetAddressOf());
			break;
		}
	}
}

void bindUAToStage(ComPtr<ID3D11DeviceContext> deviceContext, ShaderProgram* program, ComPtr<ID3D11UnorderedAccessView> uav, unsigned int index)
{
	if (program->isComputable())
		deviceContext->CSSetUnorderedAccessViews(index, 1, uav.GetAddressOf(), NULL);
	else
		deviceContext->OMSetRenderTargetsAndUnorderedAccessViews(D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL,
			NULL, NULL, index, 1, uav.GetAddressOf(), NULL);
}

unsigned int DX11RenderContext::bindBufferBase(IGPUBuffer* buffer, const ShaderPropertyName& name, BufferOption bufferOption)
{
	if (currentProgram == NULL)
		return 0;
	DX11GPUBuffer* dxBuffer = dynamic_cast<DX11GPUBuffer*>(buffer);
	if (dxBuffer == NULL || dxBuffer->dx11Buffer == NULL)
		return 0;
	if (bufferOption.output && dxBuffer->desc.gpuAccess != GAF_ReadWrite)
		throw runtime_error("Buffer with GAF_Read cannot be binded on output");
	if (bufferOption.output) {
		currentProgram->bindUAV(deviceContext, name, dxBuffer->dx11BufferUAV);
	}
	else {
		switch (dxBuffer->desc.type)
		{
		case GB_Constant:
			currentProgram->bindCBV(deviceContext, name, dxBuffer->dx11Buffer);
			break;
		case GB_Storage:
		case GB_Command:
			currentProgram->bindSRV(deviceContext, name, dxBuffer->dx11BufferSRV);
			break;
		default:
			throw runtime_error("Unknown Error");
			break;
		}
	}
	return dxBuffer->desc.id;
}

bool DX11RenderContext::unbindBufferBase(const ShaderPropertyName& name)
{
	if (currentProgram == NULL)
		return 0;
	return currentProgram->unbindBuffer(deviceContext, name);
}

unsigned int DX11RenderContext::bindBufferBase(IGPUBuffer* buffer, unsigned int index, BufferOption bufferOption)
{
	DX11GPUBuffer* dxBuffer = dynamic_cast<DX11GPUBuffer*>(buffer);
	if (dxBuffer == NULL || dxBuffer->dx11Buffer == NULL)
		return 0;
	if (bufferOption.output && dxBuffer->desc.gpuAccess != GAF_ReadWrite)
		throw runtime_error("Buffer with GAF_Read cannot be binded on output");
	if (bufferOption.output) {
		bindUAToStage(deviceContext, currentProgram, dxBuffer->dx11BufferUAV, index);
	}
	else {
		switch (dxBuffer->desc.type)
		{
		case GB_Constant:
			bindCBToStage(deviceContext, currentProgram, dxBuffer->dx11Buffer, index);
			break;
		case GB_Vertex:
		{
			unsigned int stride = bufferOption.stride == -1 ? dxBuffer->desc.cellSize : bufferOption.stride;
			unsigned int offset = bufferOption.offset;
			deviceContext->IASetVertexBuffers(index, 1, dxBuffer->dx11Buffer.GetAddressOf(), &stride, &offset);
			break;
		}
		case GB_Index:
		{
			unsigned int stride = sizeof(unsigned int);
			unsigned int offset = 0;
			deviceContext->IASetIndexBuffer(dxBuffer->dx11Buffer.Get(), DXGI_FORMAT_R32_UINT, offset);
			break;
		}
		case GB_Storage:
		case GB_Command:
			bindSRToStage(deviceContext, currentProgram, dxBuffer->dx11BufferSRV, index);
			break;
		default:
			throw runtime_error("Unknown Error");
			break;
		}
	}
	return dxBuffer->desc.id;
}

unsigned int DX11RenderContext::uploadBufferData(IGPUBuffer* buffer, unsigned int size, void* data)
{
	DX11GPUBuffer* dxBuffer = dynamic_cast<DX11GPUBuffer*>(buffer);
	if (dxBuffer == NULL)
		return 0;
	if (dxBuffer->desc.size != size)
		throw runtime_error("Buffer resize not allowed");
	D3D11_MAPPED_SUBRESOURCE mpd;
	deviceContext->Map(dxBuffer->dx11Buffer.Get(), 0, dxBuffer->desc.type == GB_Constant ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mpd);
	memcpy_s((char*)mpd.pData, size * dxBuffer->desc.cellSize, data, size * dxBuffer->desc.cellSize);
	deviceContext->Unmap(dxBuffer->dx11Buffer.Get(), 0);
	return dxBuffer->desc.id;
}

unsigned int DX11RenderContext::uploadBufferSubData(IGPUBuffer* buffer, unsigned int first, unsigned int size, void* data)
{
	DX11GPUBuffer* dxBuffer = dynamic_cast<DX11GPUBuffer*>(buffer);
	if (dxBuffer == NULL)
		return 0;
	if (size == 0)
		return dxBuffer->desc.id;
	D3D11_MAPPED_SUBRESOURCE mpd;
	deviceContext->Map(dxBuffer->dx11Buffer.Get(), 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mpd);
	memcpy_s((char*)mpd.pData + first * dxBuffer->desc.cellSize, size * dxBuffer->desc.cellSize, data, size * dxBuffer->desc.cellSize);
	deviceContext->Unmap(dxBuffer->dx11Buffer.Get(), 0);
	return dxBuffer->desc.id;
}

void DX11RenderContext::copyBufferData(IGPUBuffer* srcBuffer, IGPUBuffer* dstBuffer)
{
	DX11GPUBuffer* srcDxBuffer = dynamic_cast<DX11GPUBuffer*>(srcBuffer);
	DX11GPUBuffer* dstDxBuffer = dynamic_cast<DX11GPUBuffer*>(dstBuffer);
	if (srcDxBuffer && dstDxBuffer)
		deviceContext->CopyResource(dstDxBuffer->dx11Buffer.Get(), srcDxBuffer->dx11Buffer.Get());
}

void DX11RenderContext::copyBufferSubData(IGPUBuffer* srcBuffer, unsigned int srcFirst, IGPUBuffer* dstBuffer, unsigned int dstFirst, unsigned int size)
{
	DX11GPUBuffer* srcDxBuffer = dynamic_cast<DX11GPUBuffer*>(srcBuffer);
	DX11GPUBuffer* dstDxBuffer = dynamic_cast<DX11GPUBuffer*>(dstBuffer);
	if (srcBuffer->desc.cellSize != dstBuffer->desc.cellSize)
		throw runtime_error("Data format is not the same");
	int cellSize = srcBuffer->desc.cellSize;
	if (srcDxBuffer && dstDxBuffer) {
		D3D11_BOX srcBox;
		srcBox.left = srcFirst * cellSize;
		srcBox.right = (srcFirst + size) * cellSize;
		srcBox.top = 0;
		srcBox.bottom = 1;
		srcBox.front = 0;
		srcBox.back = 1;
		dxContext.deviceContext->CopySubresourceRegion(
			dstDxBuffer->dx11Buffer.Get(), 0, dstFirst * cellSize,
			0, 0, srcDxBuffer->dx11Buffer.Get(), 0, &srcBox);
	}
}

unsigned int DX11RenderContext::bindFrame(IRenderTarget* target)
{
	DX11RenderTarget* dxTarget = dynamic_cast<DX11RenderTarget*>(target);
	if (dxTarget == NULL)
		return 0;

	if (!dxTarget->desc.inited)
		return 0;

	if (currentRenderTarget == dxTarget)
		return dxTarget->desc.frameID;

	DX11Texture2D* depthTex = NULL;
	if (dxTarget->desc.depthOnly)
		depthTex = (DX11Texture2D*)dxTarget->desc.depthTexure->getVendorTexture();
	else if (dxTarget->desc.withDepthStencil) {
		if (dxTarget->desc.multisampleLevel > 1)
			depthTex = dxTarget->multisampleDepthTex;
		else
			depthTex = dxTarget->dx11DepthTex;
	}
	else
		depthTex = NULL;

	if (depthTex) {
#if TEX_BINGING_REC
		if (canBindRTV(depthTex))
			dx11DSV = depthTex->getDSV(0);
		else
			throw runtime_error("Texture is still binded as SRV or UAV");
#else
		dx11DSV = depthTex->getDSV(0);
#endif
	}
	else
		dx11DSV = nullptr;

	dx11RTVs.resize(dxTarget->desc.textureList.size());
	bool isMs = dxTarget->desc.multisampleLevel > 1;
	for (int i = 0; i < dxTarget->desc.textureList.size(); i++) {
		RTInfo& rtInfo = dxTarget->desc.textureList[i];
		DX11Texture2D* tex = NULL;
		if (isMs) {
			DX11RenderTarget::MSTex& mstex = dxTarget->multisampleTexs[i];
			tex = mstex.tex;
		}
		else {
			tex = (DX11Texture2D*)rtInfo.texture->getVendorTexture();
		}
		RTOption option;
		option.mipLevel = rtInfo.mipLevel;
		option.arrayBase = rtInfo.arrayBase;
		option.arrayCount = rtInfo.arrayCount;
		option.multisample = isMs;
#if TEX_BINGING_REC
		if (canBindRTV(tex))
			dx11RTVs[i] = tex->getRTV(option);
		else
			throw runtime_error("Texture is still binded as SRV or UAV");
#else
		dx11RTVs[i] = tex->getRTV(option);
#endif
	}
	
	deviceContext->OMSetRenderTargets(dx11RTVs.size(),
		(ID3D11RenderTargetView* const*)dx11RTVs.data(), dx11DSV.Get());
	currentRenderTarget = dxTarget;
	currentDeviceSurface = NULL;
	return dxTarget->desc.frameID;
}

void DX11RenderContext::bindSurface(IDeviceSurface* surface)
{
	DX11DeviceSurface* dxSurface = dynamic_cast<DX11DeviceSurface*>(surface);
	if (dxSurface == NULL)
		return;
	if (!dxSurface->desc.inited)
		return;
	/*if (currentDeviceSurface == dxSurface)
		return;*/
	dxContext.deviceContext->OMSetRenderTargets(1, dxSurface->backBufferRTV.GetAddressOf(), NULL);
	currentDeviceSurface = dxSurface;
	currentRenderTarget = NULL;
}

void DX11RenderContext::clearOutputBufferBindings()
{
	clearUAV();
}

void DX11RenderContext::clearFrameBindings()
{
	clearRTV();
	dx11RTVs.clear();
	dx11DSV = nullptr;
}

void DX11RenderContext::resolveMultisampleFrame(IRenderTarget* target)
{
	DX11RenderTarget* dxTarget = dynamic_cast<DX11RenderTarget*>(target);
	if (dxTarget == NULL)
		return;
	if (dxTarget->desc.multisampleLevel > 1) {
		for (int i = 0; i < dxTarget->desc.textureList.size(); i++) {
			DX11Texture2D* tex = (DX11Texture2D*)dxTarget->desc.textureList[i].texture->getVendorTexture();
			if (tex == NULL || tex->dx11Texture2D == NULL)
				throw runtime_error("Texture is not ready");
			deviceContext->ResolveSubresource(tex->dx11Texture2D.Get(), 0,
				dxTarget->multisampleTexs[i].tex->dx11Texture2D.Get(), 0, tex->info.texture2DDesc.Format);
		}
		if (dxTarget->desc.withDepthStencil) {
			/*dxContext.deviceContext->ResolveSubresource(dx11DepthTex->dx11Texture2D, 0,
				multisampleDepthTex->dx11Texture2D, 0, DXGI_FORMAT_D32_FLOAT);*/
			bindShaderProgram(DX11RenderTarget::depthBlitCSShader);
			auto srv = dxTarget->multisampleDepthTex->getSRV();
			RWOption rwOption;
			rwOption.mipLevel = 0;
			auto uav = dxTarget->dx11DepthTex->getUAV(rwOption);
			clearSRV();
			clearUAV();
			clearRTV();
			deviceContext->CSSetShaderResources(7, 1, srv.GetAddressOf());
			deviceContext->CSSetUnorderedAccessViews(0, 1, uav.GetAddressOf(), NULL);
			dispatchCompute(ceil(dxTarget->desc.width / 16.0f), ceil(dxTarget->desc.height / 16.0f), 1);
			clearUAV();
		}
	}
	clearRTV();
}

void DX11RenderContext::clearFrameColor(const Color& color)
{
	if (dx11RTVs.empty())
		return;
	auto rt = dx11RTVs.front();
	if (rt != NULL)
		deviceContext->ClearRenderTargetView(rt.Get(), (const float*)&color);
}

void DX11RenderContext::clearSurfaceColor(const Color& color)
{
	if (currentDeviceSurface == NULL || currentDeviceSurface->backBufferRTV == NULL)
		return;
	deviceContext->ClearRenderTargetView(currentDeviceSurface->backBufferRTV.Get(), (const float*)&color);
}

void DX11RenderContext::clearFrameColors(const vector<Color>& colors)
{
	if (colors.empty())
		return;
	for (int i = 0; i < dx11RTVs.size(); i++) {
		const Color* color;
		if (i < colors.size())
			color = &colors[i];
		else
			color = &colors.back();
		if (dx11RTVs[i] != NULL)
			dxContext.deviceContext->ClearRenderTargetView(dx11RTVs[i].Get(), (const float*)color);
	}
}

void DX11RenderContext::clearFrameDepth(float depth)
{
	if (dx11DSV != NULL)
		dxContext.deviceContext->ClearDepthStencilView(dx11DSV.Get(), D3D11_CLEAR_DEPTH, depth, 0);
}

void DX11RenderContext::clearFrameStencil(unsigned int stencil)
{
	if (dx11DSV != NULL)
		dxContext.deviceContext->ClearDepthStencilView(dx11DSV.Get(), D3D11_CLEAR_STENCIL, 0, stencil);
}

void DX11RenderContext::clearImageFloat(const Image& image, const Vector4f& value)
{
	DX11Texture2D* dxTex = NULL;
	ComPtr<ID3D11UnorderedAccessView> tex = NULL;
	if (image.texture) {
		dxTex = dynamic_cast<DX11Texture2D*>((ITexture*)image.texture->getVendorTexture());
		if (dxTex) {
			RWOption rwOption;
			rwOption.mipLevel = image.level;
			rwOption.arrayBase = image.arrayBase;
			rwOption.arrayCount = image.arrayCount;
			tex = dxTex->getUAV(rwOption);
		}
	}
	deviceContext->ClearUnorderedAccessViewFloat(tex.Get(), value.data());
}

void DX11RenderContext::clearImageUint(const Image& image, const Vector4u& value)
{
	DX11Texture2D* dxTex = NULL;
	ComPtr<ID3D11UnorderedAccessView> tex = NULL;
	if (image.texture) {
		dxTex = dynamic_cast<DX11Texture2D*>((ITexture*)image.texture->getVendorTexture());
		if (dxTex) {
			RWOption rwOption;
			rwOption.mipLevel = image.level;
			rwOption.arrayBase = image.arrayBase;
			rwOption.arrayCount = image.arrayCount;
			tex = dxTex->getUAV(rwOption);
		}
	}
	deviceContext->ClearUnorderedAccessViewUint(tex.Get(), value.data());
}

void DX11RenderContext::clearOutputBufferFloat(IGPUBuffer* buffer, const Vector4f& value)
{
	DX11GPUBuffer* dxBuffer = dynamic_cast<DX11GPUBuffer*>(buffer);
	if (dxBuffer == NULL || dxBuffer->dx11Buffer == NULL)
		return;
	if (dxBuffer->desc.gpuAccess != GAF_ReadWrite)
		throw runtime_error("Only Buffer with GAF_ReadWrite can be cleared");
	deviceContext->ClearUnorderedAccessViewFloat(dxBuffer->dx11BufferUAV.Get(), value.data());
}

void DX11RenderContext::clearOutputBufferUint(IGPUBuffer* buffer, const Vector4u& value)
{
	DX11GPUBuffer* dxBuffer = dynamic_cast<DX11GPUBuffer*>(buffer);
	if (dxBuffer == NULL || dxBuffer->dx11Buffer == NULL)
		return;
	if (dxBuffer->desc.gpuAccess != GAF_ReadWrite)
		throw runtime_error("Only Buffer with GAF_ReadWrite can be cleared");
	deviceContext->ClearUnorderedAccessViewUint(dxBuffer->dx11BufferUAV.Get(), value.data());
}

void DX11RenderContext::copyTexture2D(ITexture* srcTex, ITexture* dstTex)
{
	DX11Texture2D* dxSrcTex = dynamic_cast<DX11Texture2D*>(srcTex);
	DX11Texture2D* dxDstTex = dynamic_cast<DX11Texture2D*>(dstTex);
	if (dxSrcTex && dxDstTex)
		dxContext.deviceContext->CopyResource(dxDstTex->dx11Texture2D.Get(), dxSrcTex->dx11Texture2D.Get());
}

void DX11RenderContext::copySubTexture2D(ITexture* srcTex, unsigned int srcMip, ITexture* dstTex, unsigned int dstMip)
{
	DX11Texture2D* dxSrcTex = dynamic_cast<DX11Texture2D*>(srcTex);
	DX11Texture2D* dxDstTex = dynamic_cast<DX11Texture2D*>(dstTex);
	if (dxSrcTex && dxDstTex) {
		dxContext.deviceContext->CopySubresourceRegion(
			dxDstTex->dx11Texture2D.Get(),
			D3D11CalcSubresource(dstMip, 0, 1),
			0, 0, 0, dxSrcTex->dx11Texture2D.Get(),
			D3D11CalcSubresource(srcMip, 0, 1), NULL);
	}
}

unsigned int DX11RenderContext::bindShaderProgram(ShaderProgram* program)
{
	DX11ShaderProgram* dxProgram = dynamic_cast<DX11ShaderProgram*>(program);
	if (dxProgram == NULL)
		return 0;
	/*if (dxProgram->isDirty())
		throw runtime_error("Shader is not ready");*/
	if (currentProgram == dxProgram)
		return dxProgram->getProgramID();
	deviceContext->VSSetShader(NULL, NULL, 0);
	deviceContext->PSSetShader(NULL, NULL, 0);
	deviceContext->GSSetShader(NULL, NULL, 0);
	deviceContext->CSSetShader(NULL, NULL, 0);
	deviceContext->HSSetShader(NULL, NULL, 0);
	deviceContext->DSSetShader(NULL, NULL, 0);

	for (auto b = dxProgram->shaderStages.begin(), e = dxProgram->shaderStages.end(); b != e; b++) {
		switch (b->first)
		{
		case Vertex_Shader_Stage:
			deviceContext->VSSetShader((ID3D11VertexShader*)b->second->getShaderID(), NULL, 0);
			break;
		case Fragment_Shader_Stage:
			deviceContext->PSSetShader((ID3D11PixelShader*)b->second->getShaderID(), NULL, 0);
			break;
		case Geometry_Shader_Stage:
			deviceContext->GSSetShader((ID3D11GeometryShader*)b->second->getShaderID(), NULL, 0);
			break;
		case Compute_Shader_Stage:
			deviceContext->CSSetShader((ID3D11ComputeShader*)b->second->getShaderID(), NULL, 0);
			break;
		case Tessellation_Control_Shader_Stage:
			deviceContext->HSSetShader((ID3D11HullShader*)b->second->getShaderID(), NULL, 0);
			break;
		case Tessellation_Evalution_Shader_Stage:
			deviceContext->DSSetShader((ID3D11DomainShader*)b->second->getShaderID(), NULL, 0);
			break;
		}
	}

	currentProgram = dxProgram;
	return dxProgram->getProgramID();
}

unsigned int DX11RenderContext::dispatchCompute(unsigned int dimX, unsigned int dimY, unsigned int dimZ)
{
	if (!currentProgram->isComputable())
		return false;
	bindDrawInfo();
	deviceContext->Dispatch(dimX, dimY, dimZ);
	return true;
}

unsigned int DX11RenderContext::dispatchComputeIndirect(IGPUBuffer* buffer, unsigned int byteOffset)
{
	if (!currentProgram->isComputable())
		return false;

	DX11GPUBuffer* dxBuffer = dynamic_cast<DX11GPUBuffer*>(buffer);
	if (dxBuffer == NULL)
		return false;

	if (dxBuffer->desc.type != GB_Command)
		throw runtime_error("dispatchComputeIndirect: GPUBuffer with GB_Command is required");

	bindDrawInfo();
	deviceContext->DispatchIndirect(dxBuffer->dx11Buffer.Get(), byteOffset);
	return true;
}

void DX11RenderContext::bindMaterialTextures(IMaterial* material)
{
	if (currentProgram == NULL)
		return;
	DX11Material* dxMaterial = dynamic_cast<DX11Material*>(material);
	if (dxMaterial == NULL)
		return;
	for (auto b = dxMaterial->desc.textureField.begin(), e = dxMaterial->desc.textureField.end(); b != e; b++) {
		if (b->second.val == NULL)
			continue;
		if (b->second.val->getVendorTexture() == NULL) {
			throw runtime_error("Texture is not ready");
			continue;
		}

		bindTexture((ITexture*)b->second.val->getVendorTexture(), b->first);
	}
}

void DX11RenderContext::bindMaterialImages(IMaterial* material)
{
	if (currentProgram == NULL)
		return;
	DX11Material* dxMaterial = dynamic_cast<DX11Material*>(material);
	if (dxMaterial == NULL)
		return;
	if (!dxMaterial->desc.imageField.empty())
		clearRTV();
	for (auto b = dxMaterial->desc.imageField.begin(), e = dxMaterial->desc.imageField.end(); b != e; b++) {
		if (!b->second.val.isValid())
			continue;
		if (b->second.val.texture->getVendorTexture() == NULL) {
			throw runtime_error("Texture is not ready");
			continue;
		}
		bindImage(b->second.val, b->first);
	}
}

void DX11RenderContext::bindMaterialBuffer(IMaterial* material)
{
	if (currentProgram == NULL)
		return;
	DX11Material* dxMaterial = dynamic_cast<DX11Material*>(material);
	if (dxMaterial->matInsBuf) {
		D3D11_MAPPED_SUBRESOURCE mpd;
		deviceContext->Map(dxMaterial->matInsBuf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mpd);
		memcpy_s(mpd.pData, dxMaterial->matInsBufSize, dxMaterial->matInsBufHost, dxMaterial->matInsBufSize);
		deviceContext->Unmap(dxMaterial->matInsBuf.Get(), 0);
		currentProgram->bindCBV(deviceContext, DX11ShaderStage::materialParameterBufferName, dxMaterial->matInsBuf);
	}
}

bool DX11RenderContext::canBindRTV(ITexture* texture)
{
	return srvBindings.find(texture) == srvBindings.end() &&
		uavBindings.find(texture) == uavBindings.end();
}

void DX11RenderContext::bindTexture(ITexture* texture, ShaderStageType stage, unsigned int index, unsigned int sampleIndex, const MipOption& mipOption)
{
	DX11Texture2D* dxTex = dynamic_cast<DX11Texture2D*>(texture);
	ComPtr<ID3D11ShaderResourceView> tex = NULL;
	ComPtr<ID3D11SamplerState> sample = NULL;

#if TEX_BINGING_REC
	auto preTexIter = srvSlots.find(index);
	if (dxTex != NULL) {
		tex = dxTex->getSRV();
		sample = dxTex->getSampler();
		if (preTexIter != srvSlots.end()) {
			srvBindings.erase(preTexIter->second);
			preTexIter->second = dxTex;
		}
		else
			srvSlots.insert(make_pair(index, dxTex));
		srvBindings.insert(dxTex);
	}
	else {
		if (preTexIter != srvSlots.end()) {
			srvBindings.erase(preTexIter->second);
			srvSlots.erase(preTexIter);
		}
	}
#else
	if (dxTex != NULL) {
		tex = dxTex->getSRV(mipOption);
		sample = dxTex->getSampler();
	}
#endif
	switch (stage)
	{
	case Vertex_Shader_Stage:
		deviceContext->VSSetShaderResources(index, 1, tex.GetAddressOf());
		if (sampleIndex != -1)
			deviceContext->VSSetSamplers(sampleIndex, 1, sample.GetAddressOf());
		break;
	case Fragment_Shader_Stage:
		deviceContext->PSSetShaderResources(index, 1, tex.GetAddressOf());
		if (sampleIndex != -1)
			deviceContext->PSSetSamplers(sampleIndex, 1, sample.GetAddressOf());
		break;
	case Geometry_Shader_Stage:
		deviceContext->GSSetShaderResources(index, 1, tex.GetAddressOf());
		if (sampleIndex != -1)
			deviceContext->GSSetSamplers(sampleIndex, 1, sample.GetAddressOf());
		break;
	case Compute_Shader_Stage:
		deviceContext->CSSetShaderResources(index, 1, tex.GetAddressOf());
		if (sampleIndex != -1)
			deviceContext->CSSetSamplers(sampleIndex, 1, sample.GetAddressOf());
		break;
	case Tessellation_Control_Shader_Stage:
		deviceContext->HSSetShaderResources(index, 1, tex.GetAddressOf());
		if (sampleIndex != -1)
			deviceContext->HSSetSamplers(sampleIndex, 1, sample.GetAddressOf());
		break;
	case Tessellation_Evalution_Shader_Stage:
		deviceContext->DSSetShaderResources(index, 1, tex.GetAddressOf());
		if (sampleIndex != -1)
			deviceContext->DSSetSamplers(sampleIndex, 1, sample.GetAddressOf());
		break;
	default:
		return;
	}
}

void DX11RenderContext::bindImage(const Image& image, unsigned int index)
{
	DX11Texture2D* dxTex = NULL;
	ComPtr<ID3D11UnorderedAccessView> tex = NULL;
	if (image.texture) {
		dxTex = dynamic_cast<DX11Texture2D*>((ITexture*)image.texture->getVendorTexture());
		if (dxTex) {
			RWOption rwOption;
			rwOption.mipLevel = image.level;
			rwOption.arrayBase = image.arrayBase;
			rwOption.arrayCount = image.arrayCount;
			tex = dxTex->getUAV(rwOption);
		}
	}
	deviceContext->CSSetUnorderedAccessViews(index, 1, tex.GetAddressOf(), NULL);

#if TEX_BINGING_REC
	auto preTexIter = uavSlots.find(index);
	if (dxTex != NULL) {
		if (preTexIter != uavSlots.end()) {
			uavBindings.erase(preTexIter->second);
			preTexIter->second = dxTex;
		}
		else
			uavSlots.insert(make_pair(index, dxTex));
		uavBindings.insert(dxTex);
	}
	else {
		if (preTexIter != srvSlots.end()) {
			uavBindings.erase(preTexIter->second);
			uavSlots.erase(preTexIter);
		}
	}
#endif
}

void DX11RenderContext::bindTexture(ITexture* texture, const ShaderPropertyName& name, const MipOption& mipOption)
{
	if (currentProgram == NULL)
		return;
	DX11Texture2D* dxTex = dynamic_cast<DX11Texture2D*>(texture);
	ComPtr<ID3D11ShaderResourceView> tex = NULL;
	ComPtr<ID3D11SamplerState> sample = NULL;

#if TEX_BINGING_REC
	auto preTexIter = srvSlots.find(index);
	if (dxTex != NULL) {
		tex = dxTex->getSRV();
		sample = dxTex->getSampler();
		if (preTexIter != srvSlots.end()) {
			srvBindings.erase(preTexIter->second);
			preTexIter->second = dxTex;
		}
		else
			srvSlots.insert(make_pair(index, dxTex));
		srvBindings.insert(dxTex);
	}
	else {
		if (preTexIter != srvSlots.end()) {
			srvBindings.erase(preTexIter->second);
			srvSlots.erase(preTexIter);
		}
	}
#else
	if (dxTex != NULL) {
		tex = dxTex->getSRV(mipOption);
		sample = dxTex->getSampler();
	}
#endif
	currentProgram->bindSRVWithSampler(deviceContext, name, tex, sample);
}

void DX11RenderContext::bindImage(const Image& image, const ShaderPropertyName& name)
{
	if (currentProgram == NULL)
		return;
	DX11Texture2D* dxTex = NULL;
	ComPtr<ID3D11UnorderedAccessView> tex = NULL;
	if (image.texture) {
		dxTex = dynamic_cast<DX11Texture2D*>((ITexture*)image.texture->getVendorTexture());
		if (dxTex) {
			RWOption rwOption;
			rwOption.mipLevel = image.level;
			rwOption.arrayBase = image.arrayBase;
			rwOption.arrayCount = image.arrayCount;
			tex = dxTex->getUAV(rwOption);
		}
	}
	currentProgram->bindUAV(deviceContext, name, tex);

#if TEX_BINGING_REC
	auto preTexIter = uavSlots.find(index);
	if (dxTex != NULL) {
		if (preTexIter != uavSlots.end()) {
			uavBindings.erase(preTexIter->second);
			preTexIter->second = dxTex;
		}
		else
			uavSlots.insert(make_pair(index, dxTex));
		uavBindings.insert(dxTex);
	}
	else {
		if (preTexIter != srvSlots.end()) {
			uavBindings.erase(preTexIter->second);
			uavSlots.erase(preTexIter);
		}
	}
#endif
}

void DX11RenderContext::clearSRV()
{
	const int size = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - TEX_START_BIND_INDEX;
	ID3D11ShaderResourceView* srvs[size] = { NULL };
	deviceContext->VSSetShaderResources(TEX_START_BIND_INDEX, size, srvs);
	deviceContext->PSSetShaderResources(TEX_START_BIND_INDEX, size, srvs);
	deviceContext->GSSetShaderResources(TEX_START_BIND_INDEX, size, srvs);
	deviceContext->HSSetShaderResources(TEX_START_BIND_INDEX, size, srvs);
	deviceContext->DSSetShaderResources(TEX_START_BIND_INDEX, size, srvs);
	deviceContext->CSSetShaderResources(TEX_START_BIND_INDEX, size, srvs);
	srvBindings.clear();
	srvSlots.clear();
}

void DX11RenderContext::clearUAV()
{
	ID3D11UnorderedAccessView* uavs[D3D11_PS_CS_UAV_REGISTER_COUNT] = { NULL };
	unsigned int offs[D3D11_PS_CS_UAV_REGISTER_COUNT] = { -1 };
	deviceContext->CSSetUnorderedAccessViews(0, D3D11_PS_CS_UAV_REGISTER_COUNT, uavs, offs);
	deviceContext->OMSetRenderTargetsAndUnorderedAccessViews(D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL,
		NULL, NULL, 0, D3D11_PS_CS_UAV_REGISTER_COUNT, uavs, NULL);
	uavBindings.clear();
	uavSlots.clear();
}

void DX11RenderContext::clearRTV()
{
	deviceContext->OMSetRenderTargets(0, NULL, NULL);
	DX11RenderTarget::currentRenderTarget = nullptr;
	currentRenderTarget = nullptr;
	currentDeviceSurface = nullptr;
}

void DX11RenderContext::bindMeshData(MeshData* meshData)
{
	if (meshData == currentMeshData)
		return;
	DX11MeshData* dxMeshData = dynamic_cast<DX11MeshData*>(meshData);
	DX11SkeletonMeshData* dxSkeletonMeshData = dynamic_cast<DX11SkeletonMeshData*>(meshData);
	if (dxMeshData) {
		ID3D11Buffer* buffers[] = {
		dxMeshData->dx11VertexBuffer.Get(),
		dxMeshData->dx11UVBuffer.Get(),
		dxMeshData->dx11NormalBuffer.Get()
		};
		UINT strides[] = {
			sizeof(Vector3f),
			sizeof(Vector2f),
			sizeof(Vector3f)
		};
		UINT offsets[] = { 0, 0, 0 };

		deviceContext->IASetVertexBuffers(1, 3, buffers, strides, offsets);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		deviceContext->IASetIndexBuffer(dxMeshData->dx11ElementBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		deviceContext->IASetInputLayout(dxContext.meshInputLayout.Get());
		currentMeshData = meshData;
	}
	else if (dxSkeletonMeshData) {
		ID3D11Buffer* buffers[] = {
		dxSkeletonMeshData->dx11VertexBuffer.Get(),
		dxSkeletonMeshData->dx11UVBuffer.Get(),
		dxSkeletonMeshData->dx11NormalBuffer.Get(),
		dxSkeletonMeshData->dx11BoneIndexBuffer.Get(),
		dxSkeletonMeshData->dx11BoneWeightBuffer.Get()
		};
		UINT strides[] = {
			sizeof(Vector3f),
			sizeof(Vector2f),
			sizeof(Vector3f),
			sizeof(Vector4u),
			sizeof(Vector4f)
		};
		UINT offsets[] = { 0, 0, 0, 0, 0 };

		deviceContext->IASetVertexBuffers(1, 5, buffers, strides, offsets);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		deviceContext->IASetIndexBuffer(dxSkeletonMeshData->dx11ElementBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		deviceContext->IASetInputLayout(dxContext.skeletonMeshInputLayout.Get());

		if (dxSkeletonMeshData->dx11MorphVNView != NULL) {
			static const ShaderPropertyName morphDataName = "morphData";
			currentProgram->bindSRV(deviceContext, morphDataName, dxSkeletonMeshData->dx11MorphVNView);
			//deviceContext->VSSetShaderResources(MORPHDATA_BIND_INDEX, 1, dxSkeletonMeshData->dx11MorphVNView.GetAddressOf());
		}
		currentMeshData = meshData;
	}
	else {
		meshData->bindShapeWithContext(*this);
	}
}

void DX11RenderContext::setRenderPreState()
{
	deviceContext->OMSetBlendState(dxContext.blendOffWriteOff.Get(), NULL, 0xFFFFFFFF);
	deviceContext->OMSetDepthStencilState(dxContext.depthWriteOnTestOnLEqual.Get(), 0);
}

void DX11RenderContext::setRenderGeomtryState()
{
	deviceContext->OMSetBlendState(dxContext.blendGBuffer.Get(), NULL, 0xFFFFFFFF);
	deviceContext->OMSetDepthStencilState(dxContext.depthWriteOnTestOnLEqual.Get(), 0);
}

void DX11RenderContext::setRenderOpaqueState()
{
	deviceContext->OMSetBlendState(dxContext.blendOffWriteOn.Get(), NULL, 0xFFFFFFFF);
	deviceContext->OMSetDepthStencilState(dxContext.depthWriteOnTestOnLEqual.Get(), 0);
}

void DX11RenderContext::setRenderAlphaState()
{
	deviceContext->OMSetBlendState(dxContext.blendOffWriteOnAlphaTest.Get(), NULL, 0xFFFFFFFF);
	deviceContext->OMSetDepthStencilState(dxContext.depthWriteOnTestOnLEqual.Get(), 0);
}

void DX11RenderContext::setRenderTransparentState()
{
	deviceContext->OMSetBlendState(dxContext.blendOnWriteOn.Get(), NULL, 0xFFFFFFFF);
	deviceContext->OMSetDepthStencilState(dxContext.depthWriteOffTestOnLEqual.Get(), 0);
}

void DX11RenderContext::setRenderOverlayState()
{
	deviceContext->OMSetBlendState(dxContext.blendOnWriteOn.Get(), NULL, 0xFFFFFFFF);
	deviceContext->OMSetDepthStencilState(dxContext.depthWriteOffTestOffLEqual.Get(), 0);
}

void DX11RenderContext::setRenderPostState()
{
	deviceContext->OMSetBlendState(dxContext.blendOnWriteOn.Get(), NULL, 0xFFFFFFFF);
	deviceContext->OMSetDepthStencilState(dxContext.depthWriteOffTestOffLEqual.Get(), 0);
}

void DX11RenderContext::setRenderPostAddState()
{
	deviceContext->OMSetBlendState(dxContext.blendAddWriteOn.Get(), NULL, 0xFFFFFFFF);
	deviceContext->OMSetDepthStencilState(dxContext.depthWriteOffTestOffLEqual.Get(), 0);
}

void DX11RenderContext::setRenderPostPremultiplyAlphaState()
{
	deviceContext->OMSetBlendState(dxContext.blendPremultiplyAlphaWriteOn.Get(), NULL, 0xFFFFFFFF);
	deviceContext->OMSetDepthStencilState(dxContext.depthWriteOffTestOffLEqual.Get(), 0);
}

void DX11RenderContext::setRenderPostMultiplyState()
{
	deviceContext->OMSetBlendState(dxContext.blendMultiplyWriteOn.Get(), NULL, 0xFFFFFFFF);
	deviceContext->OMSetDepthStencilState(dxContext.depthWriteOffTestOffLEqual.Get(), 0);
}

void DX11RenderContext::setRenderPostMaskState()
{
	deviceContext->OMSetBlendState(dxContext.blendMaskWriteOn.Get(), NULL, 0xFFFFFFFF);
	deviceContext->OMSetDepthStencilState(dxContext.depthWriteOffTestOffLEqual.Get(), 0);
}

void DX11RenderContext::setRenderPostReplaceState()
{
	deviceContext->OMSetBlendState(dxContext.blendOffWriteOn.Get(), NULL, 0xFFFFFFFF);
	deviceContext->OMSetDepthStencilState(dxContext.depthWriteOffTestOffLEqual.Get(), 0);
}

void DX11RenderContext::setCullState(CullType type)
{
	if (type == Cull_Back)
		deviceContext->RSSetState(dxContext.rasterizerCullBack.Get());
	else if (type == Cull_Front)
		deviceContext->RSSetState(dxContext.rasterizerCullFront.Get());
	else
		deviceContext->RSSetState(dxContext.rasterizerCullOff.Get());
}

void DX11RenderContext::setViewport(unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
	D3D11_VIEWPORT vp;
	vp.TopLeftX = x;
	vp.TopLeftY = y;
	vp.Width = w;
	vp.Height = h;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	deviceContext->RSSetViewports(1, &vp);
}

void DX11RenderContext::setLineDrawContext()
{
	deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	deviceContext->IASetInputLayout(dxContext.screenInputLayout.Get());
}

void DX11RenderContext::setMeshDrawContext()
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(dxContext.meshInputLayout.Get());
}

void DX11RenderContext::setSkeletonMeshDrawContext()
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(dxContext.skeletonMeshInputLayout.Get());
}

void DX11RenderContext::setTerrainDrawContext()
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	deviceContext->IASetInputLayout(dxContext.terrainInputLayout.Get());
}

void DX11RenderContext::setDrawInfo(int passIndex, int passNum, unsigned int materialID)
{
	drawInfo.passID = passIndex;
	drawInfo.passNum = passNum;
	drawInfo.materialID = materialID;
}

void DX11RenderContext::bindDrawInfo()
{
	if (drawInfoBuf == NULL) {
		D3D11_BUFFER_DESC cbDesc;
		ZeroMemory(&cbDesc, sizeof(D3D11_BUFFER_DESC));
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.ByteWidth = sizeof(DrawInfo);
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		dxContext.device->CreateBuffer(&cbDesc, NULL, &drawInfoBuf);
	}
	drawInfo.gameTime = Time::duration().toSecond();
	if (drawInfo != uploadedDrawInfo) {
		uploadedDrawInfo = drawInfo;
		D3D11_MAPPED_SUBRESOURCE mpd;
		deviceContext->Map(drawInfoBuf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mpd);
		memcpy_s(mpd.pData, sizeof(DrawInfo), &uploadedDrawInfo, sizeof(DrawInfo));
		deviceContext->Unmap(drawInfoBuf.Get(), 0);
	}
	currentProgram->bindCBV(deviceContext, DX11ShaderStage::drawInfoBufferName, drawInfoBuf);
}

void DX11RenderContext::meshDrawCall(const MeshPartDesc& mesh)
{
	if (mesh.meshData == NULL) {
		currentMeshData = NULL;
	}
	else {
		bindMeshData(mesh.meshData);
	}
	bindDrawInfo();
	deviceContext->DrawIndexed(mesh.elementCount, mesh.elementFirst, mesh.vertexFirst);
}

void DX11RenderContext::postProcessCall()
{
	currentMeshData = NULL;
	deviceContext->IASetInputLayout(dxContext.screenInputLayout.Get());
	deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	bindDrawInfo();
	deviceContext->Draw(4, 0);
	deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DX11RenderContext::execteParticleDraw(IRenderExecution* execution, const vector<DrawArraysIndirectCommand>& cmds)
{
	// DX11RenderExecution* dxExec = dynamic_cast<DX11RenderExecution*>(execution);
	// if (dxExec == NULL)
	// 	return;
	deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
#if INDIRECT_DRAW
	size_t size = sizeof(DrawArraysIndirectCommand) * cmds.size();
	if (dxExec->cmdBufferDesc.ByteWidth != size) {
		dxExec->cmdBufferDesc.ByteWidth = size;
		/*if (cmdBuffer != NULL) {
			cmdBuffer->Release();
		}*/
		dxContext.device->CreateBuffer(&dxExec->cmdBufferDesc, NULL, &dxExec->cmdBuffer);
	}
	D3D11_MAPPED_SUBRESOURCE cmdmappedData;
	deviceContext->Map(dxExec->cmdBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &cmdmappedData);
	memcpy_s(cmdmappedData.pData, dxExec->cmdBufferDesc.ByteWidth, cmds.data(), dxExec->cmdBufferDesc.ByteWidth);
	deviceContext->Unmap(dxExec->cmdBuffer.Get(), 0);
#endif
	for (int i = 0; i < cmds.size(); i++) {
		const DrawArraysIndirectCommand& c = cmds[i];
		bindDrawInfo();
#if INDIRECT_DRAW
		deviceContext->DrawInstancedIndirect(dxExec->cmdBuffer.Get(), sizeof(DrawArraysIndirectCommand) * i);
#else
		deviceContext->DrawInstanced(c.count, c.instanceCount, c.first, c.baseInstance);
#endif
	}
}

void DX11RenderContext::execteMeshDraw(IRenderExecution* execution, const vector<DrawElementsIndirectCommand>& cmds)
{
	DX11RenderExecution* dxExec = dynamic_cast<DX11RenderExecution*>(execution);
	if (dxExec == NULL)
		return;
#if INDIRECT_DRAW
	size_t size = sizeof(DrawElementsIndirectCommand) * cmds.size();
	if (dxExec->cmdBufferDesc.ByteWidth != size) {
		dxExec->cmdBufferDesc.ByteWidth = size;
		/*if (cmdBuffer != NULL) {
			cmdBuffer->Release();
		}*/
		dxContext.device->CreateBuffer(&dxExec->cmdBufferDesc, NULL, &dxExec->cmdBuffer);
	}
	D3D11_MAPPED_SUBRESOURCE cmdmappedData;
	deviceContext->Map(dxExec->cmdBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &cmdmappedData);
	memcpy_s(cmdmappedData.pData, dxExec->cmdBufferDesc.ByteWidth, cmds.data(), dxExec->cmdBufferDesc.ByteWidth);
	deviceContext->Unmap(dxExec->cmdBuffer.Get(), 0);
#endif
	for (int i = 0; i < cmds.size(); i++) {
		const DrawElementsIndirectCommand& c = cmds[i];
		bindDrawInfo();
#if INDIRECT_DRAW
		deviceContext->DrawIndexedInstancedIndirect(dxExec->cmdBuffer.Get(), sizeof(DrawArraysIndirectCommand) * i);
#else
		deviceContext->DrawIndexedInstanced(c.count, c.instanceCount, c.firstIndex, c.baseVertex, c.baseInstance);
#endif
	}
}

void DX11RenderContext::execteImGuiDraw(ImDrawData* drawData)
{
	if (drawData == NULL || !drawData->Valid)
		return;
	ImGui_ImplDX11_RenderDrawData(drawData, deviceContext.Get());
}

bool DX11RenderContext::drawMesh(const DrawElementsIndirectCommand& cmd)
{
	if (currentProgram->isComputable())
		return false;
	bindDrawInfo();
	deviceContext->DrawIndexedInstanced(cmd.count, cmd.instanceCount, cmd.firstIndex, cmd.baseVertex, cmd.baseInstance);
	return true;
}

bool DX11RenderContext::drawArray(const DrawArraysIndirectCommand& cmd)
{
	if (currentProgram->isComputable())
		return false;
	bindDrawInfo();
	deviceContext->DrawInstanced(cmd.count, cmd.instanceCount, cmd.first, cmd.baseInstance);
	return true;
}

bool DX11RenderContext::drawMeshIndirect(IGPUBuffer* argBuffer, unsigned int byteOffset)
{
	if (currentProgram->isComputable())
		return false;

	DX11GPUBuffer* dxBuffer = dynamic_cast<DX11GPUBuffer*>(argBuffer);
	if (dxBuffer == NULL)
		return false;

	if (dxBuffer->desc.type != GB_Command)
		throw runtime_error("drawMeshIndirect: GPUBuffer with GB_Command is required");

	bindDrawInfo();
	deviceContext->DrawIndexedInstancedIndirect(dxBuffer->dx11Buffer.Get(), byteOffset);
	return true;
}

bool DX11RenderContext::drawArrayIndirect(IGPUBuffer* argBuffer, unsigned int byteOffset)
{
	if (currentProgram->isComputable())
		return false;

	DX11GPUBuffer* dxBuffer = dynamic_cast<DX11GPUBuffer*>(argBuffer);
	if (dxBuffer == NULL)
		return false;

	if (dxBuffer->desc.type != GB_Command)
		throw runtime_error("drawArrayIndirect: GPUBuffer with GB_Command is required");

	bindDrawInfo();
	deviceContext->DrawInstancedIndirect(dxBuffer->dx11Buffer.Get(), byteOffset);
	return true;
}

void DX11RenderContext::submit()
{
	if (isDefaultContext)
		return;
	ComPtr<ID3D11CommandList> commandList;
	if (FAILED(deviceContext->FinishCommandList(true, commandList.GetAddressOf())))
		throw runtime_error("FinishCommandList failed");
	if (commandList)
		dxContext.deviceContext->ExecuteCommandList(commandList.Get(), true);
}
