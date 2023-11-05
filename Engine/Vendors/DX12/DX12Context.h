#pragma once

#include "DX12GraphicContext.h"
#include "DX12ResourceUploader.h"

struct DX12Context
{
	bool enableDebugLayer = false;
	HWND hWnd = NULL;
	ComPtr<ID3D12Device> device = NULL;
	ComPtr<ID3D12Debug> dx12Debug = NULL;

	ComPtr<IDXGIFactory4> dxgiFactory = NULL;
	ComPtr<IDXGISwapChain3> swapChain = NULL;
	HANDLE frameLatencyWaitableObject = NULL;

	unsigned int maxFPS = 0;
	Time lastTime;
	Time duration;

	static const int backBufferCount = 3;
	int activeBackBufferIndex = 0;

	uint64_t gpuFrequency = 0;

	// DescHeap
	/*DX12DescriptorHeap dynamicCSUViewDescHeap = DX12DescriptorHeap(50, false, true);
	DX12DescriptorHeap dynamicSamplerDescHeap = DX12DescriptorHeap(50, false, true);*/
	DX12DescriptorHeap backBufferRTVDescHeap = DX12DescriptorHeap(backBufferCount, true, true);

	DX12BackBuffer backBuffer[backBufferCount];

	D3D12_RESOURCE_BARRIER resourceBarrier[backBufferCount];
	ComPtr<ID3D12CommandQueue> uploadCommandQueue = NULL;
	ComPtr<ID3D12CommandQueue> commandQueue = NULL;
	ComPtr<ID3D12CommandQueue> computeCommandQueue = NULL;

	ComPtr<ID3D12Fence> fence;

	UINT64 fenceValues[backBufferCount];
	HANDLE fenceEvent;

	// Graphic Context
	DX12GraphicContext graphicContext;

	// Buffer Pool
	DX12BufferPool constantBufferPool;
	DX12BufferPool readBackBufferPool;
	DX12BufferPool meshBufferPool;
	DX12BufferPool textureBufferPool;
	DX12BufferPool textureMSBufferPool;

	// Reource Uploader
	DX12ResourceUploader resourceUploader;

	void setHWnd(HWND hWnd);
	bool createDevice(unsigned int width, unsigned int height);
	void cleanupDevice();
	void fetchGPUFrrequency();
	void createSwapChain(unsigned int width, unsigned int height, unsigned int multisampleLevels);

	void executeCommandQueue();
	void waitForGPU();
	void waitNextFrame();
	void resetFrame();
	void swap(bool vsync, unsigned int maxFPS);
};