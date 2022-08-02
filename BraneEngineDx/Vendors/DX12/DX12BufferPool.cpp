#include "DX12BufferPool.h"

ComPtr<ID3D12Resource> DX12SubBuffer::get()
{
    return buffer->buffer;
}

void DX12SubBuffer::release()
{
    deallocate();
}

bool DX12SubBuffer::isUnused() const
{
    return subBufferPointer == NULL;
}

void DX12SubBuffer::upload(void* data, int size, int first)
{
    memcpy_s((char*)subBufferPointer + first, this->size - first, data, size);
}

void DX12SubBuffer::read(void* data, int size, int first)
{
    memcpy_s(data, size, (char*)subBufferPointer + first, this->size - first);
}

DX12ResourceView DX12SubBuffer::getCBV()
{
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = buffer->buffer->GetGPUVirtualAddress() + base;
    cbvDesc.SizeInBytes = size;
    return DX12ResourceView(cbvDesc);
}

DX12ResourceView DX12SubBuffer::getBufferSRV(DXGI_FORMAT format)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = getFirstElement();
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = format;
    srvDesc.Buffer.NumElements = getNumElements();
    return DX12ResourceView(*buffer, srvDesc);
}

DX12ResourceView DX12SubBuffer::getStructSRV()
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = getFirstElement();
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.Buffer.StructureByteStride = getStride();
    srvDesc.Buffer.NumElements = getNumElements();
    return DX12ResourceView(*buffer, srvDesc);
}

int DX12SubBuffer::getSize() const
{
    return size;
}

int DX12SubBuffer::getStride() const
{
    return buffer->stride;
}

int DX12SubBuffer::getOffset() const
{
    return base;
}

int DX12SubBuffer::getFirstElement() const
{
    return base / buffer->stride;
}

int DX12SubBuffer::getNumElements() const
{
    return size / buffer->stride;
}

bool DX12SubBuffer::tryAllocate(int size)
{
    return isUnused() && this->size >= size;
}

DX12SubBuffer* DX12SubBuffer::allocate(int size)
{
    if (!isUnused())
        throw domain_error("allocate on unused memory");
    else if (size > this->size)
        throw overflow_error("out of memory");
    else if (size == this->size) {

        if (buffer->mappedPointer == NULL)
            throw runtime_error("Unknown null pointer error");

        subBufferPointer = (char*)buffer->mappedPointer + base;

        OutputDebugString(("[Brane] suballocate (base: " + to_string(base) + ", size:" + to_string(size) + ")\n").c_str());
        return this;
    }
    else {
        DX12SubBuffer* subBuffer = new DX12SubBuffer;
        subBuffer->buffer = buffer;
        subBuffer->last = last;
        subBuffer->next = this;
        subBuffer->size = size;
        subBuffer->base = base;

        if (last != NULL)
            last->next = subBuffer;

        if (buffer->mappedPointer == NULL)
            throw runtime_error("Unknown null pointer error");

        subBuffer->subBufferPointer = (char*)buffer->mappedPointer + base;

        last = subBuffer;
        this->size -= size;
        base += size;

        if (buffer->rootSubBuffer == this) {
            buffer->rootSubBuffer = subBuffer;
        }

        OutputDebugString(("[Brane] suballocate (base: " + to_string(subBuffer->base) + ", size:" + to_string(subBuffer->size) + ")\n").c_str());

        return subBuffer;
    }
    return nullptr;
}

void DX12SubBuffer::deallocate()
{
    OutputDebugString(("[Brane] release suballocate (base: " + to_string(base) + ", size:" + to_string(size) + ")\n").c_str());
    if (isUnused())
        throw runtime_error("deallocate unused memory");
    else {
        bool lastUnused = last != NULL && last->isUnused();
        bool nextUnused = next != NULL && next->isUnused();
        DX12SubBuffer* lastBuf = last, * nextBuf = next;
        if (lastUnused && nextUnused) {
            base = last->base;
            size += last->size + next->size;
            if (buffer->rootSubBuffer == last) {
                buffer->rootSubBuffer = this;
            }
            last = last->last;
            next = next->next;
            if (last != NULL)
                last->next = this;
            if (next != NULL)
                next->last = this;
            delete lastBuf;
            delete nextBuf;
        }
        else if (lastUnused) {
            base = last->base;
            size += last->size;
            if (buffer->rootSubBuffer == last) {
                buffer->rootSubBuffer = this;
            }
            last = last->last;
            if (last != NULL)
                last->next = this;
            delete lastBuf;
        }
        else if (nextUnused) {
            size += next->size;
            next = next->next;
            if (next != NULL)
                next->last = this;
            delete nextBuf;
        }
        buffer->currentSubBuffer = NULL;
        buffer->tryDeallocate();
    }
}

ComPtr<ID3D12Resource> DX12Buffer::get()
{
    return buffer;
}

void DX12Buffer::release()
{
    unmap();
    deallocate();
}

bool DX12Buffer::isUnused() const
{
    return buffer == NULL;
}

int DX12Buffer::getSize() const
{
    return size;
}

int DX12Buffer::getStride() const
{
    return stride;
}

void DX12Buffer::transitionBarrier(ComPtr<ID3D12GraphicsCommandList> cmdLst, D3D12_RESOURCE_STATES newState, UINT subresouces)
{
    DX12ResourceBarrier& barrier = getBarrier(subresouces);
    if (barrier.state == newState)
        return;
    if (barrier.transitingState != (D3D12_RESOURCE_STATES)-1)
        throw runtime_error("using split barrier");
    cmdLst->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer.Get(),
        barrier.state, newState, subresouces));
    barrier.state = newState;
}

void DX12Buffer::beginTransitionBarrier(ComPtr<ID3D12GraphicsCommandList> cmdLst, D3D12_RESOURCE_STATES newState, UINT subresouces)
{
    DX12ResourceBarrier& barrier = getBarrier(subresouces);
    if (barrier.state == newState)
        return;
    if (barrier.transitingState != (D3D12_RESOURCE_STATES)-1)
        throw runtime_error("using split barrier");
    cmdLst->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer.Get(),
        barrier.state, newState, subresouces, D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY));
    barrier.transitingState = newState;
}

void DX12Buffer::endTransitionBarrier(ComPtr<ID3D12GraphicsCommandList> cmdLst, UINT subresouces)
{
    DX12ResourceBarrier& barrier = getBarrier(subresouces);
    if (barrier.transitingState == (D3D12_RESOURCE_STATES)-1)
        return;
    cmdLst->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer.Get(),
        barrier.state, barrier.transitingState, subresouces, D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY));
    barrier.state = barrier.transitingState;
    barrier.transitingState = (D3D12_RESOURCE_STATES)-1;
}

bool DX12Buffer::CreateResource(const D3D12_RESOURCE_DESC& desc, D3D12_RESOURCE_STATES initialState, unsigned int stride)
{
    ComPtr<ID3D12Device> device = block->pool->device;
    this->stride = stride;
    D3D12_CLEAR_VALUE ClearValue = {};
    if (desc.Format == DXGI_FORMAT_R32_TYPELESS) {
        ClearValue.Format = DXGI_FORMAT_R32_FLOAT;
        ClearValue.DepthStencil.Depth = 1;
        ClearValue.DepthStencil.Stencil = 0;
    }
    else {
        ClearValue.Format = desc.Format;
        ClearValue.Color[0] = 0;
        ClearValue.Color[1] = 0;
        ClearValue.Color[2] = 0;
        ClearValue.Color[3] = 0;
    }
    getBarrier(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) =
    { D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, initialState, (D3D12_RESOURCE_STATES)-1 };
    return SUCCEEDED(device->CreatePlacedResource(block->heap.Get(), base, &desc,
        initialState, NULL, IID_PPV_ARGS(&buffer)));
}

bool DX12Buffer::CreateResource(const D3D12_RESOURCE_DESC& desc, D3D12_RESOURCE_STATES initialState, Vector4f clearValue, unsigned int stride)
{
    ComPtr<ID3D12Device> device = block->pool->device;
    D3D12_CLEAR_VALUE ClearValue = {};
    ClearValue.Format = desc.Format;
    ClearValue.Color[0] = clearValue.x();
    ClearValue.Color[1] = clearValue.y();
    ClearValue.Color[2] = clearValue.z();
    ClearValue.Color[3] = clearValue.w();
    this->stride = stride;
    getBarrier(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) =
    { D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, initialState, (D3D12_RESOURCE_STATES)-1 };
    return SUCCEEDED(device->CreatePlacedResource(block->heap.Get(), base, &desc,
        initialState, &ClearValue, IID_PPV_ARGS(&buffer)));
}

void* DX12Buffer::map()
{
    if (mappedPointer == NULL) {
        CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
        buffer->Map(0, &readRange, reinterpret_cast<void**>(&mappedPointer));
    }
    return mappedPointer;
}

void DX12Buffer::unmap()
{
    if (mappedPointer != NULL) {
        buffer->Unmap(0, NULL);
        mappedPointer = NULL;
    }
}

DX12ResourceBarrier& DX12Buffer::getBarrier(UINT subresource)
{
    DX12ResourceBarrier* barrier = NULL;
    auto iter = barriers.find(subresource);
    if (iter == barriers.end()) {
        barrier = &barriers.insert(make_pair(subresource, DX12ResourceBarrier())).first->second;
        barrier->state = barriers[D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES].state;
    }
    else
        barrier = &iter->second;
    return *barrier;
}

bool DX12Buffer::tryAllocate(int size)
{
    return isUnused() && this->size >= size;
}

DX12Buffer* DX12Buffer::allocate(int size)
{
    if (!isUnused())
        throw domain_error("allocate on unused memory");
    else if (size > this->size)
        throw overflow_error("out of memory");
    else if (size == this->size) {
        OutputDebugString(("[Brane] allocate (base: " + to_string(base) + ", size:" + to_string(size) + ")\n").c_str());
        return this;
    }
    else {
        DX12Buffer* buffer = new DX12Buffer;
        buffer->block = block;
        buffer->last = last;
        buffer->next = this;
        buffer->size = size;
        buffer->base = base;

        if (last != NULL)
            last->next = buffer;

        last = buffer;
        this->size -= size;
        base += size;

        if (block->rootNode == this) {
            block->rootNode = buffer;
        }

        OutputDebugString(("[Brane] allocate (base: " + to_string(buffer->base) + ", size:" + to_string(buffer->size) + ")\n").c_str());

        return buffer;
    }
    return nullptr;
}

bool DX12Buffer::trySuballocate(int size, int stride)
{
    if (isUnused())
        return true;
    if (this->stride != stride)
        return false;
    if (currentSubBuffer == NULL)
        currentSubBuffer = rootSubBuffer;
    while (currentSubBuffer != NULL) {
        if (currentSubBuffer->tryAllocate(size))
            return true;
        currentSubBuffer = currentSubBuffer->next;
    }
    return false;
}

DX12SubBuffer* DX12Buffer::suballocate(int size, int stride)
{
    if (!trySuballocate(size, stride))
        return NULL;
    if (buffer == NULL) {
        CreateResource(CD3DX12_RESOURCE_DESC::Buffer(this->size), D3D12_RESOURCE_STATE_GENERIC_READ, stride);
        map();
    }
    if (mappedPointer == NULL)
        throw runtime_error("map failed");
    if (rootSubBuffer == NULL) {
        rootSubBuffer = new DX12SubBuffer;
        rootSubBuffer->buffer = this;
        rootSubBuffer->base = 0;
        rootSubBuffer->size = this->size;
        currentSubBuffer = rootSubBuffer;
    }
    return currentSubBuffer->allocate(size);
}

void DX12Buffer::tryDeallocate()
{
    if (rootSubBuffer != NULL && rootSubBuffer->isUnused() && rootSubBuffer->size == size) {
        delete rootSubBuffer;
        unmap();
        deallocate();
    }
}

void DX12Buffer::deallocate()
{
    OutputDebugString(("[Brane] release allocate (base: " + to_string(base) + ", size:" + to_string(size) + ")\n").c_str());
    lock_guard lock = lock_guard(block->pool->mutexLock);
    if (isUnused())
        throw runtime_error("deallocate unused memory");
    else {
        buffer.Reset();
        bool lastUnused = last != NULL && last->isUnused();
        bool nextUnused = next != NULL && next->isUnused();
        DX12Buffer *lastBuf = last, *nextBuf = next;
        if (lastUnused && nextUnused) {
            base = last->base;
            size += last->size + next->size;
            if (block->rootNode == last) {
                block->rootNode = this;
            }
            last = last->last;
            next = next->next;
            if (last != NULL)
                last->next = this;
            if (next != NULL)
                next->last = this;
            delete lastBuf;
            delete nextBuf;
        }
        else if (lastUnused) {
            base = last->base;
            size += last->size;
            if (block->rootNode == last) {
                block->rootNode = this;
            }
            last = last->last;
            if (last != NULL)
                last->next = this;
            delete lastBuf;
        }
        else if (nextUnused) {
            size += next->size;
            next = next->next;
            if (next != NULL)
                next->last = this;
            delete nextBuf;
        }
        block->currentNode = NULL;
        block->tryDeallocate();
    }
}

DX12BackBuffer::DX12BackBuffer(ComPtr<ID3D12Resource> backBuffer)
{
    init(backBuffer);
}

void DX12BackBuffer::init(ComPtr<ID3D12Resource> backBuffer)
{
    buffer = backBuffer;
    barriers.clear();
    getBarrier(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES).state = D3D12_RESOURCE_STATE_COMMON;
}

void DX12BackBuffer::release()
{
    buffer.Reset();
}

void DX12BufferBlock::init(int size)
{
    this->size = size;

    rootNode = new DX12Buffer;
    rootNode->block = this;
    rootNode->base = 0;
    rootNode->size = size;
    currentNode = rootNode;
}

bool DX12BufferBlock::tryAllocate(int size)
{
    if (currentNode == NULL)
        currentNode = rootNode;
    while (currentNode != NULL) {
        if (currentNode->tryAllocate(size))
            return true;
        currentNode = currentNode->next;
    }
    return false;
}

DX12Buffer* DX12BufferBlock::allocate(int size, ComPtr<ID3D12Device> device, const D3D12_HEAP_PROPERTIES& props, D3D12_HEAP_FLAGS flags, UINT alignment)
{
    if (!tryAllocate(size))
        return NULL;
    if (heap == NULL) {
        D3D12_HEAP_DESC desc = {};
        desc.Properties = props;
        desc.Flags = flags;
        desc.SizeInBytes = this->size;
        desc.Alignment = alignment;
        device->CreateHeap(&desc, IID_PPV_ARGS(&heap));
    }
    return currentNode->allocate(size);
}

bool DX12BufferBlock::trySuballocate(int size, int stride)
{
    if (currentNode == NULL)
        currentNode = rootNode;
    while (currentNode != NULL) {
        if (currentNode->trySuballocate(size, stride))
            return true;
        currentNode = currentNode->next;
    }
    return false;
}

DX12SubBuffer* DX12BufferBlock::suballocate(int size, int stride, ComPtr<ID3D12Device> device, const D3D12_HEAP_PROPERTIES& props, D3D12_HEAP_FLAGS flags, UINT alignment)
{
    if (!trySuballocate(size, stride))
        return NULL;
    if (heap == NULL) {
        D3D12_HEAP_DESC desc = {};
        desc.Properties = props;
        desc.Flags = flags;
        desc.SizeInBytes = this->size;
        desc.Alignment = alignment;
        device->CreateHeap(&desc, IID_PPV_ARGS(&heap));
    }
    return currentNode->suballocate(size, stride);
}

void DX12BufferBlock::tryDeallocate()
{
    if (rootNode->isUnused() && rootNode->size == size) {
        delete rootNode;
        heap.Reset();
        if (pool->currentBlock == this) {
            if (pool->rootBlock == this) {
                pool->rootBlock = NULL;
                pool->currentBlock = NULL;
            }
            else {
                pool->currentBlock = next;
            }
        }
        DX12BufferBlock* curBlock = next;
        while (curBlock != NULL) {
            if (curBlock->next == this) {
                curBlock->next = next;
                break;
            }
            curBlock = curBlock->next;
        }
        delete this;
    }
}

void DX12BufferPool::init(ComPtr<ID3D12Device> device, const D3D12_HEAP_PROPERTIES& heapProps, D3D12_HEAP_FLAGS heapFlags, UINT heapAlignment, Enum<DX12BufferPoolFlag> flags)
{
    lock_guard lock = lock_guard(mutexLock);
    this->device = device;
    this->heapProps = heapProps;
    this->heapFlags = heapFlags;
    this->heapAlignment = heapAlignment;
    this->flags = flags;
}

UINT DX12BufferPool::getAlignment() const
{
    return heapAlignment;
}

DX12Buffer* DX12BufferPool::allocate(int size)
{
    lock_guard lock = lock_guard(mutexLock);

    if (rootBlock == NULL) {
        int newBlockSize = blockSize;

        newBlockSize = GRS_UPPER(size, heapAlignment);

        rootBlock = new DX12BufferBlock;
        rootBlock->init(newBlockSize);
        rootBlock->pool = this;
        rootBlock->next = rootBlock;
        currentBlock = rootBlock;
        this->size += blockSize;
    }

    DX12Buffer* buffer = NULL;
    DX12BufferBlock* startBlock = currentBlock;
    do {
        buffer = currentBlock->allocate(size, device, heapProps, heapFlags, heapAlignment);
        if (buffer != NULL)
            return buffer;
        currentBlock = currentBlock->next;
    } while (startBlock != currentBlock);

    if (currentBlock->currentNode != NULL) {
        buffer = currentBlock->allocate(size, device, heapProps, heapFlags, heapAlignment);
        if (buffer != NULL)
            return buffer;
    }

    if (!flags.has(DX12BufferPoolFlag::AllowResize))
        return buffer;

    currentBlock = new DX12BufferBlock;

    int newBlockSize = blockSize;

    newBlockSize = GRS_UPPER(size, heapAlignment);

    currentBlock->init(newBlockSize);
    currentBlock->pool = this;
    this->size += newBlockSize;
    currentBlock->next = startBlock->next;
    startBlock->next = currentBlock;

    return currentBlock->allocate(size, device, heapProps, heapFlags, heapAlignment);
}

DX12SubBuffer* DX12BufferPool::suballocate(int size, int stride)
{
    if (stride < 4)
        throw runtime_error("stride not support");
    if (size % stride != 0)
        throw runtime_error("size not aligned by stride");
    lock_guard lock = lock_guard(mutexLock);

    if (rootBlock == NULL) {
        int newBlockSize = blockSize;

        newBlockSize = GRS_UPPER(size, heapAlignment);

        rootBlock = new DX12BufferBlock;
        rootBlock->init(newBlockSize);
        rootBlock->pool = this;
        rootBlock->next = rootBlock;
        currentBlock = rootBlock;
        this->size += blockSize;
    }

    DX12SubBuffer* subBuffer = NULL;
    DX12BufferBlock* startBlock = currentBlock;
    do {
        subBuffer = currentBlock->suballocate(size, stride, device, heapProps, heapFlags, heapAlignment);
        if (subBuffer != NULL)
            return subBuffer;
        currentBlock = currentBlock->next;
    } while (startBlock != currentBlock);

    if (currentBlock->currentNode != NULL) {
        subBuffer = currentBlock->suballocate(size, stride, device, heapProps, heapFlags, heapAlignment);
        if (subBuffer != NULL)
            return subBuffer;
    }

    if (!flags.has(DX12BufferPoolFlag::AllowResize))
        return subBuffer;

    currentBlock = new DX12BufferBlock;

    int newBlockSize = blockSize;

    newBlockSize = GRS_UPPER(size, heapAlignment);

    currentBlock->init(newBlockSize);
    currentBlock->pool = this;
    this->size += newBlockSize;
    currentBlock->next = startBlock->next;
    startBlock->next = currentBlock;

    return currentBlock->suballocate(size, stride, device, heapProps, heapFlags, heapAlignment);
}
