#pragma once
#ifndef _DX11_H_
#define _DX11_H_

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>

#include <dxgi1_6.h>
#include <d3d11.h>
#include <D3Dcompiler.h>

#include <wrl.h>
#include <string>
#include <shellapi.h>

#include "../../Core/Config.h"

struct DX11Context
{
	bool enableDebugLayer = true;
	HWND hWnd = NULL;
	IDXGIFactory1* dxgiFactory = NULL;
	ID3D11DeviceContext* deviceContext = NULL;
	ID3D11Device* device = NULL;
	IDXGISwapChain* swapChain = NULL;

	ID3D11RasterizerState* rasterizerCullOff = NULL;
	ID3D11RasterizerState* rasterizerCullBack = NULL;
	ID3D11RasterizerState* rasterizerCullFront = NULL;
	                                                            // Render Order:
	ID3D11BlendState* blendOffWriteOff = NULL;                  //    0 - 500
	ID3D11BlendState* blendOffWriteOn = NULL;                   //  500 - 2449
	ID3D11BlendState* blendOffWriteOnAlphaTest = NULL;          // 2450 - 2499
	ID3D11BlendState* blendOnWriteOn = NULL;                    // 2500 -

	ID3D11DepthStencilState* depthWriteOnTestOnLEqual = NULL;   //    0 - 2499
	ID3D11DepthStencilState* depthWriteOffTestOnLEqual = NULL;  // 2500 - 4999
	ID3D11DepthStencilState* depthWriteOffTestOffLEqual = NULL; // 5000 - 

	void setHWnd(HWND hWnd);

	bool createDevice(unsigned int width, unsigned int height);
	void cleanupDevice();
	void createSwapChain(unsigned int width, unsigned int height, unsigned int multisampleLevels);
	void createRenderState();
	void cleanupRenderState();

	void clearSRV();
	void clearUAV();
};

#endif // !_DX12_H_
