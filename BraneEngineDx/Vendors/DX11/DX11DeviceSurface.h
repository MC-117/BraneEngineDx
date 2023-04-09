#pragma once
#include "../../Core/IRenderTarget.h"
#include "DX11.h"

class DX11DeviceSurface : public IDeviceSurface
{
public:
	DX11Context& dxContext;
	const unsigned int backBufferCount = 3;
	ComPtr<IDXGISwapChain1> swapChain;
	ComPtr<ID3D11Texture2D> backBuffer;
	ComPtr<ID3D11RenderTargetView> backBufferRTV;

	ComPtr<ID3D11Fence> fence = NULL;
	HANDLE frameLatencyWaitableObject = NULL;

	unsigned int maxFPS = 0;
	Time lastTime;
	Time duration;

	UINT64 fenceValue = 0;
	HANDLE fenceEvent;

	DX11DeviceSurface(DX11Context& context, DeviceSurfaceDesc& desc);
	virtual ~DX11DeviceSurface();

	virtual void bindSurface();
	virtual void resize(unsigned int width, unsigned int height, unsigned int multisampleLevel = 0);
	virtual void clearColor(const Color& color);
	virtual void swapBuffer(bool vsync, int maxFps);
	virtual void frameFence();
};