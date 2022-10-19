#pragma once
#ifndef _DX11_H_
#define _DX11_H_

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include <dxgi1_6.h>
#include <d3d11.h>
#include <D3Dcompiler.h>

#include <wrl.h>

#include "../../Core/Unit.h"

template <class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

struct DX11Context
{
	bool enableDebugLayer = false;
	HWND hWnd = NULL;
	ComPtr<IDXGIFactory4> dxgiFactory = NULL;
	ComPtr<ID3D11DeviceContext> deviceContext = NULL;
	ComPtr<ID3D11Device> device = NULL;
	ComPtr<IDXGISwapChain1> swapChain = NULL;
	HANDLE frameLatencyWaitableObject = NULL;

	unsigned int maxFPS = 0;
	Time lastTime;
	Time duration;

	int backBufferCount = 3;
	int activeBackBufferIndex = 0;
	ComPtr<ID3D11Texture2D> backBuffer[3] = { 0 };
	ComPtr<ID3D11RenderTargetView> backBufferRTV[3] = { 0 };

	ComPtr<ID3D11Query> endQuery = NULL;

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

	ComPtr<ID3D11DepthStencilState> depthWriteOnTestOnLEqual = NULL;   //    0 - 2499
	ComPtr<ID3D11DepthStencilState> depthWriteOffTestOnLEqual = NULL;  // 2500 - 4999
	ComPtr<ID3D11DepthStencilState> depthWriteOffTestOffLEqual = NULL; // 5000 - 

	ComPtr<ID3D11InputLayout> screenInputLayout = NULL;
	ComPtr<ID3D11InputLayout> meshInputLayout = NULL;
	ComPtr<ID3D11InputLayout> skeletonMeshInputLayout = NULL;
	ComPtr<ID3D11InputLayout> terrainInputLayout = NULL;

	void setHWnd(HWND hWnd);

	bool createDevice(unsigned int width, unsigned int height);
	void cleanupDevice();
	void createSwapChain(unsigned int width, unsigned int height, unsigned int multisampleLevels);
	void createRenderState();
	void createInputLayout();
	void cleanupRenderState();
	void cleanupInputLayout();

	void swap(bool vsync, unsigned int maxFPS);
	void fence();

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
