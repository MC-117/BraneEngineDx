#include "DX11.h"
#include <iostream>
#include "../../Core/Console.h"
#include "../../Core/Engine.h"

#define DEPTH_BIAS_D32_FLOAT(d) (d/(1/pow(2,23)))

void DX11Context::setHWnd(HWND hWnd)
{
	this->hWnd = hWnd;
}

bool DX11Context::createDevice(unsigned int width, unsigned int height)
{
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	UINT createDeviceFlags = 0;
	int dxDebugLayerFlag = 0;
	Engine::engineConfig.configInfo.get("dxDebugLayerFlag", dxDebugLayerFlag);
	enableDebugLayer = dxDebugLayerFlag;
	if (enableDebugLayer)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL FeatureLevelsRequested = D3D_FEATURE_LEVEL_11_0;
	UINT numLevelsRequested = 1;
	D3D_FEATURE_LEVEL FeatureLevelsSupported;

	if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
		createDeviceFlags, &featureLevel, 1, D3D11_SDK_VERSION, &device,
		&FeatureLevelsSupported, &deviceContext)))
		return false;

	if (FAILED(device->QueryInterface<ID3D11Device5>(&device5)))
		return false;

	if (FAILED(deviceContext->QueryInterface<ID3D11DeviceContext4>(&deviceContext4)))
		return false;

	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory))))
		return false;

	createRenderState();
	createInputLayout();

	fetchGPUFrrequency();

	return true;
}

void DX11Context::cleanupDevice()
{
	cleanupRenderState();
	cleanupInputLayout();
	if (dxgiFactory) {
		dxgiFactory.Reset();
	}
	if (deviceContext4) {
		deviceContext4.Reset();
	}
	if (deviceContext) {
		deviceContext.Reset();
	}
	if (device5) {
		device5.Reset();
	}
	if (device) {
		device.Reset();
	}
}

void DX11Context::fetchGPUFrrequency()
{
	D3D11_QUERY_DESC queryDesc;
	queryDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
	queryDesc.MiscFlags = 0;
	ComPtr<ID3D11Query> freqQuery;
	device->CreateQuery(&queryDesc, freqQuery.GetAddressOf());
	int tryCount = 0;
	bool isSuccessed = false;
	D3D11_QUERY_DATA_TIMESTAMP_DISJOINT freqQueryData;
	while (!isSuccessed && tryCount < 10)
	{
		deviceContext->Begin(freqQuery.Get());
		deviceContext->End(freqQuery.Get());

		freqQueryData.Frequency = 0;
		freqQueryData.Disjoint = 0;
		const Time startTime = Time::now();
		do {
			this_thread::sleep_for(5ms);
			if (SUCCEEDED(deviceContext->GetData(freqQuery.Get(), &freqQueryData, sizeof(freqQueryData), 0)))
			{
				if (freqQueryData.Disjoint)
				{
					isSuccessed = true;
					break;
				}
			}
		}
		while ((Time::now() - startTime) < 0.5s);

		tryCount++;
	}

	if (freqQueryData.Frequency == 0)
		throw runtime_error("Fetch gpu frequency failed");

	gpuFrequency = freqQueryData.Frequency;
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
}

static D3D11_BLEND_OP blendOpMap[5] = {
	D3D11_BLEND_OP_ADD,
	D3D11_BLEND_OP_SUBTRACT,
	D3D11_BLEND_OP_REV_SUBTRACT,
	D3D11_BLEND_OP_MIN,
	D3D11_BLEND_OP_MAX
};

static D3D11_BLEND blendFactorMap[16] = {
	D3D11_BLEND_ZERO,
	D3D11_BLEND_ONE,
	D3D11_BLEND_SRC_COLOR,
	D3D11_BLEND_INV_SRC_COLOR,
	D3D11_BLEND_SRC_ALPHA,
	D3D11_BLEND_INV_SRC_ALPHA,
	D3D11_BLEND_DEST_ALPHA,
	D3D11_BLEND_INV_DEST_ALPHA,
	D3D11_BLEND_DEST_COLOR,
	D3D11_BLEND_INV_DEST_COLOR,
	D3D11_BLEND_BLEND_FACTOR,
	D3D11_BLEND_INV_BLEND_FACTOR,
	D3D11_BLEND_SRC1_COLOR,
	D3D11_BLEND_INV_SRC1_COLOR,
	D3D11_BLEND_SRC1_ALPHA,
	D3D11_BLEND_INV_SRC1_ALPHA
};

ComPtr<ID3D11BlendState> DX11Context::getOrCreateBlendState(const RenderTargetModes& modes)
{
	auto iter = blendStateMap.find(modes);
	if (iter != blendStateMap.end())
		return iter->second;

	D3D11_BLEND_DESC desc;
	ZeroMemory(&desc, sizeof desc);

	desc.IndependentBlendEnable = true;

	for (int index = 0; index < MaxRenderTargets; index++) {
		const RenderTargetMode& mode = modes[index];
		auto& rt = desc.RenderTarget[index];

		desc.AlphaToCoverageEnable |= mode.enableAlphaTest;

		rt.BlendEnable = mode.enableBlend;
		rt.BlendOp = blendOpMap[mode.colorOp];
		rt.BlendOpAlpha = blendOpMap[mode.alphaOp];
		rt.SrcBlend = blendFactorMap[mode.srcColorFactor];
		rt.DestBlend = blendFactorMap[mode.destColorFactor];
		rt.SrcBlendAlpha = blendFactorMap[mode.srcAlphaFactor];
		rt.DestBlendAlpha = blendFactorMap[mode.destAlphaFactor];
		rt.RenderTargetWriteMask = mode.writeMask;
	}

	ComPtr<ID3D11BlendState> blendState;
	device->CreateBlendState(&desc, &blendState);

	blendStateMap.emplace(modes, blendState);
	return blendState;
}

ComPtr<ID3D11DepthStencilState> DX11Context::getOrCreateDepthStencilState(DepthStencilMode mode)
{
	auto iter = depthStencilStateMap.find(mode);
	if (iter != depthStencilStateMap.end())
		return iter->second;
	
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof dsDesc);

	ComPtr<ID3D11DepthStencilState> depthStencilState;

	dsDesc.DepthEnable = mode.depthTest;
	dsDesc.DepthWriteMask = mode.depthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = (D3D11_COMPARISON_FUNC)(mode.depthComparion + 1);

	dsDesc.StencilEnable = mode.stencilTest;
	dsDesc.StencilReadMask = mode.stencilReadMask;
	dsDesc.StencilWriteMask = mode.stencilWriteMask;
	
	dsDesc.FrontFace.StencilFailOp = (D3D11_STENCIL_OP)(mode.stencilFailedOp_front + 1);
	dsDesc.FrontFace.StencilDepthFailOp = (D3D11_STENCIL_OP)(mode.stencilDepthFailedOp_front + 1);
	dsDesc.FrontFace.StencilPassOp = (D3D11_STENCIL_OP)(mode.stencilPassOp_front + 1);
	dsDesc.FrontFace.StencilFunc = (D3D11_COMPARISON_FUNC)(mode.stencilComparion_front + 1);
	
	dsDesc.BackFace.StencilFailOp = (D3D11_STENCIL_OP)(mode.stencilFailedOp_back + 1);
	dsDesc.BackFace.StencilDepthFailOp = (D3D11_STENCIL_OP)(mode.stencilDepthFailedOp_back + 1);
	dsDesc.BackFace.StencilPassOp = (D3D11_STENCIL_OP)(mode.stencilPassOp_back + 1);
	dsDesc.BackFace.StencilFunc = (D3D11_COMPARISON_FUNC)(mode.stencilComparion_back + 1);

	device->CreateDepthStencilState(&dsDesc, &depthStencilState);

	depthStencilStateMap.emplace(mode, depthStencilState);

	return depthStencilState;
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

	blendStateMap.clear();
	depthStencilStateMap.clear();
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
	unsigned int offs[D3D11_PS_CS_UAV_REGISTER_COUNT] = { ~0u };
	deviceContext->CSSetUnorderedAccessViews(0, D3D11_PS_CS_UAV_REGISTER_COUNT, srvs, offs);
}

void DX11Context::clearRTV()
{
	deviceContext->OMSetRenderTargets(0, NULL, NULL);
}
