#include "DX12DescriptorHeap.h"
#include "DX12BufferPool.h"

bool DX12Descriptor::isValid() const
{
	return heap != NULL && index < heap->size();
}

bool DX12Descriptor::isActive() const
{
	if (heap == NULL)
		return false;
	return heap->isActive(index);
}

SIZE_T DX12Descriptor::getCPUHandle() const
{
	if (heap == NULL)
		return 0;
	else
		return heap->getCPUHandle(index).ptr;
}

SIZE_T DX12Descriptor::getGPUHandle() const
{
	if (heap == NULL)
		return 0;
	else
		return heap->getGPUHandle(index).ptr;
}

DXGI_FORMAT DX12Descriptor::getFormat() const
{
	if (heap == NULL)
		return NULL_FORMAT;
	else
		return heap->resourceViews[index].format;
}

DX12Buffer* DX12Descriptor::getBuffer() const
{
	if (heap == NULL)
		return NULL;
	else
		return heap->resourceViews[index].resource;
}

bool DX12Descriptor::copyFrom(const DX12Descriptor& other)
{
	if (!isValid() || !other.isValid())
		return false;
	heap->copyDescriptor(other, index);
	return true;
}

bool DX12Descriptor::updateCBV(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc)
{
	if (!isValid())
		return false;
	heap->updateCBV(desc, index);
	return true;
}

bool DX12Descriptor::updateSRV(DX12Buffer& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc)
{
	if (!isValid())
		return false;
	heap->updateSRV(resource, desc, index);
	return true;
}

bool DX12Descriptor::updateUAV(DX12Buffer& resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc)
{
	if (!isValid())
		return false;
	heap->updateUAV(resource, desc, index);
	return true;
}

bool DX12Descriptor::updateSampler(const D3D12_SAMPLER_DESC& desc)
{
	if (!isValid())
		return false;
	heap->updateSampler(desc, index);
	return true;
}

bool DX12Descriptor::updateRTV(DX12Buffer& resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc)
{
	if (!isValid())
		return false;
	heap->updateRTV(resource, desc, index);
	return true;
}

bool DX12Descriptor::updateRTV(DX12Buffer& resource)
{
	if (!isValid())
		return false;
	heap->updateRTV(resource, index);
	return true;
}

bool DX12Descriptor::updateDSV(DX12Buffer& resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc)
{
	if (!isValid())
		return false;
	heap->updateDSV(resource, desc, index);
	return true;
}

bool DX12Descriptor::update(const DX12ResourceView& view)
{
	if (!isValid())
		return false;
	heap->update(view, index);
	return true;
}

void DX12Descriptor::reset()
{
	if (isValid())
		heap->reset(index);
	heap = NULL;
	index = -1;
}

DX12Descriptor::operator D3D12_CPU_DESCRIPTOR_HANDLE() const
{
	return D3D12_CPU_DESCRIPTOR_HANDLE{ getCPUHandle() };
}

DX12Descriptor::operator D3D12_GPU_DESCRIPTOR_HANDLE() const
{
	return D3D12_GPU_DESCRIPTOR_HANDLE{ getGPUHandle() };
}

bool DX12Descriptor::operator==(const DX12Descriptor& d) const
{
	return heap == d.heap && index == d.index;
}

DX12ResourceView::DX12ResourceView(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc)
{
	init(desc);
}

DX12ResourceView::DX12ResourceView(DX12Buffer& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc)
{
	init(resource, desc);
}

DX12ResourceView::DX12ResourceView(DX12Buffer& resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc)
{
	init(resource, desc);
}

DX12ResourceView::DX12ResourceView(const D3D12_SAMPLER_DESC& desc)
{
	init(desc);
}

DX12ResourceView::DX12ResourceView(DX12Buffer& resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc)
{
	init(resource, desc);
}

DX12ResourceView::DX12ResourceView(DX12Buffer& resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc)
{
	init(resource, desc);
}

DX12ResourceView::DX12ResourceView(const DX12ResourceView& view)
{
	if (size != view.size && descData != NULL) {
		delete[] descData;
		descData = NULL;
	}
	size = view.size;
	if (descData == NULL) {
		descData = new char[size];
	}
	memcpy(descData, view.descData, size);
	type = view.type;
	resource = view.resource;
	format = view.format;
}

DX12ResourceView::DX12ResourceView(DX12ResourceView&& view)
{
	if (descData != NULL)
		delete[] descData;
	type = view.type;
	descData = view.descData;
	view.descData = NULL;
	size = view.size;
	view.size = 0;
	resource = view.resource;
	format = view.format;
}

DX12ResourceView::~DX12ResourceView()
{
	reset();
}

bool DX12ResourceView::isValid() const
{
	return (type == DX12ResourceViewType::CBV && format == CBV_FORMAT) ||
		(type == DX12ResourceViewType::SRV && format != NULL_FORMAT && resource != nullptr) ||
		(type == DX12ResourceViewType::UAV && format != NULL_FORMAT && resource != nullptr) ||
		(type == DX12ResourceViewType::Sampler && format != SAMPLER_FORMAT) ||
		(type == DX12ResourceViewType::RTV && format != NULL_FORMAT && resource != nullptr) ||
		(type == DX12ResourceViewType::DSV && format != NULL_FORMAT && resource != nullptr);
}

void DX12ResourceView::reset()
{
	if (descData != NULL)
		delete[] descData;
	type = DX12ResourceViewType::None;
	descData = NULL;
	size = 0;
	resource = nullptr;
	format = NULL_FORMAT;
}

DX12ResourceView& DX12ResourceView::operator=(const DX12ResourceView& view)
{
	if (size != view.size && descData != NULL) {
		delete[] descData;
		descData = NULL;
	}
	size = view.size;
	if (descData == NULL) {
		descData = new char[size];
	}
	memcpy(descData, view.descData, size);
	type = view.type;
	resource = view.resource;
	format = view.format;
	return *this;
}

DX12ResourceView& DX12ResourceView::operator=(DX12ResourceView&& view)
{
	if (descData != NULL)
		delete[] descData;
	type = view.type;
	descData = view.descData;
	view.descData = NULL;
	size = view.size;
	view.size = 0;
	resource = view.resource;
	format = view.format;
	return *this;
}

void DX12ResourceView::init(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc)
{
	type = DX12ResourceViewType::CBV;
	update(nullptr, desc, CBV_FORMAT);
}

void DX12ResourceView::init(DX12Buffer& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc)
{
	type = DX12ResourceViewType::SRV;
	update(&resource, desc, desc.Format);
}

void DX12ResourceView::init(DX12Buffer& resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc)
{
	type = DX12ResourceViewType::UAV;
	update(&resource, desc, desc.Format);
}

void DX12ResourceView::init(const D3D12_SAMPLER_DESC& desc)
{
	type = DX12ResourceViewType::Sampler;
	update(nullptr, desc, SAMPLER_FORMAT);
}

void DX12ResourceView::init(DX12Buffer& resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc)
{
	type = DX12ResourceViewType::RTV;
	update(&resource, desc, desc.Format);
}

void DX12ResourceView::init(DX12Buffer& resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc)
{
	type = DX12ResourceViewType::DSV;
	update(&resource, desc, desc.Format);
}

void DX12ResourceView::update(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc, ComPtr<ID3D12Device> device, const D3D12_CPU_DESCRIPTOR_HANDLE& DestDescriptor)
{
	type = DX12ResourceViewType::CBV;
	update(nullptr, desc, CBV_FORMAT);
	device->CreateConstantBufferView(&desc, DestDescriptor);
}

void DX12ResourceView::update(DX12Buffer& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, ComPtr<ID3D12Device> device, const D3D12_CPU_DESCRIPTOR_HANDLE& DestDescriptor)
{
	type = DX12ResourceViewType::SRV;
	update(&resource, desc, desc.Format);
	device->CreateShaderResourceView(resource.get().Get(), &desc, DestDescriptor);
}

void DX12ResourceView::update(DX12Buffer& resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, ComPtr<ID3D12Device> device, const D3D12_CPU_DESCRIPTOR_HANDLE& DestDescriptor)
{
	type = DX12ResourceViewType::UAV;
	update(&resource, desc, desc.Format);
	device->CreateUnorderedAccessView(resource.get().Get(), NULL, &desc, DestDescriptor);
}

void DX12ResourceView::update(const D3D12_SAMPLER_DESC& desc, ComPtr<ID3D12Device> device, const D3D12_CPU_DESCRIPTOR_HANDLE& DestDescriptor)
{
	type = DX12ResourceViewType::Sampler;
	update(nullptr, desc, SAMPLER_FORMAT);
	device->CreateSampler(&desc, DestDescriptor);
}

void DX12ResourceView::update(DX12Buffer& resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc, ComPtr<ID3D12Device> device, const D3D12_CPU_DESCRIPTOR_HANDLE& DestDescriptor)
{
	type = DX12ResourceViewType::RTV;
	update(&resource, desc, desc.Format);
	device->CreateRenderTargetView(resource.get().Get(), &desc, DestDescriptor);
}

void DX12ResourceView::update(DX12Buffer& resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc, ComPtr<ID3D12Device> device, const D3D12_CPU_DESCRIPTOR_HANDLE& DestDescriptor)
{
	type = DX12ResourceViewType::DSV;
	update(&resource, desc, desc.Format);
	device->CreateDepthStencilView(resource.get().Get(), &desc, DestDescriptor);
}

void DX12ResourceView::update(ComPtr<ID3D12Device> device, const D3D12_CPU_DESCRIPTOR_HANDLE& DestDescriptor)
{
	if (!isValid())
		return;
	switch (type)
	{
	case DX12ResourceViewType::CBV:
		device->CreateConstantBufferView((const D3D12_CONSTANT_BUFFER_VIEW_DESC*)descData, DestDescriptor);
		break;
	case DX12ResourceViewType::SRV: {
		const D3D12_SHADER_RESOURCE_VIEW_DESC* srv = (const D3D12_SHADER_RESOURCE_VIEW_DESC*)descData;
		device->CreateShaderResourceView(resource->get().Get(), srv, DestDescriptor);
	}
		break;
	case DX12ResourceViewType::UAV:
		device->CreateUnorderedAccessView(resource->get().Get(), NULL, (const D3D12_UNORDERED_ACCESS_VIEW_DESC*)descData, DestDescriptor);
		break;
	case DX12ResourceViewType::Sampler:
		device->CreateSampler((const D3D12_SAMPLER_DESC*)descData, DestDescriptor);
		break;
	case DX12ResourceViewType::RTV:
		device->CreateRenderTargetView(resource->get().Get(), (const D3D12_RENDER_TARGET_VIEW_DESC*)descData, DestDescriptor);
		break;
	case DX12ResourceViewType::DSV:
		device->CreateDepthStencilView(resource->get().Get(), (const D3D12_DEPTH_STENCIL_VIEW_DESC*)descData, DestDescriptor);
		break;
	default:
		break;
	}
}

DX12DescriptorHeap::DX12DescriptorHeap(int initCapacity, bool fixed, bool resizeCopy)
	: m_capacity(initCapacity), fixed(fixed), resizeCopy(resizeCopy)
{
}

DX12DescriptorHeap::DX12DescriptorHeap(ComPtr<ID3D12Device> device, const D3D12_DESCRIPTOR_HEAP_DESC& desc,
	int initCapacity, bool fixed, bool resizeCopy)
	: DX12DescriptorHeap(initCapacity, fixed, resizeCopy)
{
	init(device, desc);
}

void DX12DescriptorHeap::init(ComPtr<ID3D12Device> device, const D3D12_DESCRIPTOR_HEAP_DESC& desc)
{
	this->device = device;
	this->desc = desc;
	descriptorSize = device->GetDescriptorHandleIncrementSize(desc.Type);
	if (fixed)
		resize(m_capacity);
	else
		reserve(m_capacity);
}

bool DX12DescriptorHeap::resize(int size)
{
	if (fixed) {
		if (size > m_capacity) {
			throw runtime_error("resizing not allowed in fixed heap");
			return false;
		}
		reserve(size);
	}
	else if (size == 0)
		reserve(0);
	else if (size > m_capacity || size < m_capacity / 2)
		reserve(size * 1.5);
	this->m_size = size;
	return true;
}

void DX12DescriptorHeap::reserve(int capacity)
{
	if (capacity == 0) {
		release();
		return;
	}
	desc.NumDescriptors = capacity;
	if (resizeCopy && heap != NULL) {
		ComPtr<ID3D12DescriptorHeap> newHeap;
		device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&newHeap));
		device->CopyDescriptorsSimple(this->m_capacity, newHeap->GetCPUDescriptorHandleForHeapStart(),
			heap->GetCPUDescriptorHandleForHeapStart(), desc.Type);
		heap = newHeap;
	}
	else {
		device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap));
	}

	this->m_capacity = capacity;
	resourceViews.resize(capacity);
}

void DX12DescriptorHeap::release()
{
	heap.Reset();
	resourceViews.clear();
}

int DX12DescriptorHeap::size() const
{
	return m_size;
}

bool DX12DescriptorHeap::isInited() const
{
	return heap != NULL;
}

bool DX12DescriptorHeap::isFixed() const
{
	return fixed;
}

bool DX12DescriptorHeap::willResizeCopy() const
{
	return resizeCopy;
}

bool DX12DescriptorHeap::isActive(int index) const
{
	if (index >= m_size)
		return false;
	return resourceViews[index].isValid();
}

void DX12DescriptorHeap::reset(int index)
{
	if (index >= m_size)
		return;
	resourceViews[index].reset();
}

ComPtr<ID3D12DescriptorHeap> DX12DescriptorHeap::get()
{
	return heap;
}

DX12Descriptor DX12DescriptorHeap::update(const DX12ResourceView& view, int index)
{
	if (!view.isValid())
		throw runtime_error("view not valid");
	D3D12_CPU_DESCRIPTOR_HANDLE handle = allocateCPUHandle(index, true);
	DX12Descriptor item;
	item.heap = this;
	item.index = index;
	if (handle.ptr == 0)
		return item;
	DX12ResourceView& _view = resourceViews[index];
	_view = view;
	_view.update(device, handle);
	return item;
}

DX12Descriptor DX12DescriptorHeap::updateCBV(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc, int index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = allocateCPUHandle(index, true);
	DX12Descriptor item;
	item.heap = this;
	item.index = index;
	if (handle.ptr == 0)
		return item;
	resourceViews[index].update(desc, device, handle);
	return item;
}

DX12Descriptor DX12DescriptorHeap::updateSRV(DX12Buffer& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, int index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = allocateCPUHandle(index, true);
	DX12Descriptor item;
	item.heap = this;
	item.index = index;
	if (handle.ptr == 0)
		return item;
	resourceViews[index].update(resource, desc, device, handle);
	return item;
}

DX12Descriptor DX12DescriptorHeap::updateUAV(DX12Buffer& resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, int index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = allocateCPUHandle(index, true);
	DX12Descriptor item;
	item.heap = this;
	item.index = index;
	if (handle.ptr == 0)
		return item;
	resourceViews[index].update(resource, desc, device, handle);
	return item;
}

DX12Descriptor DX12DescriptorHeap::updateSampler(const D3D12_SAMPLER_DESC& desc, int index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = allocateCPUHandle(index, true);
	DX12Descriptor item;
	item.heap = this;
	item.index = index;
	if (handle.ptr == 0)
		return item;
	resourceViews[index].update(desc, device, handle);
	return item;
}

DX12Descriptor DX12DescriptorHeap::updateRTV(DX12Buffer& resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc, int index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = allocateCPUHandle(index, true);
	DX12Descriptor item;
	item.heap = this;
	item.index = index;
	if (handle.ptr == 0)
		return item;
	resourceViews[index].update(resource, desc, device, handle);
	return item;
}

DX12Descriptor DX12DescriptorHeap::updateRTV(DX12Buffer& resource, int index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = allocateCPUHandle(index, true);
	DX12Descriptor item;
	item.heap = this;
	item.index = index;
	if (handle.ptr == 0)
		return item;
	D3D12_RESOURCE_DESC resDesc = resource.get()->GetDesc();
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = resDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	resourceViews[index].update(resource, rtvDesc, device, handle);
	return item;
}

DX12Descriptor DX12DescriptorHeap::updateDSV(DX12Buffer& resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc, int index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = allocateCPUHandle(index, true);
	DX12Descriptor item;
	item.heap = this;
	item.index = index;
	if (handle.ptr == 0)
		return item;
	resourceViews[index].update(resource, desc, device, handle);
	return item;
}

DX12Descriptor DX12DescriptorHeap::copyDescriptor(const DX12Descriptor& descriptor, int index)
{
	if (!descriptor.isActive())
		return DX12Descriptor();
	if (desc.Type != descriptor.heap->desc.Type)
		return DX12Descriptor();
	D3D12_CPU_DESCRIPTOR_HANDLE handle = allocateCPUHandle(index, true);
	DX12Descriptor item;
	item.heap = this;
	item.index = index;
	if (handle.ptr == 0)
		return item;
	resourceViews[index] = descriptor.heap->resourceViews[descriptor.index];
	resourceViews[index].update(device, handle);
	return item;
}

DX12Descriptor DX12DescriptorHeap::operator[](int i)
{
	DX12Descriptor item = {};
	if (i >= m_size) {
		throw overflow_error("access DescriptorHeap overflow");
		return item;
	}
	item.heap = this;
	item.index = i;
	return item;
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12DescriptorHeap::getCPUHandle(int i) const
{
	if (i < m_size) {
		D3D12_CPU_DESCRIPTOR_HANDLE handle = heap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += i * descriptorSize;
		return handle;
	}
	return D3D12_CPU_DESCRIPTOR_HANDLE{ 0 };
}

D3D12_GPU_DESCRIPTOR_HANDLE DX12DescriptorHeap::getGPUHandle(int i) const
{
	if (i < m_size) {
		D3D12_GPU_DESCRIPTOR_HANDLE handle = heap->GetGPUDescriptorHandleForHeapStart();
		handle.ptr += i * descriptorSize;
		return handle;
	}
	return D3D12_GPU_DESCRIPTOR_HANDLE{ 0 };
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12DescriptorHeap::allocateCPUHandle(int& i, bool allowResize)
{
	if (i == -1) {
		i = m_size;
	}
	if (i >= m_size) {
		if (!allowResize)
			return D3D12_CPU_DESCRIPTOR_HANDLE{ 0 };
		resize(i + 1);
	}
	D3D12_CPU_DESCRIPTOR_HANDLE handle = heap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += i * descriptorSize;
	return handle;
}

DX12DescriptorRingHeap::DX12DescriptorRingHeap(int initCapacity)
	: DX12DescriptorHeap(initCapacity, true, true)
{
}

DX12DescriptorRingHeap::DX12DescriptorRingHeap(ComPtr<ID3D12Device> device, const D3D12_DESCRIPTOR_HEAP_DESC& desc, int initCapacity)
	: DX12DescriptorHeap(device, desc, initCapacity, true, true)
{
}

void DX12DescriptorRingHeap::init(ComPtr<ID3D12Device> device, const D3D12_DESCRIPTOR_HEAP_DESC& desc)
{
	DX12DescriptorHeap::init(device, desc);
	m_index = 0;
}

int DX12DescriptorRingHeap::size() const
{
	return m_size;
}

int DX12DescriptorRingHeap::getIndex() const
{
	return m_index;
}

void DX12DescriptorRingHeap::resize(int size)
{
	DX12DescriptorHeap::resize(size);
}

void DX12DescriptorRingHeap::release()
{
	DX12DescriptorHeap::release();
}

ComPtr<ID3D12DescriptorHeap> DX12DescriptorRingHeap::get()
{
	return DX12DescriptorHeap::get();
}

DX12Descriptor DX12DescriptorRingHeap::newCBV(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc)
{
	if (lookupNextNullSlot())
		return DX12DescriptorRingHeap::updateCBV(desc, m_index);
	else {
		throw overflow_error("no empty slot for new cbv");
		return DX12Descriptor();
	}
}

DX12Descriptor DX12DescriptorRingHeap::newSRV(DX12Buffer& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc)
{
	if (lookupNextNullSlot())
		return DX12DescriptorRingHeap::updateSRV(resource, desc, m_index);
	else {
		throw overflow_error("no empty slot for new srv");
		return DX12Descriptor();
	}
}

DX12Descriptor DX12DescriptorRingHeap::newUAV(DX12Buffer& resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc)
{
	if (lookupNextNullSlot())
		return DX12DescriptorRingHeap::updateUAV(resource, desc, m_index);
	else {
		throw overflow_error("no empty slot for new uav");
		return DX12Descriptor();
	}
}

DX12Descriptor DX12DescriptorRingHeap::newSampler(const D3D12_SAMPLER_DESC& desc)
{
	if (lookupNextNullSlot())
		return DX12DescriptorRingHeap::updateSampler(desc, m_index);
	else {
		throw overflow_error("no empty slot for new sampler");
		return DX12Descriptor();
	}
}

DX12Descriptor DX12DescriptorRingHeap::newRTV(DX12Buffer& resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc)
{
	if (lookupNextNullSlot())
		return DX12DescriptorRingHeap::updateRTV(resource, desc, m_index);
	else {
		throw overflow_error("no empty slot for new rtv");
		return DX12Descriptor();
	}
}

DX12Descriptor DX12DescriptorRingHeap::newRTV(DX12Buffer& resource)
{
	if (lookupNextNullSlot())
		return DX12DescriptorRingHeap::updateRTV(resource, m_index);
	else {
		throw overflow_error("no empty slot for new rtv");
		return DX12Descriptor();
	}
}

DX12Descriptor DX12DescriptorRingHeap::newDSV(DX12Buffer& resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc)
{
	if (lookupNextNullSlot())
		return DX12DescriptorRingHeap::updateDSV(resource, desc, m_index);
	else {
		throw overflow_error("no empty slot for new dsv");
		return DX12Descriptor();
	}
}

DX12Descriptor DX12DescriptorRingHeap::newDescriptor()
{
	if (lookupNextNullSlot())
		return (*this)[m_index];
	else {
		throw overflow_error("no empty slot for new dsv");
		return DX12Descriptor();
	}
}

DX12Descriptor DX12DescriptorRingHeap::newDescriptor(const DX12Descriptor& descriptor)
{
	if (lookupNextNullSlot())
		return DX12DescriptorRingHeap::copyDescriptor(descriptor, m_index);
	else {
		throw overflow_error("no empty slot for new dsv");
		return DX12Descriptor();
	}
}

DX12Descriptor DX12DescriptorRingHeap::newDescriptor(const DX12ResourceView& view)
{
	if (lookupNextNullSlot())
		return DX12DescriptorRingHeap::update(view, m_index);
	else {
		throw overflow_error("no empty slot for new dsv");
		return DX12Descriptor();
	}
}

bool DX12DescriptorRingHeap::lookupNextNullSlot()
{
	int end = m_index;
	do {
		if (!resourceViews[m_index].isValid()) {
			return true;
		}
		m_index++;
		m_index %= m_size;
	} while (m_index != end);
	return false;
}
