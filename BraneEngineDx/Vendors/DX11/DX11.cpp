#include "DX11.h"
#include <iostream>
#include "../../Core/Console.h"

#define DEPTH_BIAS_D32_FLOAT(d) (d/(1/pow(2,23)))

void DX11Context::setHWnd(HWND hWnd)
{
	this->hWnd = hWnd;
}

bool DX11Context::createDevice(unsigned int width, unsigned int height)
{
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	UINT createDeviceFlags = 0;
	enableDebugLayer = true;
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
	createInputLayout();

	D3D11_QUERY_DESC queryDesc = { D3D11_QUERY_EVENT, 0 };
	device->CreateQuery(&queryDesc, endQuery.ReleaseAndGetAddressOf());
	return true;
}

void DX11Context::cleanupDevice()
{
	if (swapChain != NULL)
		swapChain->SetFullscreenState(false, nullptr);
	if (swapChain) {
		swapChain.Reset();
	}
	cleanupRenderState();
	cleanupInputLayout();
	if (dxgiFactory) {
		dxgiFactory.Reset();
	}
	if (device) {
		device.Reset();
	}
	if (deviceContext) {
		deviceContext.Reset();
	}
}

void DX11Context::createSwapChain(unsigned int width, unsigned int height, unsigned int multisampleLevels)
{
	/*if (swapChain != NULL) {
		swapChain->Release();
	}*/

	if (swapChain == NULL) {
		DXGI_SWAP_CHAIN_DESC1 sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = backBufferCount;
		sd.Width = width;
		sd.Height = height;
		sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		//sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		sd.Scaling = DXGI_SCALING_STRETCH;
		sd.Stereo = FALSE;

		if (multisampleLevels > 1) {
			unsigned int q = 0;
			device->CheckMultisampleQualityLevels(DXGI_FORMAT_B8G8R8A8_UNORM, multisampleLevels, &q);
			if (q != 0)
				--q;
			sd.SampleDesc.Quality = q;
		}

		ComPtr<IDXGISwapChain1> baseSwapChain = NULL;

		if (FAILED(dxgiFactory->CreateSwapChainForHwnd(device.Get(), hWnd, &sd, NULL, NULL, &baseSwapChain)))
			throw std::runtime_error("DX11: Create swap chain failed");

		baseSwapChain.As(&swapChain);

		//activeBackBufferIndex = swapChain->GetCurrentBackBufferIndex();
		//frameLatencyWaitableObject = swapChain->GetFrameLatencyWaitableObject();
	}
	else {
		swapChain->Present(0, 0);

		for (int i = 0; i < 3; i++) {
			if (i < 1) {
				backBuffer[i].Reset();
				backBufferRTV[i].Reset();
			}
		}

		if (FAILED(swapChain->ResizeBuffers(backBufferCount, width, height,
			DXGI_FORMAT_B8G8R8A8_UNORM, 0)))
			throw std::runtime_error("DX11: Resize swap chain failed");
		//frameLatencyWaitableObject = swapChain->GetFrameLatencyWaitableObject();
	}

	for (int i = 0; i < 3; i++) {
		if (i < 1) {
			swapChain->GetBuffer(i, IID_PPV_ARGS(backBuffer[i].GetAddressOf()));
			ComPtr<ID3D11RenderTargetView>& rtv = backBufferRTV[i];
			if (FAILED(device->CreateRenderTargetView(backBuffer[i].Get(), NULL, &rtv)))
				throw runtime_error("DX11: Create default render target view failed");
		}
		backBuffer[i].Reset();
	}
}

void DX11Context::createRenderState()
{
	D3D11_RASTERIZER_DESC rastDesc;
	ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FrontCounterClockwise = true;
	rastDesc.DepthClipEnable = true;
	//rastDesc.DepthBias = DEPTH_BIAS_D32_FLOAT(-0.0001);
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

	rt0.SrcBlend = D3D11_BLEND_ONE;
	rt0.DestBlend = D3D11_BLEND_ONE;
	rt0.SrcBlendAlpha = D3D11_BLEND_ZERO;
	rt0.DestBlendAlpha = D3D11_BLEND_ONE;
	device->CreateBlendState(&blendDesc, &blendAddWriteOn);

	rt0.SrcBlend = D3D11_BLEND_ONE;
	rt0.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rt0.SrcBlendAlpha = D3D11_BLEND_ONE;
	rt0.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	device->CreateBlendState(&blendDesc, &blendPremultiplyAlphaWriteOn);

	rt0.SrcBlend = D3D11_BLEND_DEST_COLOR;
	rt0.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rt0.SrcBlendAlpha = D3D11_BLEND_ZERO;
	rt0.DestBlendAlpha = D3D11_BLEND_ONE;
	device->CreateBlendState(&blendDesc, &blendMultiplyWriteOn);

	rt0.SrcBlend = D3D11_BLEND_ZERO;
	rt0.DestBlend = D3D11_BLEND_INV_SRC_COLOR;
	rt0.SrcBlendAlpha = D3D11_BLEND_ZERO;
	rt0.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	device->CreateBlendState(&blendDesc, &blendMaskWriteOn);

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

void DX11Context::createInputLayout()
{
	if (screenInputLayout == NULL) {
		const char* signatureShader = "void main(uint vertexID : SV_VertexID) { }";
		const size_t len = strlen(signatureShader);
		ComPtr<ID3DBlob> sigBlob;
		ComPtr<ID3DBlob> errorBlob;
		if (FAILED(D3DCompile(signatureShader, len, "SignatureShader", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &sigBlob, &errorBlob))) {
			throw runtime_error((const char*)errorBlob->GetBufferPointer());
		}
		const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[1] = {
			{ "SV_VertexID", 0, DXGI_FORMAT_R32_UINT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		if (FAILED(device->CreateInputLayout(inputLayoutDesc, 1, sigBlob->GetBufferPointer(),
			sigBlob->GetBufferSize(), &screenInputLayout))) {
			throw runtime_error("DX11: Create screen input layout failed");
		}
		/*sigBlob->Release();
		if (errorBlob != NULL)
			errorBlob->Release();*/
	}

	if (meshInputLayout == NULL) {
		const char* signatureShader =
			"void main(uint2 ins : INS, float3 pos : POSITION, float2 uv : TEXCOORD, float3 normal : NORMAL) { }";
		const size_t len = strlen(signatureShader);
		ComPtr<ID3DBlob> sigBlob;
		ComPtr<ID3DBlob> errorBlob;
		if (FAILED(D3DCompile(signatureShader, len, "SignatureShader", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &sigBlob, &errorBlob))) {
			throw runtime_error((const char*)errorBlob->GetBufferPointer());
		}
		const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[4] = {
			{ "INS", 0, DXGI_FORMAT_R32G32_UINT, 0, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		if (FAILED(device->CreateInputLayout(inputLayoutDesc, 4, sigBlob->GetBufferPointer(),
			sigBlob->GetBufferSize(), &meshInputLayout))) {
			throw runtime_error("DX11: Create mesh input layout failed");
		}
		/*sigBlob->Release();
		if (errorBlob != NULL)
			errorBlob->Release();*/
	}

	if (skeletonMeshInputLayout == NULL) {
		const char* signatureShader =
			"void main(uint2 ins : INS, float3 pos : POSITION, float2 uv : TEXCOORD, float3 normal : NORMAL, uint4 bondId : BONEINDEX, float4 weights : BONEWEIGHT) { }";
		const size_t len = strlen(signatureShader);
		ComPtr<ID3DBlob> sigBlob;
		ComPtr<ID3DBlob> errorBlob;
		if (FAILED(D3DCompile(signatureShader, len, "SignatureShader", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &sigBlob, &errorBlob))) {
			throw runtime_error((const char*)errorBlob->GetBufferPointer());
		}
		const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[6] = {
			{ "INS", 0, DXGI_FORMAT_R32G32_UINT, 0, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 4, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		if (FAILED(device->CreateInputLayout(inputLayoutDesc, 6, sigBlob->GetBufferPointer(),
			sigBlob->GetBufferSize(), &skeletonMeshInputLayout))) {
			throw runtime_error("DX11: Create skeleton mesh input layout failed");
		}
		/*if (sigBlob != NULL)
			sigBlob->Release();
		if (errorBlob != NULL)
			errorBlob->Release();*/
	}

	if (terrainInputLayout == NULL) {
		const char* signatureShader =
			"void main(uint2 ins : INS, float3 pos : POSITION, float2 uv : TEXCOORD) { }";
		const size_t len = strlen(signatureShader);
		ComPtr<ID3DBlob> sigBlob;
		ComPtr<ID3DBlob> errorBlob;
		if (FAILED(D3DCompile(signatureShader, len, "SignatureShader", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &sigBlob, &errorBlob))) {
			throw runtime_error((const char*)errorBlob->GetBufferPointer());
		}
		const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[3] = {
			{ "INS", 0, DXGI_FORMAT_R32G32_UINT, 0, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		if (FAILED(device->CreateInputLayout(inputLayoutDesc, 3, sigBlob->GetBufferPointer(),
			sigBlob->GetBufferSize(), &terrainInputLayout))) {
			throw runtime_error("DX11: Create terrain input layout failed");
		}
		/*if (sigBlob != NULL)
			sigBlob->Release();
		if (errorBlob != NULL)
			errorBlob->Release();*/
	}
}

void DX11Context::cleanupRenderState()
{
	if (rasterizerCullOff != NULL) {
		rasterizerCullOff.Reset();
	}
	if (rasterizerCullBack != NULL) {
		rasterizerCullBack.Reset();
	}
	if (rasterizerCullFront != NULL) {
		rasterizerCullFront.Reset();
	}
	if (blendOffWriteOff != NULL) {
		blendOffWriteOff.Reset();
	}
	if (blendOffWriteOn != NULL) {
		blendOffWriteOn.Reset();
	}
	if (blendOffWriteOnAlphaTest != NULL) {
		blendOffWriteOnAlphaTest.Reset();
	}
	if (blendOnWriteOn != NULL) {
		blendOnWriteOn.Reset();
	}
	if (blendAddWriteOn != NULL) {
		blendAddWriteOn.Reset();
	}
	if (depthWriteOnTestOnLEqual != NULL) {
		depthWriteOnTestOnLEqual.Reset();
	}
	if (depthWriteOffTestOnLEqual != NULL) {
		depthWriteOffTestOnLEqual.Reset();
	}
	if (depthWriteOffTestOffLEqual != NULL) {
		depthWriteOffTestOffLEqual.Reset();
	}
}

void DX11Context::cleanupInputLayout()
{
	if (screenInputLayout != NULL) {
		screenInputLayout.Reset();
	}
	if (meshInputLayout != NULL) {
		meshInputLayout.Reset();
	}
	if (skeletonMeshInputLayout != NULL) {
		skeletonMeshInputLayout.Reset();
	}
	if (terrainInputLayout != NULL) {
		terrainInputLayout.Reset();
	}
}

void DX11Context::swap(bool vsync, unsigned int maxFPS)
{
	this->maxFPS = maxFPS;

	deviceContext->End(endQuery.Get());

	swapChain->Present(vsync ? 1 : 0, 0);

	//DWORD result = WaitForSingleObjectEx(
	//	frameLatencyWaitableObject,
	//	1000, // 1 second timeout (shouldn't ever occur)
	//	true
	//);
	//activeBackBufferIndex = swapChain->GetCurrentBackBufferIndex();

	if (maxFPS == 0 || lastTime == 0) {
		duration = Time::now() - lastTime;
		lastTime = Time::now();
	}
	else {
		duration = Time::now() - lastTime;
		double milisec = 1000.0f / maxFPS;
		bool flush = false;
		/*double sleepTime = milisec - duration.toMillisecond();
		if (sleepTime > 0)
			Sleep(sleepTime);*/
		while (duration.toMillisecond() < milisec) {
			if (!flush) {
				//deviceContext->Flush();
				flush = true;
			}
			duration = Time::now() - lastTime;
		}
		lastTime = Time::now();
	}
}

void DX11Context::fence()
{
	if (endQuery == NULL)
		return;
	BOOL completed = false;

	do
	{
		HRESULT hr = deviceContext->GetData(endQuery.Get(), &completed, sizeof(BOOL), 0);
		if (hr == S_FALSE)
		{
		}
		else if (SUCCEEDED(hr) && completed)
		{
			break;
		}
		else
		{
			// error
			break;
		}
	} while (!completed);
}

void DX11Context::clearSRV()
{
	const int size = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - TEX_START_BIND_INDEX;
	ID3D11ShaderResourceView* srvs[size] = { NULL };
	deviceContext->VSSetShaderResources(TEX_START_BIND_INDEX, size, srvs);
	deviceContext->PSSetShaderResources(TEX_START_BIND_INDEX, size, srvs);
	deviceContext->GSSetShaderResources(TEX_START_BIND_INDEX, size, srvs);
	deviceContext->HSSetShaderResources(TEX_START_BIND_INDEX, size, srvs);
	deviceContext->DSSetShaderResources(TEX_START_BIND_INDEX, size, srvs);
	deviceContext->CSSetShaderResources(TEX_START_BIND_INDEX, size, srvs);
}

void DX11Context::clearUAV()
{
	ID3D11UnorderedAccessView* srvs[D3D11_PS_CS_UAV_REGISTER_COUNT] = { NULL };
	unsigned int offs[D3D11_PS_CS_UAV_REGISTER_COUNT] = { -1 };
	deviceContext->CSSetUnorderedAccessViews(0, D3D11_PS_CS_UAV_REGISTER_COUNT, srvs, offs);
}

void DX11Context::clearRTV()
{
	deviceContext->OMSetRenderTargets(0, NULL, NULL);
}
