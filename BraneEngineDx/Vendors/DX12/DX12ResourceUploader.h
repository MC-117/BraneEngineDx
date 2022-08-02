#pragma once

#include "DX12DescriptorHeap.h"
#include "DX12CommandListDispatcher.h"
#include "DX12BufferPool.h"

class DX12ResourceUploader
{
public:
	ComPtr<ID3D12Device> device = NULL;
	DX12BufferPool uploadBufferPool;

	DX12ResourceUploader() = default;

	void init(ComPtr<ID3D12Device> device);
	void reset(int threadID);

	DX12CommandList* getCommandList();
	DX12SubBuffer* createUploadResource(UINT size);

	bool execute(ComPtr<ID3D12CommandQueue> queue);

	void uploadResource(DX12Buffer* buffer, int base, int size, D3D12_SUBRESOURCE_DATA* data,
		D3D12_RESOURCE_STATES transitionState);
	void generateMips(DX12Buffer* texture, DXGI_FORMAT format, int width, int height, int mips);
protected:
	DX12CommandListDispatcher commandListDispatcher;
	DX12CommandList* commandList = NULL;

	ComPtr<ID3DBlob> mipRootSignatureBlob;
	ComPtr<ID3D12RootSignature> mipRootSignature;
	ComPtr<ID3D12PipelineState> mipPipelineState;
	ComPtr<ID3DBlob> genMipShaderBlob;
	string genMipShaderCode;

	list<DX12SubBuffer*>* uploadBufferList = NULL;
	list<DX12DescriptorHeap*>* uploadDescHeapList = NULL;
		
	list<DX12SubBuffer*> uploadBufferList1;
	list<DX12SubBuffer*> uploadBufferList2;

	list<DX12DescriptorHeap*> uploadDescHeapList1;
	list<DX12DescriptorHeap*> uploadDescHeapList2;

	void initGenMipResource();
};

