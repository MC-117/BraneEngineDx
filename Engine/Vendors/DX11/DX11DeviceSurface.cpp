#include "DX11DeviceSurface.h"

DX11DeviceSurface::DX11DeviceSurface(DX11Context& context, DeviceSurfaceDesc& desc)
	: dxContext(context), IDeviceSurface(desc)
{
}

DX11DeviceSurface::~DX11DeviceSurface()
{
	backBufferRTV.Reset();
	backBuffer.Reset();
	swapChain.Reset();
	fence.Reset();
}

void DX11DeviceSurface::bindSurface()
{
	if (!desc.inited) {
		resize(desc.width, desc.height, desc.multisampleLevel);
	}
	dxContext.deviceContext->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), NULL);
}

void DX11DeviceSurface::resize(unsigned int width, unsigned int height, unsigned int multisampleLevel)
{
	if (desc.windowHandle == NULL)
		throw runtime_error("windowHandle is invalid when resizing DX11DeviceSurface");

	bool recreate = true;
	multisampleLevel = desc.multisampleLevel;
	multisampleLevel = multisampleLevel == 0 ? 1 : multisampleLevel;

	if (fence == NULL) {
		if (FAILED(dxContext.device5->CreateFence(fenceValue, D3D11_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
			throw std::runtime_error("DX11: Create fence failed");
	}

	if (swapChain == NULL || desc.multisampleLevel != multisampleLevel) {
		if (swapChain)
			swapChain.Reset();
		DXGI_SWAP_CHAIN_DESC1 sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = backBufferCount;
		sd.Width = width;
		sd.Height = height;
		sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER;
		//sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		sd.Scaling = DXGI_SCALING_STRETCH;
		sd.Stereo = FALSE;
		/*sd.BufferCount = backBufferCount;
		sd.BufferDesc.Width = 0;
		sd.BufferDesc.Height = 0;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = (HWND)desc.windowHandle;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;
		sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;*/

		if (multisampleLevel > 1) {
			unsigned int q = 0;
			dxContext.device->CheckMultisampleQualityLevels(DXGI_FORMAT_B8G8R8A8_UNORM, multisampleLevel, &q);
			if (q != 0)
				--q;
			sd.SampleDesc.Quality = q;
		}

		//ComPtr<IDXGISwapChain1> baseSwapChain = NULL;

		if (FAILED(dxContext.dxgiFactory->CreateSwapChainForHwnd(dxContext.device.Get(), (HWND)desc.windowHandle, &sd, NULL, NULL, &swapChain)))
			throw std::runtime_error("DX11: Create swap chain failed");
		/*if (FAILED(dxContext.dxgiFactory->CreateSwapChain(dxContext.device.Get(), &sd, &baseSwapChain)))
			throw std::runtime_error("DX11: Create swap chain failed");*/

		//baseSwapChain.As(&swapChain);

		//activeBackBufferIndex = swapChain->GetCurrentBackBufferIndex();
		//frameLatencyWaitableObject = swapChain->GetFrameLatencyWaitableObject();
	}
	else if (desc.width != width || desc.height != height) {
		//swapChain->Present(0, 0);

		backBuffer.Reset();
		backBufferRTV.Reset();

		if (FAILED(swapChain->ResizeBuffers(backBufferCount, width, height,
			DXGI_FORMAT_B8G8R8A8_UNORM, 0)))
			throw std::runtime_error("DX11: Resize swap chain failed");
		//frameLatencyWaitableObject = swapChain->GetFrameLatencyWaitableObject();
	}
	else recreate = false;

	if (recreate) {
		swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
		if (FAILED(dxContext.device->CreateRenderTargetView(backBuffer.Get(), NULL, &backBufferRTV)))
			throw runtime_error("DX11: Create default render target view failed");

		desc.width = width;
		desc.height = height;
		desc.multisampleLevel = multisampleLevel;
	}

	desc.inited = true;
}

void DX11DeviceSurface::clearColor(const Color& color)
{
	dxContext.deviceContext->ClearRenderTargetView(backBufferRTV.Get(), (const float*)&color);
}

void DX11DeviceSurface::swapBuffer(bool vsync, int maxFps)
{
	if (!desc.inited || swapChain == NULL) {
		resize(desc.width, desc.height, desc.multisampleLevel);
	}
	//fenceValue++;

	//dxContext.deviceContext4->Signal(fence.Get(), fenceValue);

	swapChain->Present(vsync ? 1 : 0, 0);

	//DWORD result = WaitForSingleObjectEx(
	//	frameLatencyWaitableObject,
	//	1000, // 1 second timeout (shouldn't ever occur)
	//	true
	//);
	//activeBackBufferIndex = swapChain->GetCurrentBackBufferIndex();

	if (maxFps == 0 || lastTime == 0) {
		duration = Time::now() - lastTime;
		lastTime = Time::now();
	}
	else {
		duration = Time::now() - lastTime;
		double milisec = 1000.0f / maxFps;
		while (duration.toMillisecond() < milisec) {
			this_thread::yield();
			duration = Time::now() - lastTime;
		}
		lastTime = Time::now();
	}
}

void DX11DeviceSurface::frameFence()
{
	/*if (fence && fence->GetCompletedValue() < fenceValue)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		fence->SetEventOnCompletion(fenceValue, eventHandle);
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}*/
}
