#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>

#include <dxgi1_6.h>
#undef Serialize
#include "d3dx12.h"
#include <D3Dcompiler.h>

#include <wrl.h>
#include <string>
#include <shellapi.h>

#include "../../Core/Utility/Utility.h"

#define NULL_FORMAT (DXGI_FORMAT)0xfffffffe
#define SAMPLER_FORMAT (DXGI_FORMAT)0xfffffffd
#define CBV_FORMAT (DXGI_FORMAT)0xfffffffc

#define GRS_UPPER(A,B) ((UINT)(((A)+((B)-1))&~(B - 1)))

template <class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

enum struct DX12BlendFlags : uint8_t
{
	None = 0,
	Blend = 1,
	BlendDefault = 1,
	BlendAdd = 2,
	BlendPremultiplyAlpha = 3,
	BlendMultiply = 4,
	BlendMask = 5,
	Write = 8,
	AlphaTest = 16
};

enum struct DX12DepthFlags : uint8_t
{
	None = 0,
	Test = 1,
	Write = 2
};

enum struct DX12CullType : uint8_t
{
	Cull_Off, Cull_Back, Cull_Front
};

enum struct DX12InputLayoutType : uint8_t
{
	None = 0, Mesh = 1, SkeletonMesh = 2, Terrain = 3
};

struct DX12ShaderProgramData
{
	unsigned int programID = 0;
	ComPtr<ID3DBlob> VS;
	ComPtr<ID3DBlob> GS;
	ComPtr<ID3DBlob> HS;
	ComPtr<ID3DBlob> DS;
	ComPtr<ID3DBlob> PS;
	ComPtr<ID3DBlob> CS;
};