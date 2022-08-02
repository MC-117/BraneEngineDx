#pragma once
#include "DX12DescriptorHeap.h"

class DX12Buffer;
class DX12BufferBlock;
class DX12BufferPool;

struct DX12ResourceBarrier
{
	UINT subresource;
	D3D12_RESOURCE_STATES state = (D3D12_RESOURCE_STATES)-1;
	D3D12_RESOURCE_STATES transitingState = (D3D12_RESOURCE_STATES)-1;
};

class DX12SubBuffer
{
	friend DX12Buffer;
	friend DX12BufferBlock;
	friend DX12BufferPool;
public:
	ComPtr<ID3D12Resource> get();
	void release();
	bool isUnused() const;

	void upload(void* data, int size, int first);
	void read(void* data, int size, int first);
	DX12ResourceView getCBV();
	// For normal texture format buffer
	DX12ResourceView getBufferSRV(DXGI_FORMAT format);
	// For structured buffer
	DX12ResourceView getStructSRV();

	int getSize() const;
	int getStride() const;
	int getOffset() const;
	int getFirstElement() const;
	int getNumElements() const;
protected:
	DX12Buffer* buffer = NULL;
	DX12SubBuffer* last = NULL;
	DX12SubBuffer* next = NULL;

	void* subBufferPointer = NULL;

	int base = 0;
	int size = 0;

	DX12SubBuffer() = default;

	bool tryAllocate(int size);
	DX12SubBuffer* allocate(int size);
	void deallocate();
};

class DX12Buffer
{
	friend DX12SubBuffer;
	friend DX12BufferBlock;
	friend DX12BufferPool;
public:
	ComPtr<ID3D12Resource> get();
	void release();
	bool isUnused() const;

	int getSize() const;
	int getStride() const;

	void transitionBarrier(ComPtr<ID3D12GraphicsCommandList> cmdLst, D3D12_RESOURCE_STATES newState, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
	void beginTransitionBarrier(ComPtr<ID3D12GraphicsCommandList> cmdLst, D3D12_RESOURCE_STATES newState, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
	void endTransitionBarrier(ComPtr<ID3D12GraphicsCommandList> cmdLst, UINT subresouces = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

	bool CreateResource(const D3D12_RESOURCE_DESC& pDesc, D3D12_RESOURCE_STATES InitialState, unsigned int stride = 0);
	bool CreateResource(const D3D12_RESOURCE_DESC& pDesc, D3D12_RESOURCE_STATES InitialState, Vector4f clearValue, unsigned int stride = 0);

	void* map();
	void unmap();
protected:
	DX12BufferBlock* block = NULL;
	DX12Buffer* last = NULL;
	DX12Buffer* next = NULL;

	DX12SubBuffer* rootSubBuffer = NULL;
	DX12SubBuffer* currentSubBuffer = NULL;

	DX12Buffer* lastNull = NULL;
	DX12Buffer* nextNull = NULL;
	int base = 0;
	int size = 0;
	int stride = 0;
	ComPtr<ID3D12Resource> buffer;
	std::map<UINT, DX12ResourceBarrier> barriers;

	void* mappedPointer = NULL;

	DX12Buffer() = default;

	DX12ResourceBarrier& getBarrier(UINT subresource);

	bool tryAllocate(int size);
	DX12Buffer* allocate(int size);

	bool trySuballocate(int size, int stride);
	DX12SubBuffer* suballocate(int size, int stride);

	void tryDeallocate();
	void deallocate();
};

class DX12BackBuffer : public DX12Buffer
{
public:
	DX12BackBuffer() = default;
	DX12BackBuffer(ComPtr<ID3D12Resource> backBuffer);

	void init(ComPtr<ID3D12Resource> backBuffer);

	void release();

	bool CreateResource(const D3D12_RESOURCE_DESC& pDesc, D3D12_RESOURCE_STATES InitialState, unsigned int stride = 0) = delete;
	bool CreateResource(const D3D12_RESOURCE_DESC& pDesc, D3D12_RESOURCE_STATES InitialState, Vector4f clearValue, unsigned int stride = 0) = delete;

	void* map() = delete;
	void unmap() = delete;
};

class DX12BufferBlock
{
	friend DX12Buffer;
	friend DX12BufferPool;
public:
protected:
	DX12BufferPool* pool = NULL;
	DX12BufferBlock* next = NULL;
	int size = 0;
	ComPtr<ID3D12Heap> heap = NULL;

	DX12Buffer* rootNode = NULL;
	DX12Buffer* currentNode = NULL;

	DX12BufferBlock() = default;
	void init(int size);
	bool tryAllocate(int size);
	DX12Buffer* allocate(int size, ComPtr<ID3D12Device> device, const D3D12_HEAP_PROPERTIES& props, D3D12_HEAP_FLAGS flags, UINT alignment);

	bool trySuballocate(int size, int stride);
	DX12SubBuffer* suballocate(int size, int stride, ComPtr<ID3D12Device> device, const D3D12_HEAP_PROPERTIES& props, D3D12_HEAP_FLAGS flags, UINT alignment);
	void tryDeallocate();
};

const int ONE_K_1_CHNL = 1024 * 1024; // 1MB
const int TWO_K_1_CHNL = 2048 * 2048; // 4MB
const int FOUR_K_1_CHNL = 4096 * 4096; // 16MB
const int EIGHT_K_1_CHNL = 8192 * 8192; // 64MB

const int ONE_K_4_CHNL = ONE_K_1_CHNL * 4; // 4MB
const int TWO_K_4_CHNL = TWO_K_1_CHNL * 4; // 16MB
const int FOUR_K_4_CHNL = FOUR_K_1_CHNL * 4; // 64MB
const int EIGHT_K_4_CHNL = EIGHT_K_1_CHNL * 4; // 256MB

enum struct DX12BufferPoolFlag : uint32_t
{
	None = 0,
	AllowResize = 1,
	GCPositive = 2
};

class DX12BufferPool
{
	friend DX12Buffer;
	friend DX12BufferBlock;
public:
	static const int defaultBlockSize = 4194304; // 4MB

	DX12BufferPool() = default;
	void init(ComPtr<ID3D12Device> device, const D3D12_HEAP_PROPERTIES& heapProps, D3D12_HEAP_FLAGS heapFlags, UINT heapAlignment, Enum<DX12BufferPoolFlag> flags);

	UINT getAlignment() const;
	DX12Buffer* allocate(int size);
	DX12SubBuffer* suballocate(int size, int stride);
protected:
	ComPtr<ID3D12Device> device;
	D3D12_HEAP_PROPERTIES heapProps;
	D3D12_HEAP_FLAGS heapFlags;
	UINT heapAlignment;

	Enum<DX12BufferPoolFlag> flags = DX12BufferPoolFlag::None;
	int blockSize = defaultBlockSize;
	int size = 0;

	DX12BufferBlock* rootBlock = NULL;
	DX12BufferBlock* currentBlock = NULL;

	mutex mutexLock;
};