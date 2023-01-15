#include "DX12GPUBuffer.h"
#include "../DXGI_Helper.h"

DX12GPUBuffer::DX12GPUBuffer(DX12Context& context, GPUBufferDesc& desc)
    : dxContext(context), IGPUBuffer(desc)
{

}

DX12GPUBuffer::~DX12GPUBuffer()
{
    release();
}

unsigned int DX12GPUBuffer::bind()
{
    return resize(desc.size);
}

unsigned int DX12GPUBuffer::resize(unsigned int size)
{
	if (size == 0) {
		release();
	}
	else if (size > desc.capacity || size < desc.capacity / 2) {
		release();
		desc.capacity = size * 1.5;
		addTask(TaskType::Resize, dxContext.activeBackBufferIndex, dxContext.backBufferCount, desc.capacity);
		desc.id = (unsigned int)this;
	}
	desc.size = size;
	return desc.id;
}

void DX12GPUBuffer::release()
{
	addTask(TaskType::Release, dxContext.activeBackBufferIndex, dxContext.backBufferCount);
	desc.capacity = 0;
	desc.size = 0;
	desc.id = 0;
}

const char* getGPUBufferTypeName(GPUBufferType type)
{
	switch (type)
	{
	case GB_Constant:
		return ("GB_Constant");
		break;
	case GB_Vertex:
		return ("GB_Vertex");
		break;
	case GB_Index:
		return ("GB_Index");
		break;
	case GB_Storage:
		return ("GB_Storage");
		break;
	}
	return "";
}

unsigned int DX12GPUBuffer::bindBase(unsigned int index, BufferOption bufferOption)
{
	if (desc.type == GB_Storage && desc.format == GBF_Struct && index == 0) {
		OutputDebugStringA(("[Brane]--------Frame(" + to_string(dxContext.activeBackBufferIndex) + ")--------\n").c_str());
		OutputDebugStringA((string("[Brane] ") + getGPUBufferTypeName(desc.type) + "(" + to_string(index) + ")\n").c_str());
		list<Task>& task = tasks[dxContext.activeBackBufferIndex];
		auto iter = task.begin();
		auto end = task.end();
		while (iter != end) {
			switch (iter->type)
			{
			case TaskType::Release:
				OutputDebugStringA("[Brane] GPUBuffer::Release\n");
				break;
			case TaskType::Resize:
				OutputDebugStringA(("[Brane] GPUBuffer::Resize(" + to_string(iter->size) + ")\n").c_str());
				break;
			case TaskType::Upload:
				OutputDebugStringA(("[Brane] GPUBuffer::Upload(" + to_string(iter->first) + ", " + to_string(iter->size) + ")\n").c_str());
				break;
			}
			iter++;
		}
		OutputDebugStringA("[Brane]------------------------\n\n");
	}
	processTasks(dxContext.activeBackBufferIndex);
	DX12SubBuffer*& dx12Buffer = dx12Buffers[dxContext.activeBackBufferIndex];
	switch (desc.type)
	{
	case GB_Constant:
		if (dx12Buffer == NULL)
			return 0;
		dxContext.graphicContext.bindDefaultCBV(index, dx12Buffer->getCBV());
		break;
	case GB_Vertex:
	{
		if (dx12Buffer == NULL)
			return 0;
		dxContext.graphicContext.bindVertexBuffer(index, dx12Buffer);
		break;
	}
	case GB_Index:
	{
		if (dx12Buffer == NULL)
			return 0;
		dxContext.graphicContext.bindIndexBuffer(dx12Buffer);
		break;
	}
	case GB_Storage:
	{
		if (dx12Buffer == NULL)
			return 0;
		if (bufferOption.output)
			throw runtime_error("Not implemented");
		DXGI_FORMAT format = getDXGIFormat(desc.format);
		dxContext.graphicContext.bindDefaultSRV(index, dx12Buffer->getBufferSRV(format));
		break;
	}
	default:
		throw runtime_error("Unknown Error");
		break;
	}
	return desc.id;
}

unsigned int DX12GPUBuffer::uploadData(unsigned int size, void* data, bool discard)
{
	resize(size);
	if (size == 0)   
		return desc.id;
	UploadSource* source = new UploadSource();
	unsigned int alignedSize = size * desc.cellSize;
	source->data = new char[alignedSize];
	memcpy(source->data, data, alignedSize);
	addTask(TaskType::Upload, dxContext.activeBackBufferIndex, dxContext.backBufferCount, size, source);
	return desc.id;
}

unsigned int DX12GPUBuffer::uploadSubData(unsigned int first, unsigned int size, void* data)
{
	if (size == 0)
		return desc.id;
	UploadSource* source = new UploadSource();
	unsigned int alignedSize = size * desc.cellSize;
	source->data = new char[alignedSize];
	memcpy(source->data, data, alignedSize);
	addTask(TaskType::Upload, dxContext.activeBackBufferIndex, dxContext.backBufferCount, size, source, first);
	return desc.id;
}

bool DX12GPUBuffer::readData(void* data)
{
	if (desc.size == 0 || data == NULL)
		return false;
	DX12SubBuffer*& dx12Buffer = dx12Buffers[0];
	if (dx12Buffer == NULL)
		return false;
	dx12Buffer->read(data, desc.size, 0);
	return true;
}

bool DX12GPUBuffer::readSubData(unsigned int first, unsigned int size, void* data)
{
	if (desc.size == 0 || data == NULL)
		return false;
	DX12SubBuffer*& dx12Buffer = dx12Buffers[0];
	if (dx12Buffer == NULL)
		return false;
	dx12Buffer->read(data, size, first);
	return true;
}

void DX12GPUBuffer::addTask(TaskType type, int backBufferIndex, int count, int size, UploadSource* source, int first)
{
	for (int i = 0; i < count; i++) {
		int index = (dxContext.activeBackBufferIndex + i) % dxContext.backBufferCount;
		list<Task>& task = tasks[index];

		switch (type)
		{
		case TaskType::Release:
			processTasks(index, true);
			break;
		case TaskType::Resize:
		{
			bool releaseFound = false;
			auto b = task.rbegin();
			while (b != task.rend()) {
				if (b->type == TaskType::Release)
					releaseFound = true;
				doTask(*b, true);
				b++;
			}
			task.clear();
			if (releaseFound)
				task.emplace_back(Task{ TaskType::Release, index, NULL, 0, 0, 0 });
		}
			break;
		case TaskType::Upload:
		{
			auto b = task.begin();
			while (b != task.end()) {
				if (b->type == TaskType::Upload) {
					int end = first + size;
					int bEnd = b->first + b->size;
					if (first <= b->first && end > b->first && end < bEnd) {
						b->sourceFirst += end - b->first;
						b->first = end;
					}
					else if (first <= b->first && end >= bEnd) {
						doTask(*b, true);
						b = task.erase(b);
						continue;
					}
					else if (first > b->first && first < bEnd && end >= bEnd) {
						b->size = first - b->first;
					}
				}
				b++;
			}
		}
			break;
		default:
			break;
		}

		task.emplace_back(Task{ type, index, source, 0, first, size });
		if (source != NULL)
			source->life++;
	}
}

void DX12GPUBuffer::doTask(Task& task, bool skip)
{
	switch (task.type)
	{
	case TaskType::Release:
		if (!skip)
			releaseInternal(task.backBufferIndex);
		break;
	case TaskType::Resize:
		if (!skip)
			resizeInternal(task.backBufferIndex, task.size);
		break;
	case TaskType::Upload:
		if (!skip)
			uploadInternal(task.backBufferIndex, task.first, task.size, task.source->data + task.sourceFirst * desc.cellSize);
		//OutputDebugStringA(("[Brane]--------Index(" + to_string(task.backBufferIndex) + ")--------\n").c_str());
		//OutputDebugStringA((string("[Brane] ") + getGPUBufferTypeName(desc.type) + "\n").c_str());
		task.source->life--;
		//OutputDebugStringA(("[Brane] source(" + to_string((long long)task.source->data) + ") life" + to_string(task.source->life) + "\n").c_str());
		if (task.source->life == 0) {
			delete[] task.source->data;
			delete task.source;
			//OutputDebugStringA("[Brane] source delete\n");
		}
		//OutputDebugStringA("[Brane]------------------------\n");
		break;
	default:
		break;
	}
}

void DX12GPUBuffer::processTasks(int backBufferIndex, bool skip)
{
	list<Task>& task = tasks[backBufferIndex];
	auto iter = task.begin();
	auto end = task.end();
	while (iter != end) {
		doTask(*iter, skip);
		iter++;
	}
	task.clear();
}

void DX12GPUBuffer::resizeInternal(int backBufferIndex, unsigned int size)
{
	backBufferIndex = desc.cpuAccess == CAF_Read || desc.cpuAccess == CAF_ReadWrite ? 0 : backBufferIndex;
	DX12SubBuffer*& dx12Buffer = dx12Buffers[backBufferIndex];
	unsigned int alignedSize = size * desc.cellSize;
	if (desc.cpuAccess == CAF_Read || desc.cpuAccess == CAF_ReadWrite)
		dx12Buffer = dxContext.readBackBufferPool.suballocate(alignedSize, desc.cellSize);
	else {
		switch (desc.type)
		{
		case GB_Storage:
		case GB_Vertex:
		case GB_Index:
			dx12Buffer = dxContext.constantBufferPool.suballocate(alignedSize, desc.cellSize);
			break;
		case GB_Constant:
			alignedSize = GRS_UPPER(alignedSize, 256);
			dx12Buffer = dxContext.constantBufferPool.suballocate(alignedSize, 256);
			break;
		default:
			break;
		}
	}
}

void DX12GPUBuffer::releaseInternal(int backBufferIndex)
{
	backBufferIndex = desc.cpuAccess == CAF_Read || desc.cpuAccess == CAF_ReadWrite ? 0 : backBufferIndex;
	DX12SubBuffer*& dx12Buffer = dx12Buffers[backBufferIndex];
	if (dx12Buffer != NULL) {
		dx12Buffer->release();
		dx12Buffer = NULL;
	}
}

void DX12GPUBuffer::uploadInternal(int backBufferIndex, unsigned int first, unsigned int size, void* data)
{
	backBufferIndex = desc.cpuAccess == CAF_Read || desc.cpuAccess == CAF_ReadWrite ? 0 : backBufferIndex;
	DX12SubBuffer*& dx12Buffer = dx12Buffers[dxContext.activeBackBufferIndex];
	dx12Buffer->upload(data, size * desc.cellSize, first * desc.cellSize);
}
