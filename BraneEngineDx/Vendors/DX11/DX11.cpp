#include "DX11.h"
#include <iostream>

void DX11Context::setHWnd(HWND hWnd)
{
	this->hWnd = hWnd;
}

bool DX11Context::createDevice(unsigned int width, unsigned int height)
{
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	UINT createDeviceFlags = 0;
	if (enableDebugLayer)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL FeatureLevelsRequested = D3D_FEATURE_LEVEL_11_0;
	UINT numLevelsRequested = 1;
	D3D_FEATURE_LEVEL FeatureLevelsSupported;

	if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
		createDeviceFlags, &featureLevel, 1, D3D11_SDK_VERSION, &device,
		&FeatureLevelsSupported, &deviceContext)))
		return false;

	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory))))
		return false;
	createRenderState();
	return true;
}

void DX11Context::cleanupDevice()
{
	swapChain->SetFullscreenState(false, nullptr);
	if (swapChain) {
		swapChain->Release();
		swapChain = NULL;
	}
	cleanupRenderState();
	if (dxgiFactory) {
		dxgiFactory->Release();
		dxgiFactory = NULL;
	}
	if (device) {
		device->Release();
		device = NULL;
	}
	if (deviceContext) {
		deviceContext->Release();
		deviceContext = NULL;
	}
}

void DX11Context::createSwapChain(unsigned int width, unsigned int height, unsigned int multisampleLevels)
{
	if (swapChain != NULL)
		swapChain->Release();
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = multisampleLevels;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	if (multisampleLevels > 1) {
		unsigned int q = 0;
		device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, multisampleLevels, &q);
		if (q != 0)
			--q;
		sd.SampleDesc.Quality = q;
	}
	if (FAILED(dxgiFactory->CreateSwapChain(device, &sd, &swapChain)))
		throw std::runtime_error("DX11: Create swap chain failed");
}

void DX11Context::createRenderState()
{
	D3D11_RASTERIZER_DESC rastDesc;
	ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FrontCounterClockwise = true;
	rastDesc.DepthClipEnable = true;
	device->CreateRasterizerState(&rastDesc, &rasterizerCullOff);

	rastDesc.CullMode = D3D11_CULL_BACK;
	device->CreateRasterizerState(&rastDesc, &rasterizerCullBack);

	rastDesc.CullMode = D3D11_CULL_FRONT;
	device->CreateRasterizerState(&rastDesc, &rasterizerCullFront);

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	auto& rt0 = blendDesc.RenderTarget[0];
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	rt0.BlendEnable = false;
	rt0.SrcBlend = D3D11_BLEND_ZERO;
	rt0.DestBlend = D3D11_BLEND_ONE;
	rt0.BlendOp = D3D11_BLEND_OP_ADD;
	rt0.SrcBlendAlpha = D3D11_BLEND_ZERO;
	rt0.DestBlendAlpha = D3D11_BLEND_ONE;
	rt0.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rt0.RenderTargetWriteMask = 0;
	device->CreateBlendState(&blendDesc, &blendOffWriteOff);

	rt0.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&blendDesc, &blendOffWriteOn);

	blendDesc.AlphaToCoverageEnable = true;
	device->CreateBlendState(&blendDesc, &blendOffWriteOnAlphaTest);

	blendDesc.AlphaToCoverageEnable = false;
	rt0.BlendEnable = true;
	rt0.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rt0.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rt0.BlendOp = D3D11_BLEND_OP_ADD;
	rt0.SrcBlendAlpha = D3D11_BLEND_ONE;
	rt0.DestBlendAlpha = D3D11_BLEND_ZERO;
	rt0.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	device->CreateBlendState(&blendDesc, &blendOnWriteOn);

	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof dsDesc);
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&dsDesc, &depthWriteOnTestOnLEqual);

	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	device->CreateDepthStencilState(&dsDesc, &depthWriteOffTestOnLEqual);

	dsDesc.DepthEnable = false;
	device->CreateDepthStencilState(&dsDesc, &depthWriteOffTestOffLEqual);
}

void DX11Context::cleanupRenderState()
{
	if (rasterizerCullOff != NULL)
		rasterizerCullOff->Release();
	if (rasterizerCullBack != NULL)
		rasterizerCullBack->Release();
	if (rasterizerCullFront != NULL)
		rasterizerCullFront->Release();
	if (blendOffWriteOff != NULL)
		blendOffWriteOff->Release();
	if (blendOffWriteOn != NULL)
		blendOffWriteOn->Release();
	if (blendOffWriteOnAlphaTest != NULL)
		blendOffWriteOnAlphaTest->Release();
	if (blendOnWriteOn != NULL)
		blendOnWriteOn->Release();
	if (depthWriteOnTestOnLEqual != NULL)
		depthWriteOnTestOnLEqual->Release();
	if (depthWriteOffTestOnLEqual != NULL)
		depthWriteOffTestOnLEqual->Release();
	if (depthWriteOffTestOffLEqual != NULL)
		depthWriteOffTestOffLEqual->Release();
}
