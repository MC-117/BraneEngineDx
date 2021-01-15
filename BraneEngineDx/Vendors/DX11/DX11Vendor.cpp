#include "DX11Vendor.h"
#include "DX11Texture2D.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#ifdef VENDOR_USE_DX11

REG_VENDOR_DEC(DX11Vendor);
REG_VENDOR_IMP(DX11Vendor, "DX11");

DX11Vendor::DX11Vendor() : StaticMeshData(dxContext)
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
	dxContext.setHWnd(context._hwnd);
	if (!dxContext.createDevice(context.screenSize.x, context.screenSize.y))
		return false;
	return true;
}

bool DX11Vendor::imGuiInit(const EngineConfig & config, const WindowContext & context)
{
	if (!ImGui_ImplWin32_Init(context._hwnd))
		return false;
	if (!ImGui_ImplDX11_Init(dxContext.device, dxContext.deviceContext))
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

bool DX11Vendor::swap(const EngineConfig & config, const WindowContext & context)
{
	dxContext.deviceContext->Flush();
	dxContext.swapChain->Present(config.vsnyc ? 1 : 0, 0);
	return true;
}

bool DX11Vendor::imGuiShutdown(const EngineConfig & config, const WindowContext & context)
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
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
	if (dxContext.swapChain) {
		dxContext.swapChain->SetFullscreenState(fullscreen, nullptr);
		return true;
	}
	return false;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
DX11Vendor::WndProcFunc DX11Vendor::getWndProcFunc()
{
	return ImGui_ImplWin32_WndProcHandler;
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
	dxContext.deviceContext->OMSetBlendState(dxContext.blendOffWriteOff, NULL, 0xFFFFFFFF);
	dxContext.deviceContext->OMSetDepthStencilState(dxContext.depthWriteOnTestOnLEqual, 0);
}

void DX11Vendor::setRenderGeomtryState()
{
	dxContext.deviceContext->OMSetBlendState(dxContext.blendOffWriteOn, NULL, 0xFFFFFFFF);
	dxContext.deviceContext->OMSetDepthStencilState(dxContext.depthWriteOnTestOnLEqual, 0);
}

void DX11Vendor::setRenderOpaqueState()
{
	dxContext.deviceContext->OMSetBlendState(dxContext.blendOffWriteOn, NULL, 0xFFFFFFFF);
	dxContext.deviceContext->OMSetDepthStencilState(dxContext.depthWriteOnTestOnLEqual, 0);
}

void DX11Vendor::setRenderAlphaState()
{
	dxContext.deviceContext->OMSetBlendState(dxContext.blendOffWriteOnAlphaTest, NULL, 0xFFFFFFFF);
	dxContext.deviceContext->OMSetDepthStencilState(dxContext.depthWriteOnTestOnLEqual, 0);
}

void DX11Vendor::setRenderTransparentState()
{
	dxContext.deviceContext->OMSetBlendState(dxContext.blendOnWriteOn, NULL, 0xFFFFFFFF);
	dxContext.deviceContext->OMSetDepthStencilState(dxContext.depthWriteOffTestOnLEqual, 0);
}

void DX11Vendor::setRenderOverlayState()
{
	dxContext.deviceContext->OMSetBlendState(dxContext.blendOnWriteOn, NULL, 0xFFFFFFFF);
	dxContext.deviceContext->OMSetDepthStencilState(dxContext.depthWriteOffTestOffLEqual, 0);
}

void DX11Vendor::setRenderPostState()
{
	dxContext.deviceContext->OMSetBlendState(dxContext.blendOnWriteOn, NULL, 0xFFFFFFFF);
	dxContext.deviceContext->OMSetDepthStencilState(dxContext.depthWriteOnTestOnLEqual, 0);
}

void DX11Vendor::setCullState(CullType type)
{
	if (type == Cull_Back)
		dxContext.deviceContext->RSSetState(dxContext.rasterizerCullBack);
	else if (type == Cull_Front)
		dxContext.deviceContext->RSSetState(dxContext.rasterizerCullFront);
	else
		dxContext.deviceContext->RSSetState(dxContext.rasterizerCullOff);
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

void DX11Vendor::postProcessCall()
{
	DX11ShaderProgram::currentDx11Program->uploadDrawInfo();
	dxContext.deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	dxContext.deviceContext->Draw(4, 0);
	dxContext.deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

#endif // VENDOR_USE_DX11