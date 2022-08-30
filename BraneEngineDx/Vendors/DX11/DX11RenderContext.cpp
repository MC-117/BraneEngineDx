#include "DX11RenderContext.h"
#include "imgui_impl_dx11.h"

#define TEX_BINGING_REC 0
#define INDIRECT_DRAW 0

DX11RenderContext::DX11RenderContext(DX11Context& context, RenderContextDesc& desc)
    : IRenderContext(desc), dxContext(context)
{
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
    if (deviceContext)
        deviceContext.Reset();
}

unsigned int DX11RenderContext::bindBufferBase(IGPUBuffer* buffer, unsigned int index)
{
	DX11GPUBuffer* dxBuffer = dynamic_cast<DX11GPUBuffer*>(buffer);
	if (dxBuffer == NULL || dxBuffer->dx11Buffer == NULL)
		return 0;
	switch (dxBuffer->desc.type)
	{
	case GB_Constant:
		deviceContext->VSSetConstantBuffers(index, 1, dxBuffer->dx11Buffer.GetAddressOf());
		deviceContext->PSSetConstantBuffers(index, 1, dxBuffer->dx11Buffer.GetAddressOf());
		deviceContext->GSSetConstantBuffers(index, 1, dxBuffer->dx11Buffer.GetAddressOf());
		deviceContext->HSSetConstantBuffers(index, 1, dxBuffer->dx11Buffer.GetAddressOf());
		deviceContext->DSSetConstantBuffers(index, 1, dxBuffer->dx11Buffer.GetAddressOf());
		break;
	case GB_Vertex:
	{
		unsigned int strides = dxBuffer->desc.cellSize;
		unsigned int offset = 0;
		deviceContext->IASetVertexBuffers(index, 1, dxBuffer->dx11Buffer.GetAddressOf(), &strides, &offset);
		break;
	}
	case GB_Index:
	{
		unsigned int strides = sizeof(unsigned int);
		unsigned int offset = 0;
		deviceContext->IASetIndexBuffer(dxBuffer->dx11Buffer.Get(), DXGI_FORMAT_R32_UINT, offset);
		break;
	}
	case GB_Storage:
	case GB_Struct:
		if (dxBuffer->dx11BufferView == NULL)
			return 0;
		deviceContext->VSSetShaderResources(index, 1, dxBuffer->dx11BufferView.GetAddressOf());
		deviceContext->PSSetShaderResources(index, 1, dxBuffer->dx11BufferView.GetAddressOf());
		deviceContext->GSSetShaderResources(index, 1, dxBuffer->dx11BufferView.GetAddressOf());
		deviceContext->HSSetShaderResources(index, 1, dxBuffer->dx11BufferView.GetAddressOf());
		deviceContext->DSSetShaderResources(index, 1, dxBuffer->dx11BufferView.GetAddressOf());
		break;
	default:
		throw runtime_error("Unknown Error");
		break;
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

unsigned int DX11RenderContext::bindFrame(IRenderTarget* target)
{
	DX11RenderTarget* dxTarget = dynamic_cast<DX11RenderTarget*>(target);
	if (dxTarget == NULL)
		return 0;
	if (dxTarget->isDefault()) {
		deviceContext->OMSetRenderTargets(1, dxContext.backBufferRTV[0].GetAddressOf(), NULL);
		currentRenderTarget = dxTarget;
		dx11RTVs.resize(1);
		dx11RTVs[0] = dxContext.backBufferRTV[0];
		dx11DSV = nullptr;
		return 0;
	}
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
#if TEX_BINGING_REC
		if (canBindRTV(tex))
			dx11RTVs[i] = tex->getRTV(rtInfo.mipLevel, isMs);
		else
			throw runtime_error("Texture is still binded as SRV or UAV");
#else
		dx11RTVs[i] = tex->getRTV(rtInfo.mipLevel, isMs);
#endif
	}
	
	deviceContext->OMSetRenderTargets(dx11RTVs.size(),
		(ID3D11RenderTargetView* const*)dx11RTVs.data(), dx11DSV.Get());
	currentRenderTarget = dxTarget;
	return dxTarget->desc.frameID;
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
			auto uav = dxTarget->dx11DepthTex->getUAV(0);
			clearSRV();
			clearUAV();
			clearRTV();
			currentRenderTarget = NULL;
			deviceContext->CSSetShaderResources(7, 1, srv.GetAddressOf());
			deviceContext->CSSetUnorderedAccessViews(0, 1, uav.GetAddressOf(), NULL);
			dispatchCompute(ceil(dxTarget->desc.width / 16.0f), ceil(dxTarget->desc.height / 16.0f), 1);
			clearUAV();
		}
	}
}

void DX11RenderContext::clearFrameColor(const Color& color)
{
	if (dx11RTVs.empty())
		return;
	auto rt = dx11RTVs.front();
	if (rt != NULL)
		deviceContext->ClearRenderTargetView(rt.Get(), (const float*)&color);
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

unsigned int DX11RenderContext::bindShaderProgram(ShaderProgram* program)
{
	DX11ShaderProgram* dxProgram = dynamic_cast<DX11ShaderProgram*>(program);
	if (dxProgram == NULL)
		return 0;
	if (dxProgram->isDirty())
		throw runtime_error("Shader is not ready");
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
		bindCBToStage(deviceContext, currentProgram, dxMaterial->matInsBuf, MAT_INS_BIND_INDEX);
	}
}

bool DX11RenderContext::canBindRTV(ITexture* texture)
{
	return srvBindings.find(texture) == srvBindings.end() &&
		uavBindings.find(texture) == uavBindings.end();
}

void DX11RenderContext::bindTexture(ITexture* texture, ShaderStageType stage, unsigned int index)
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
		tex = dxTex->getSRV();
		sample = dxTex->getSampler();
	}
#endif
	switch (stage)
	{
	case Vertex_Shader_Stage:
		deviceContext->VSSetShaderResources(index, 1, tex.GetAddressOf());
		if (index < 16)
			deviceContext->VSSetSamplers(index, 1, sample.GetAddressOf());
		break;
	case Fragment_Shader_Stage:
		deviceContext->PSSetShaderResources(index, 1, tex.GetAddressOf());
		if (index < 16)
			deviceContext->PSSetSamplers(index, 1, sample.GetAddressOf());
		break;
	case Geometry_Shader_Stage:
		deviceContext->GSSetShaderResources(index, 1, tex.GetAddressOf());
		if (index < 16)
			deviceContext->GSSetSamplers(index, 1, sample.GetAddressOf());
		break;
	case Compute_Shader_Stage:
		deviceContext->CSSetShaderResources(index, 1, tex.GetAddressOf());
		if (index < 16)
			deviceContext->CSSetSamplers(index, 1, sample.GetAddressOf());
		break;
	case Tessellation_Control_Shader_Stage:
		deviceContext->HSSetShaderResources(index, 1, tex.GetAddressOf());
		if (index < 16)
			deviceContext->HSSetSamplers(index, 1, sample.GetAddressOf());
		break;
	case Tessellation_Evalution_Shader_Stage:
		deviceContext->DSSetShaderResources(index, 1, tex.GetAddressOf());
		if (index < 16)
			deviceContext->DSSetSamplers(index, 1, sample.GetAddressOf());
		break;
	default:
		return;
	}
}

void DX11RenderContext::bindImage(const Image& image, unsigned int index)
{
	DX11Texture2D* dxTex = dynamic_cast<DX11Texture2D*>((ITexture*)image.texture->getVendorTexture());
	if (dxTex == NULL)
		return;

	ComPtr<ID3D11UnorderedAccessView> tex = dxTex->getUAV(image.level);
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

void DX11RenderContext::bindTexture(ITexture* texture, const string& name)
{
	if (currentProgram == NULL)
		return;
	DX11Texture2D* dxTex = dynamic_cast<DX11Texture2D*>(texture);
	if (dxTex == NULL)
		return;
	DX11ShaderProgram::AttributeDesc desc = currentProgram->getAttributeOffset(name);
	if (!desc.isTex || desc.offset == -1 || desc.meta == -1)
		return;
	bindTexture(dxTex, (ShaderStageType)desc.meta, desc.offset);
}

void DX11RenderContext::bindImage(const Image& image, const string& name)
{
	if (currentProgram == NULL || image.texture == NULL)
		return;
	DX11Texture2D* dxTex = dynamic_cast<DX11Texture2D*>((ITexture*)image.texture->getVendorTexture());
	if (dxTex == NULL)
		return;
	DX11ShaderProgram::AttributeDesc desc = currentProgram->getAttributeOffset(name);
	if (!desc.isTex || desc.offset == -1 || desc.meta == -1)
		return;
	bindImage(image, desc.offset);
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
	ID3D11UnorderedAccessView* srvs[D3D11_PS_CS_UAV_REGISTER_COUNT] = { NULL };
	unsigned int offs[D3D11_PS_CS_UAV_REGISTER_COUNT] = { -1 };
	deviceContext->CSSetUnorderedAccessViews(0, D3D11_PS_CS_UAV_REGISTER_COUNT, srvs, offs);
	uavBindings.clear();
	uavSlots.clear();
}

void DX11RenderContext::clearRTV()
{
	deviceContext->OMSetRenderTargets(0, NULL, NULL);
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

		dxContext.deviceContext->IASetVertexBuffers(1, 5, buffers, strides, offsets);
		dxContext.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		dxContext.deviceContext->IASetIndexBuffer(dxSkeletonMeshData->dx11ElementBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		dxContext.deviceContext->IASetInputLayout(dxContext.skeletonMeshInputLayout.Get());

		if (dxSkeletonMeshData->dx11MorphVNView != NULL)
			dxContext.deviceContext->VSSetShaderResources(MORPHDATA_BIND_INDEX, 1, dxSkeletonMeshData->dx11MorphVNView.GetAddressOf());
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
	deviceContext->OMSetBlendState(dxContext.blendOffWriteOn.Get(), NULL, 0xFFFFFFFF);
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

void DX11RenderContext::setDrawInfo(int passIndex, int passNum)
{
	drawInfo.passID = passIndex;
	drawInfo.passNum = passNum;
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
	if (drawInfo != uploadedDrawInfo) {
		uploadedDrawInfo = drawInfo;
		D3D11_MAPPED_SUBRESOURCE mpd;
		deviceContext->Map(drawInfoBuf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mpd);
		memcpy_s(mpd.pData, sizeof(DrawInfo), &uploadedDrawInfo, sizeof(DrawInfo));
		deviceContext->Unmap(drawInfoBuf.Get(), 0);
	}
	bindCBToStage(deviceContext, currentProgram, drawInfoBuf, DRAW_INFO_BIND_INDEX);
}

void DX11RenderContext::meshDrawCall(const MeshPartDesc& mesh)
{
	if (mesh.meshData == NULL) {
		currentMeshData = NULL;
	}
	else {
		bindMeshData(mesh.meshData);
	}
	drawInfo.baseInstance = 0;
	drawInfo.baseVertex = mesh.vertexFirst;
	bindDrawInfo();
	deviceContext->DrawIndexed(mesh.elementCount, mesh.elementFirst, mesh.vertexFirst);
}

void DX11RenderContext::postProcessCall()
{
	currentMeshData = NULL;
	deviceContext->IASetInputLayout(dxContext.screenInputLayout.Get());
	deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	drawInfo.baseInstance = 0;
	drawInfo.baseVertex = 0;
	bindDrawInfo();
	deviceContext->Draw(4, 0);
	deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DX11RenderContext::execteParticleDraw(IRenderExecution* execution, const vector<DrawArraysIndirectCommand>& cmds)
{
	DX11RenderExecution* dxExec = dynamic_cast<DX11RenderExecution*>(execution);
	if (dxExec == NULL)
		return;
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
		drawInfo.baseInstance = c.baseInstance;
		drawInfo.baseVertex = c.first;
		bindDrawInfo();
#if INDIRECT_DRAW
		deviceContext->DrawInstancedIndirect(dxExec->cmdBuffer.Get(), sizeof(DrawArraysIndirectCommand) * i);
#else
		deviceContext->DrawInstanced(cmds[i].count, cmds[i].instanceCount, cmds[i].first, cmds[i].baseInstance);
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
		drawInfo.baseInstance = c.baseInstance;
		drawInfo.baseVertex = c.baseVertex;
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
