#pragma once

#include "DX12.h"

class DX12DescriptorHeap;
struct DX12ResourceView;
struct DX12Buffer;

struct DX12Descriptor
{
	friend DX12DescriptorHeap;
public:
	DX12Descriptor() = default;
	bool isValid() const;
	bool isActive() const;
	SIZE_T getCPUHandle() const;
	SIZE_T getGPUHandle() const;
	DXGI_FORMAT getFormat() const;
	DX12Buffer* getBuffer() const;
	bool copyFrom(const DX12Descriptor& other);
	bool updateCBV(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc);
	bool updateSRV(DX12Buffer& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
	bool updateUAV(DX12Buffer& resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc);
	bool updateSampler(const D3D12_SAMPLER_DESC& desc);
	bool updateRTV(DX12Buffer& resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc);
	bool updateRTV(DX12Buffer& resource);
	bool updateDSV(DX12Buffer& resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc);
	bool update(const DX12ResourceView& view);
	void reset();
	operator D3D12_CPU_DESCRIPTOR_HANDLE() const;
	operator D3D12_GPU_DESCRIPTOR_HANDLE() const;
	bool operator==(const DX12Descriptor & d) const;
protected:
	DX12DescriptorHeap* heap = NULL;
	int index = -1;
};

enum struct DX12ResourceViewType : unsigned char
{
	None, CBV, SRV, UAV, Sampler, RTV, DSV
};

struct DX12ResourceView
{
	DX12Buffer* resource;
	DXGI_FORMAT format = NULL_FORMAT;
	void* descData = NULL;
	unsigned int size = 0;
	DX12ResourceViewType type = DX12ResourceViewType::None;

	DX12ResourceView() = default;
	DX12ResourceView(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc);
	DX12ResourceView(DX12Buffer& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
	DX12ResourceView(DX12Buffer& resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc);
	DX12ResourceView(const D3D12_SAMPLER_DESC& desc);
	DX12ResourceView(DX12Buffer& resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc);
	DX12ResourceView(DX12Buffer& resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc);

	DX12ResourceView(const DX12ResourceView& view);
	DX12ResourceView(DX12ResourceView&& view);
	~DX12ResourceView();

	bool isValid() const;
	void reset();

	DX12ResourceView& operator=(const DX12ResourceView& view);
	DX12ResourceView& operator=(DX12ResourceView&& view);

	void init(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc);
	void init(DX12Buffer& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
	void init(DX12Buffer& resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc);
	void init(const D3D12_SAMPLER_DESC& desc);
	void init(DX12Buffer& resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc);
	void init(DX12Buffer& resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc);

	void update(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc, ComPtr<ID3D12Device> device, const D3D12_CPU_DESCRIPTOR_HANDLE& DestDescriptor);
	void update(DX12Buffer& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, ComPtr<ID3D12Device> device, const D3D12_CPU_DESCRIPTOR_HANDLE& DestDescriptor);
	void update(DX12Buffer& resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, ComPtr<ID3D12Device> device, const D3D12_CPU_DESCRIPTOR_HANDLE& DestDescriptor);
	void update(const D3D12_SAMPLER_DESC& desc, ComPtr<ID3D12Device> device, const D3D12_CPU_DESCRIPTOR_HANDLE& DestDescriptor);
	void update(DX12Buffer& resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc, ComPtr<ID3D12Device> device, const D3D12_CPU_DESCRIPTOR_HANDLE& DestDescriptor);
	void update(DX12Buffer& resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc, ComPtr<ID3D12Device> device, const D3D12_CPU_DESCRIPTOR_HANDLE& DestDescriptor);

	void update(ComPtr<ID3D12Device> device, const D3D12_CPU_DESCRIPTOR_HANDLE& DestDescriptor);
protected:
	template<class DESC>
	void update(DX12Buffer* resource, const DESC& desc, DXGI_FORMAT format);
};

template<class DESC>
void DX12ResourceView::update(DX12Buffer* resource, const DESC& desc, DXGI_FORMAT format)
{
	if (size != sizeof(DESC) && descData != NULL) {
		delete[] descData;
		descData = NULL;
	}
	size = sizeof(DESC);
	if (descData == NULL) {
		descData = new char[size];
	}
	memcpy(descData, &desc, size);
	this->resource = resource;
	this->format = format;
}

class DX12DescriptorHeap
{
	friend DX12Descriptor;
public:
	DX12DescriptorHeap(int initCapacity = 0, bool fixed = true, bool resizeCopy = false);
	DX12DescriptorHeap(ComPtr<ID3D12Device> device, const D3D12_DESCRIPTOR_HEAP_DESC& desc, int initCapacity = 0, bool fixed = true, bool resizeCopy = false);

	void init(ComPtr<ID3D12Device> device, const D3D12_DESCRIPTOR_HEAP_DESC& desc);

	bool resize(int size);
	void reserve(int capacity);

	void release();

	int size() const;
	bool isInited() const;
	bool isFixed() const;
	bool willResizeCopy() const;

	bool isActive(int index) const;
	void reset(int index);

	ComPtr<ID3D12DescriptorHeap> get();

	DX12Descriptor update(const DX12ResourceView& view, int index = -1);

	DX12Descriptor updateCBV(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc, int index = -1);
	DX12Descriptor updateSRV(DX12Buffer& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, int index = -1);
	DX12Descriptor updateUAV(DX12Buffer& resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, int index = -1);

	DX12Descriptor updateSampler(const D3D12_SAMPLER_DESC& desc, int index = -1);

	DX12Descriptor updateRTV(DX12Buffer& resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc, int index = -1);
	DX12Descriptor updateRTV(DX12Buffer& resource, int index = -1);

	DX12Descriptor updateDSV(DX12Buffer& resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc, int index = -1);

	DX12Descriptor copyDescriptor(const DX12Descriptor& descriptor, int index = -1);

	DX12Descriptor operator[](int i);
protected:
	ComPtr<ID3D12Device> device = NULL;
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	SIZE_T descriptorSize = 0;
	ComPtr<ID3D12DescriptorHeap> heap = NULL;
	vector<DX12ResourceView> resourceViews;
	int m_size = 0;
	int m_capacity = 0;
	bool fixed = true;
	bool resizeCopy = false;

	D3D12_CPU_DESCRIPTOR_HANDLE getCPUHandle(int i) const;
	D3D12_GPU_DESCRIPTOR_HANDLE getGPUHandle(int i) const;
	D3D12_CPU_DESCRIPTOR_HANDLE allocateCPUHandle(int& i, bool allowResize);
};

class DX12DescriptorRingHeap : protected DX12DescriptorHeap
{
public:
	DX12DescriptorRingHeap(int initCapacity);
	DX12DescriptorRingHeap(ComPtr<ID3D12Device> device, const D3D12_DESCRIPTOR_HEAP_DESC& desc, int initCapacity);

	void init(ComPtr<ID3D12Device> device, const D3D12_DESCRIPTOR_HEAP_DESC& desc);

	int size() const;
	int getIndex() const;

	void resize(int size);
	void release();

	ComPtr<ID3D12DescriptorHeap> get();

	DX12Descriptor newCBV(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc);
	DX12Descriptor newSRV(DX12Buffer& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
	DX12Descriptor newUAV(DX12Buffer& resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc);

	DX12Descriptor newSampler(const D3D12_SAMPLER_DESC& desc);

	DX12Descriptor newRTV(DX12Buffer& resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc);
	DX12Descriptor newRTV(DX12Buffer& resource);

	DX12Descriptor newDSV(DX12Buffer& resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc);

	DX12Descriptor newDescriptor();
	DX12Descriptor newDescriptor(const DX12Descriptor& descriptor);
	DX12Descriptor newDescriptor(const DX12ResourceView& view);
protected:
	int m_index = -1;

	bool lookupNextNullSlot();
};