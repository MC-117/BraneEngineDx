#pragma once
#ifndef _DX11_H_
#define _DX11_H_

#include "../DX_Helper.h"
#include "../../Core/RenderCore/ShaderResourceFixBindingPoint.h"

#include <dxgi1_6.h>
#include <d3d11.h>
#include <d3d11_4.h>

struct DX11Context
{
	bool enableDebugLayer = false;
	HWND hWnd = NULL;
	ComPtr<IDXGIFactory4> dxgiFactory = NULL;
	ComPtr<ID3D11DeviceContext> deviceContext = NULL;
	ComPtr<ID3D11DeviceContext4> deviceContext4 = NULL;
	ComPtr<ID3D11Device> device = NULL;
	ComPtr<ID3D11Device5> device5 = NULL;

	ComPtr<ID3D11RasterizerState> rasterizerCullOff = NULL;
	ComPtr<ID3D11RasterizerState> rasterizerCullBack = NULL;
	ComPtr<ID3D11RasterizerState> rasterizerCullFront = NULL;
	// Render Order:
	ComPtr<ID3D11BlendState> blendOffWriteOff = NULL;                  //    0 - 500
	ComPtr<ID3D11BlendState> blendOffWriteOn = NULL;                   //  500 - 2449
	ComPtr<ID3D11BlendState> blendOffWriteOnAlphaTest = NULL;          // 2450 - 2499
	ComPtr<ID3D11BlendState> blendOnWriteOn = NULL;                    // 2500 -
	ComPtr<ID3D11BlendState> blendAddWriteOn = NULL;                   // Post
	ComPtr<ID3D11BlendState> blendPremultiplyAlphaWriteOn = NULL;      // Post
	ComPtr<ID3D11BlendState> blendMultiplyWriteOn = NULL;              // Post
	ComPtr<ID3D11BlendState> blendMaskWriteOn = NULL;                  // Post
	ComPtr<ID3D11BlendState> blendGBuffer = NULL;                      // GBuffer

	map<DepthStencilMode, ComPtr<ID3D11DepthStencilState>> depthStencilStateMap;
	
	ComPtr<ID3D11DepthStencilState> depthWriteOffTestOffLEqual = NULL; // 5000 - 

	ComPtr<ID3D11InputLayout> screenInputLayout = NULL;
	ComPtr<ID3D11InputLayout> meshInputLayout = NULL;
	ComPtr<ID3D11InputLayout> skeletonMeshInputLayout = NULL;
	ComPtr<ID3D11InputLayout> terrainInputLayout = NULL;

	uint64_t gpuFrequency = 0;

	void setHWnd(HWND hWnd);

	bool createDevice(unsigned int width, unsigned int height);
	void cleanupDevice();
	void fetchGPUFrrequency();
	void createRenderState();
	ComPtr<ID3D11DepthStencilState> getOrCreateDepthStencilState(DepthStencilMode mode);
	void createInputLayout();
	void cleanupRenderState();
	void cleanupInputLayout();

	void clearSRV();
	void clearUAV();
	void clearRTV();
};

template<class T>
vector<T*> toPtrVector(const vector<ComPtr<T>>& from)
{
	vector<T*> to;
	to.resize(from.size());
	for (int i = 0; i < from.size(); i++)
		to[i] = from[i].Get();
	return to;
}

#endif // !_DX11_H_
