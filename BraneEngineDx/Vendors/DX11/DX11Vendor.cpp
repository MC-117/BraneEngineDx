#include "DX11Vendor.h"
#include "DX11Texture2D.h"
#include "../../Core/Utility/RenderUtility.h"
#include "../imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#ifdef VENDOR_USE_DX11

REG_VENDOR_DEC(DX11Vendor);
REG_VENDOR_IMP(DX11Vendor, "DX11");

DX11Vendor::DX11Vendor()
	: StaticMeshData(dxContext),
	defaultRenderContext(dxContext, defaultRenderContextDesc)
{
	name = "DX11";
}

DX11Vendor::~DX11Vendor()
{
}

bool DX11Vendor::windowSetup(EngineConfig & config, WindowContext & context)
{
	return true;
}

bool DX11Vendor::setup(const EngineConfig & config, const WindowContext & context)
{
	dxContext.setHWnd(context.hwnd);
	if (!dxContext.createDevice(context.screenSize.x, context.screenSize.y))
		return false;
	defaultRenderContext.deviceContext = dxContext.deviceContext;
	DX11RenderTarget::initDepthBlit(dxContext);
	return true;
}


bool DX11Vendor::imGuiInit(const EngineConfig & config, const WindowContext & context)
{
	if (!ImGui_ImplWin32_Init(context.hwnd))
		return false;
	if (!ImGui_ImplDX11_Init(dxContext.device.Get(), dxContext.deviceContext.Get()))
		return false;
	return true;
}

bool DX11Vendor::imGuiNewFrame(const EngineConfig & config, const WindowContext & context)
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	return true;
}

bool DX11Vendor::imGuiDrawFrame(const EngineConfig & config, const WindowContext & context)
{
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	return true;
}

bool DX11Vendor::imGuiShutdown(const EngineConfig& config, const WindowContext& context)
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	return true;
}

bool DX11Vendor::swap(const EngineConfig & config, const WindowContext & context)
{
	dxContext.swap(config.vsnyc, config.maxFPS);
	return true;
}

bool DX11Vendor::clean(const EngineConfig & config, const WindowContext & context)
{
	dxContext.cleanupDevice();
	return true;
}

bool DX11Vendor::guiOnlyRender(const Color & clearColor)
{
	return true;
}

bool DX11Vendor::resizeWindow(const EngineConfig & config, const WindowContext & context, unsigned int width, unsigned int height)
{
	return true;
}

bool DX11Vendor::toggleFullscreen(const EngineConfig& config, const WindowContext& context, bool fullscreen)
{
	//if (dxContext.swapChain) {
	//	IDXGIOutput* pTarget; BOOL bFullscreen;
	//	if (SUCCEEDED(dxContext.swapChain->GetFullscreenState(&bFullscreen, &pTarget)))
	//	{
	//		if (pTarget != NULL)
	//			pTarget->Release();
	//	}
	//	else
	//		bFullscreen = FALSE;
	//	// If not full screen, enable full screen again.
	//	if (!bFullscreen && fullscreen)
	//	{
	//		ShowWindow(context.hwnd, SW_MINIMIZE);
	//		//ShowWindow(context.hwnd, SW_RESTORE);
	//		dxContext.swapChain->SetFullscreenState(TRUE, NULL);
	//	}
	//	if (bFullscreen && !fullscreen) {
	//		dxContext.swapChain->SetFullscreenState(FALSE, NULL);
	//	}
	//}
	return true;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT DX11Vendor::wndProcFunc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}

IRenderContext* DX11Vendor::getDefaultRenderContext()
{
	return &defaultRenderContext;
}

IRenderContext* DX11Vendor::newRenderContext()
{
	throw runtime_error("Not Implemented");
	return nullptr;
}

void DX11Vendor::frameFence()
{
	dxContext.fence();
}

ITexture2D * DX11Vendor::newTexture2D(Texture2DDesc& desc)
{
	return new DX11Texture2D(dxContext, desc);
}

ShaderStage* DX11Vendor::newShaderStage(const ShaderStageDesc& desc)
{
	return new DX11ShaderStage(dxContext, desc);
}

ShaderProgram* DX11Vendor::newShaderProgram()
{
	return new DX11ShaderProgram(dxContext);
}

IMaterial* DX11Vendor::newMaterial(MaterialDesc& desc)
{
	return new DX11Material(dxContext, desc);
}

IRenderTarget* DX11Vendor::newRenderTarget(RenderTargetDesc& desc)
{
	return new DX11RenderTarget(dxContext, desc);
}

IGPUBuffer* DX11Vendor::newGPUBuffer(GPUBufferDesc& desc)
{
	return new DX11GPUBuffer(dxContext, desc);
}

MeshPartDesc DX11Vendor::newMeshPart(unsigned int vertCount, unsigned int elementCount)
{
	DX11MeshData* data = NULL;
	if (StaticMeshData.isGenerated()) {
		for (auto b = MeshDataCollection.begin(), e = MeshDataCollection.end(); b != e; b++) {
			if ((*b)->type == MT_Mesh && !(*b)->isGenerated()) {
				data = (DX11MeshData*)*b;
				break;
			}
		}
	}
	else
		data = &StaticMeshData;
	if (data == NULL) {
		data = new DX11MeshData(dxContext);
		MeshDataCollection.insert(data);
	}
	MeshPartDesc desc = { data, (unsigned int)data->vertices.size(), vertCount, (unsigned int)data->elements.size(), elementCount };
	data->vertices.resize(data->vertices.size() + vertCount);
	data->uvs.resize(data->uvs.size() + vertCount);
	data->normals.resize(data->normals.size() + vertCount);
	data->elements.resize(data->elements.size() + elementCount);
	return desc;
}

SkeletonMeshPartDesc DX11Vendor::newSkeletonMeshPart(unsigned int vertCount, unsigned int elementCount, unsigned int boneCount, unsigned int morphVertCount, unsigned int morphCount)
{
	SkeletonMeshData* data = NULL;
	data = new DX11SkeletonMeshData(dxContext);
	MeshDataCollection.insert(data);
	SkeletonMeshPartDesc desc = { data, (unsigned int)data->vertices.size(), vertCount, (unsigned int)data->elements.size(), elementCount };
	if (morphCount > 0)
		desc.morphMeshData = &data->morphMeshData;
	data->vertices.resize(data->vertices.size() + vertCount);
	data->uvs.resize(data->uvs.size() + vertCount);
	data->normals.resize(data->normals.size() + vertCount);
	data->boneIndexes.resize(data->boneIndexes.size() + vertCount, Vector4u::Zero());
	data->weights.resize(data->weights.size() + vertCount, Vector4f::Zero());
	data->elements.resize(data->elements.size() + elementCount);
	data->boneCount = boneCount;
	data->morphMeshData.init(morphVertCount, morphCount);
	return desc;
}

IRenderExecution* DX11Vendor::newRenderExecution()
{
	return new DX11RenderExecution(dxContext);
}

void DX11Vendor::setRenderPreState()
{
	dxContext.deviceContext->OMSetBlendState(dxContext.blendOffWriteOff.Get(), NULL, 0xFFFFFFFF);
	dxContext.deviceContext->OMSetDepthStencilState(dxContext.depthWriteOnTestOnLEqual.Get(), 0);
}

void DX11Vendor::setRenderGeomtryState()
{
	dxContext.deviceContext->OMSetBlendState(dxContext.blendGBuffer.Get(), NULL, 0xFFFFFFFF);
	dxContext.deviceContext->OMSetDepthStencilState(dxContext.depthWriteOnTestOnLEqual.Get(), 0);
}

void DX11Vendor::setRenderOpaqueState()
{
	dxContext.deviceContext->OMSetBlendState(dxContext.blendOffWriteOn.Get(), NULL, 0xFFFFFFFF);
	dxContext.deviceContext->OMSetDepthStencilState(dxContext.depthWriteOnTestOnLEqual.Get(), 0);
}

void DX11Vendor::setRenderAlphaState()
{
	dxContext.deviceContext->OMSetBlendState(dxContext.blendOffWriteOnAlphaTest.Get(), NULL, 0xFFFFFFFF);
	dxContext.deviceContext->OMSetDepthStencilState(dxContext.depthWriteOnTestOnLEqual.Get(), 0);
}

void DX11Vendor::setRenderTransparentState()
{
	dxContext.deviceContext->OMSetBlendState(dxContext.blendOnWriteOn.Get(), NULL, 0xFFFFFFFF);
	dxContext.deviceContext->OMSetDepthStencilState(dxContext.depthWriteOffTestOnLEqual.Get(), 0);
}

void DX11Vendor::setRenderOverlayState()
{
	dxContext.deviceContext->OMSetBlendState(dxContext.blendOnWriteOn.Get(), NULL, 0xFFFFFFFF);
	dxContext.deviceContext->OMSetDepthStencilState(dxContext.depthWriteOffTestOffLEqual.Get(), 0);
}

void DX11Vendor::setRenderPostState()
{
	dxContext.deviceContext->OMSetBlendState(dxContext.blendOnWriteOn.Get(), NULL, 0xFFFFFFFF);
	dxContext.deviceContext->OMSetDepthStencilState(dxContext.depthWriteOffTestOffLEqual.Get(), 0);
}

void DX11Vendor::setRenderPostAddState()
{
	dxContext.deviceContext->OMSetBlendState(dxContext.blendAddWriteOn.Get(), NULL, 0xFFFFFFFF);
	dxContext.deviceContext->OMSetDepthStencilState(dxContext.depthWriteOffTestOffLEqual.Get(), 0);
}

void DX11Vendor::setRenderPostPremultiplyAlphaState()
{
	dxContext.deviceContext->OMSetBlendState(dxContext.blendPremultiplyAlphaWriteOn.Get(), NULL, 0xFFFFFFFF);
	dxContext.deviceContext->OMSetDepthStencilState(dxContext.depthWriteOffTestOffLEqual.Get(), 0);
}

void DX11Vendor::setRenderPostMultiplyState()
{
	dxContext.deviceContext->OMSetBlendState(dxContext.blendMultiplyWriteOn.Get(), NULL, 0xFFFFFFFF);
	dxContext.deviceContext->OMSetDepthStencilState(dxContext.depthWriteOffTestOffLEqual.Get(), 0);
}

void DX11Vendor::setRenderPostMaskState()
{
	dxContext.deviceContext->OMSetBlendState(dxContext.blendMaskWriteOn.Get(), NULL, 0xFFFFFFFF);
	dxContext.deviceContext->OMSetDepthStencilState(dxContext.depthWriteOffTestOffLEqual.Get(), 0);
}

void DX11Vendor::setCullState(CullType type)
{
	if (type == Cull_Back)
		dxContext.deviceContext->RSSetState(dxContext.rasterizerCullBack.Get());
	else if (type == Cull_Front)
		dxContext.deviceContext->RSSetState(dxContext.rasterizerCullFront.Get());
	else
		dxContext.deviceContext->RSSetState(dxContext.rasterizerCullOff.Get());
}

void DX11Vendor::setViewport(unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
	D3D11_VIEWPORT vp;
	vp.TopLeftX = x;
	vp.TopLeftY = y;
	vp.Width = w;
	vp.Height = h;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	dxContext.deviceContext->RSSetViewports(1, &vp);
}

void DX11Vendor::setMeshDrawContext()
{
	dxContext.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dxContext.deviceContext->IASetInputLayout(dxContext.meshInputLayout.Get());
}

void DX11Vendor::setSkeletonMeshDrawContext()
{
	dxContext.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dxContext.deviceContext->IASetInputLayout(dxContext.skeletonMeshInputLayout.Get());
}

void DX11Vendor::setTerrainDrawContext()
{
	dxContext.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	dxContext.deviceContext->IASetInputLayout(dxContext.terrainInputLayout.Get());
}

void DX11Vendor::meshDrawCall(const MeshPartDesc& mesh)
{
	if (mesh.meshData == NULL) {
		MeshData::currentMeshData = NULL;
	}
	else {
		mesh.meshData->bindShape();
	}
	dxContext.deviceContext->DrawIndexed(mesh.elementCount, mesh.elementFirst, mesh.vertexFirst);
}

void DX11Vendor::postProcessCall()
{
	//dxContext.deviceContext->Flush();
	MeshData::currentMeshData = NULL;
	DX11ShaderProgram::currentDx11Program->uploadDrawInfo();
	dxContext.deviceContext->IASetInputLayout(dxContext.screenInputLayout.Get());
	dxContext.deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	dxContext.deviceContext->Draw(4, 0);
	dxContext.deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DX11Vendor::readBackTexture2D(ITexture2D* texture, void* data)
{
	DX11Texture2D* dxTexture = dynamic_cast<DX11Texture2D*>(texture);
	if (dxTexture == NULL || data == NULL)
		return;
	Texture2DDesc desc = dxTexture->desc;
	desc.info.cpuAccessFlag = CAF_Read;
	desc.mipLevel = 1;
	desc.autoGenMip = false;
	desc.textureHandle = 0;
	desc.data = NULL;
	DX11Texture2D* buffer = (DX11Texture2D*)newTexture2D(desc);

	dxTexture->bind();
	buffer->bind();

	D3D11_BOX hSrcBox;
	hSrcBox.left = 0;
	hSrcBox.right = desc.width;
	hSrcBox.top = 0;
	hSrcBox.bottom = desc.height;
	hSrcBox.front = 0;
	hSrcBox.back = 1;
	dxContext.deviceContext->CopySubresourceRegion(buffer->dx11Texture2D.Get(), 0, 0, 0, 0,
		dxTexture->dx11Texture2D.Get(), 0, &hSrcBox);

	int bytesPerPixel = getPixelSize(desc.info.internalType, desc.channel);
	int bytesPerRow = desc.width * bytesPerPixel;

	D3D11_MAPPED_SUBRESOURCE mpd;
	dxContext.deviceContext->Map(buffer->dx11Texture2D.Get(), 0, D3D11_MAP_READ, 0, &mpd);
	for (int i = 0; i < desc.height; ++i) {
		memcpy_s((char*)data + bytesPerRow * i, bytesPerRow,
			(char*)mpd.pData + mpd.RowPitch * i, bytesPerRow);
	}
	dxContext.deviceContext->Unmap(buffer->dx11Texture2D.Get(), 0);
	delete buffer;
}

#endif // VENDOR_USE_DX11