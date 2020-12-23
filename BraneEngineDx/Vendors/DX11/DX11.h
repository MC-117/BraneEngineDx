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

struct DX11Context
{
	IDXGIFactory1* dxgiFactory = NULL;
	ID3D11DeviceContext* deviceContext = NULL;
	ID3D11Device* device = NULL;
	IDXGISwapChain* swapChain = NULL;
};

#endif // !_DX12_H_
