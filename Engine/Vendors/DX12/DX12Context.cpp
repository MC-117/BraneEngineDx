#include "DX12Context.h"

void DX12Context::setHWnd(HWND hWnd)
{
    this->hWnd = hWnd;
}

bool DX12Context::createDevice(unsigned int width, unsigned int height)
{
	enableDebugLayer = false;
	if (enableDebugLayer)
	{
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&dx12Debug))))
		{
			dx12Debug->EnableDebugLayer();
		}
	}

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	if (D3D12CreateDevice(NULL, featureLevel, IID_PPV_ARGS(&device)) != S_OK)
		return false;

	// DescHeap
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		/*desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		desc.NodeMask = 1;
		dynamicCSUViewDescHeap.init(device, desc);

		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		desc.NodeMask = 1;
		dynamicSamplerDescHeap.init(device, desc);*/

		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 1;
		backBufferRTVDescHeap.init(device, desc);
	}

	{
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		if (FAILED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&uploadCommandQueue))))
			return false;
		if (FAILED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&commandQueue))))
			return false;

		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		if (FAILED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&computeCommandQueue))))
			return false;
	}

	{
		UINT dxgiFactoryFlags = 0;
		if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory))))
			return false;
	}


	if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
		return false;

	// Graphic Context
	graphicContext.init(device);
	graphicContext.reset(activeBackBufferIndex);

	// Buffer Pool
	constantBufferPool.init(device, CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, true);

	constantBufferPool.init(device, CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
		D3D12_HEAP_FLAG_NONE, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, true);

	meshBufferPool.init(device, CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, true);

	textureBufferPool.init(device, CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, true);

	textureMSBufferPool.init(device, CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES, D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT, true);

	// Reource Uploader
	resourceUploader.init(device);
	resourceUploader.reset(activeBackBufferIndex);

	fetchGPUFrrequency();

	resetFrame();

    return true;
}

void DX12Context::cleanupDevice()
{
	swapChain->SetFullscreenState(false, nullptr);
	CloseHandle(fenceEvent);
	fence.Reset();
	for (int i = 0; i < backBufferCount; i++) {
		//computeFence[i].Reset();
		backBuffer[i].release();
	}
	uploadCommandQueue.Reset();
	commandQueue.Reset();
	computeCommandQueue.Reset();
	backBufferRTVDescHeap.release();
	swapChain.Reset();
	dxgiFactory.Reset();
	dx12Debug.Reset();
	dx12Debug.Reset();
	device.Reset();
}

void DX12Context::fetchGPUFrrequency()
{
	if (FAILED(commandQueue->GetTimestampFrequency(&gpuFrequency)))
		throw runtime_error("GetTimestampFrequency failed");
}

void DX12Context::createSwapChain(unsigned int width, unsigned int height, unsigned int multisampleLevels)
{
	int currentRTVIndex = -1;
	if (swapChain == NULL) {
		DXGI_SWAP_CHAIN_DESC1 sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = backBufferCount;
		sd.Width = width;
		sd.Height = height;
		sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		sd.Scaling = DXGI_SCALING_STRETCH;
		sd.Stereo = FALSE;

		ComPtr<IDXGISwapChain1> baseSwapChain = NULL;

		if (FAILED(dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), hWnd, &sd, NULL, NULL, &baseSwapChain)))
			throw std::runtime_error("DX12: Create swap chain failed");

		baseSwapChain.As(&swapChain);
	}
	else {
		DX12Descriptor currentRTV = graphicContext.getRTV(0);
		for (int i = 0; i < backBufferCount; i++) {
			if (backBufferRTVDescHeap[i] == currentRTV) {
				graphicContext.setRTVs(0, NULL, DX12Descriptor());
				if (activeBackBufferIndex == i)
					currentRTVIndex = i;
			}
		}
		graphicContext.getCommandList()->get(true)->Close();
		graphicContext.getCommandList()->reset(device);
		//graphicContext.reset(activeBackBufferIndex);
		resourceUploader.reset(activeBackBufferIndex);
		waitForGPU();

		DX12CommandList* cmdLst = graphicContext.getCommandList();
		if (cmdLst != NULL && cmdLst->getUseCount() > 0) {
			cmdLst->get(false)->Close();
			cmdLst->reset(device);
		}

		for (int i = 0; i < backBufferCount; i++) {
			backBufferRTVDescHeap.reset(i);
			backBuffer[i].release();
			fenceValues[i] = fenceValues[activeBackBufferIndex];
		}

		if (FAILED(swapChain->ResizeBuffers(backBufferCount, width, height,
			DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT)))
			throw std::runtime_error("DX12: Resize swap chain failed");
	}

	activeBackBufferIndex = swapChain->GetCurrentBackBufferIndex();
	swapChain->SetMaximumFrameLatency(backBufferCount);
	if (frameLatencyWaitableObject != NULL)
		CloseHandle(frameLatencyWaitableObject);
	frameLatencyWaitableObject = swapChain->GetFrameLatencyWaitableObject();

	for (int i = 0; i < backBufferCount; i++) {
		ComPtr<ID3D12Resource> buffer;
		swapChain->GetBuffer(i, IID_PPV_ARGS(&buffer));
		backBuffer[i].init(buffer);
		backBuffer[i].get()->SetName((L"backBuffer" + to_wstring(i)).c_str());
		backBufferRTVDescHeap.updateRTV(backBuffer[i], i);
	}

	if (currentRTVIndex >= 0) {
		graphicContext.beginRender(activeBackBufferIndex, D3D12_COMMAND_LIST_TYPE_DIRECT);
		DX12Descriptor rtv = backBufferRTVDescHeap[activeBackBufferIndex];
		graphicContext.setRTVs(1, &rtv, DX12Descriptor());

		/*D3D12_RESOURCE_BARRIER barrier = {};

		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
		barrier.Transition.pResource = backBuffer[activeBackBufferIndex].Get();
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		graphicContext.getCommandList()->get(false)->ResourceBarrier(1, &barrier);*/
	}
}

void DX12Context::executeCommandQueue()
{
	if (resourceUploader.execute(uploadCommandQueue)) {
		uploadCommandQueue->Signal(fence.Get(), fenceValues[activeBackBufferIndex]);
		computeCommandQueue->Wait(fence.Get(), fenceValues[activeBackBufferIndex]);
	}
	
	if (graphicContext.executeComputeCommandList(computeCommandQueue)) {
		computeCommandQueue->Signal(fence.Get(), fenceValues[activeBackBufferIndex]);
		commandQueue->Wait(fence.Get(), fenceValues[activeBackBufferIndex]);
	}

	if (graphicContext.executeCommandList(commandQueue)) {

	}
}

void DX12Context::waitForGPU()
{
	UINT64 fenceValueForSignal = fenceValues[activeBackBufferIndex];
	commandQueue->Signal(fence.Get(), fenceValueForSignal);

	if (fence->GetCompletedValue() < fenceValues[activeBackBufferIndex])
	{
		fence->SetEventOnCompletion(fenceValueForSignal, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	fenceValues[activeBackBufferIndex] = fenceValueForSignal + 1;
}

void DX12Context::waitNextFrame()
{
	// Schedule a Signal command in the queue.
	const UINT64 currentFenceValue = fenceValues[activeBackBufferIndex];
	commandQueue->Signal(fence.Get(), currentFenceValue);

	// Update the frame index.
	activeBackBufferIndex = swapChain->GetCurrentBackBufferIndex();

	HANDLE waitableObjects[] = { frameLatencyWaitableObject, NULL };
	DWORD numWaitableObjects = 1;

	// If the next frame is not ready to be rendered yet, wait until it is ready.
	if (fence->GetCompletedValue() < fenceValues[activeBackBufferIndex])
	{
		fence->SetEventOnCompletion(fenceValues[activeBackBufferIndex], fenceEvent);
		waitableObjects[1] = fenceEvent;
		numWaitableObjects = 2;
	}

	WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

	// Set the fence value for the next frame.
	fenceValues[activeBackBufferIndex] = currentFenceValue + 1;
}

void DX12Context::resetFrame()
{
	graphicContext.reset(activeBackBufferIndex);
	resourceUploader.reset(activeBackBufferIndex);

	graphicContext.beginRender(activeBackBufferIndex, D3D12_COMMAND_LIST_TYPE_DIRECT);
}

void DX12Context::swap(bool vsync, unsigned int maxFPS)
{
	this->maxFPS = maxFPS;

	executeCommandQueue();

	swapChain->Present(vsync ? 1 : 0, 0);

	waitNextFrame();

	resetFrame();

	backBuffer[activeBackBufferIndex].transitionBarrier(
		graphicContext.getCommandList()->get(false),
		D3D12_RESOURCE_STATE_RENDER_TARGET);

	if (maxFPS == 0 || lastTime == 0) {
		duration = Time::now() - lastTime;
		lastTime = Time::now();
	}
	else {
		duration = Time::now() - lastTime;
		double milisec = 1000.0f / maxFPS;
		/*double sleepTime = milisec - duration.toMillisecond();
		if (sleepTime > 0)
			Sleep(sleepTime);*/
		while (duration.toMillisecond() < milisec) {
			duration = Time::now() - lastTime;
		}
		lastTime = Time::now();
	}
}
