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
	if (!createDevice(context._hwnd, context.screenSize.x, context.screenSize.y))
		return false;
	return true;
}

bool DX11Vendor::imGuiInit(const EngineConfig & config, const WindowContext & context)
{
	if (!ImGui_ImplWin32_Init(context._hwnd))
		return false;
	if (!ImGui_ImplDX11_Init(this->dxContext.device, this->dxContext.deviceContext))
		return false;
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
	dxContext.swapChain->Present(config.vsnyc ? 0 : 1, 0);
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
	cleanupDevice();
	return true;
}

bool DX11Vendor::guiOnlyRender(const Color & clearColor)
{
	
	return true;
}

bool DX11Vendor::resizeWindow(const EngineConfig & config, const WindowContext & context, unsigned int width, unsigned int height)
{
	cleanupRenderTarget();
	resizeSwapChain(context._hwnd, width, height);
	createRenderTarget();
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

ITexture2D * DX11Vendor::newTexture2D(Texture2DDesc& desc) const
{
	return new DX11Texture2D(dxContext, desc);
}

ShaderStage* DX11Vendor::newShaderStage(const ShaderStageDesc& desc) const
{
	return new DX11ShaderStage(dxContext, desc);
}

ShaderProgram* DX11Vendor::newShaderProgram() const
{
	return new DX11ShaderProgram(dxContext);
}

IMaterial* DX11Vendor::newMaterial(MaterialDesc& desc) const
{
	return new DX11Material(dxContext, desc);
}

IRenderTarget* DX11Vendor::newRenderTarget(RenderTargetDesc& desc) const
{
	return new DX11RenderTarget(dxContext, desc);
}

IGPUBuffer* DX11Vendor::newGPUBuffer(GPUBufferDesc& desc) const
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

bool DX11Vendor::createDevice(HWND hWnd, unsigned int width, unsigned int height)
{
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = g_backBufferNum;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	UINT createDeviceFlags = 0;
	if (enableDebugLayer)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL FeatureLevelsRequested = D3D_FEATURE_LEVEL_11_0;
	UINT numLevelsRequested = 1;
	D3D_FEATURE_LEVEL FeatureLevelsSupported;

	if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
			createDeviceFlags, &featureLevel, 1, D3D11_SDK_VERSION, &dxContext.device,
			&FeatureLevelsSupported, &dxContext.deviceContext)))
		return false;

	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&dxContext.dxgiFactory))))
		return false;

	dxContext.dxgiFactory->CreateSwapChain(dxContext.device, &sd, &dxContext.swapChain);

	createRenderTarget();
	return true;
}

void DX11Vendor::cleanupDevice()
{
	dxContext.swapChain->SetFullscreenState(false, nullptr);
	cleanupRenderTarget();
	if (dxContext.swapChain) {
		dxContext.swapChain->Release();
		dxContext.swapChain = NULL;
	}
	if (dxContext.dxgiFactory) {
		dxContext.dxgiFactory->Release();
		dxContext.dxgiFactory = NULL;
	}
	if (dxContext.device) {
		dxContext.device->Release();
		dxContext.device = NULL;
	}
	if (dxContext.deviceContext) {
		dxContext.deviceContext->Release();
		dxContext.deviceContext = NULL;
	}
}

void DX11Vendor::createRenderTarget()
{
	g_pBackBuffers.resize(g_backBufferNum);
	g_pRenderTargetViews.resize(g_backBufferNum);
	ID3D11Texture2D* pBackBuffer = NULL;
	ID3D11RenderTargetView* pRenderTargetView = NULL;
	for (UINT i = 0; i < g_backBufferNum; i++) {
		dxContext.swapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
		dxContext.deviceContext->OMSetRenderTargets(i, &pRenderTargetView, NULL);
		g_pBackBuffers[i] = pBackBuffer;
		g_pRenderTargetViews[i] = pRenderTargetView;
	}
}

void DX11Vendor::cleanupRenderTarget()
{
	for (UINT i = 0; i < g_backBufferNum; i++)
		if (g_pRenderTargetViews[i]) {
			g_pRenderTargetViews[i]->Release();
			g_pRenderTargetViews[i] = NULL;
		}
}

void DX11Vendor::resizeSwapChain(HWND hWnd, int width, int height)
{
	DXGI_SWAP_CHAIN_DESC sd;
	dxContext.swapChain->GetDesc(&sd);
	sd.OutputWindow = hWnd;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	dxContext.swapChain->Release();
	dxContext.dxgiFactory->CreateSwapChain(dxContext.device, &sd, &dxContext.swapChain);
}

#endif // VENDOR_USE_DX11