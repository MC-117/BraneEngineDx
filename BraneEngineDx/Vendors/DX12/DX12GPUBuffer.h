#pragma once
#include "../../Core/IGPUBuffer.h"

#ifdef VENDOR_USE_DX12

#include "DX12Context.h"

class DX12GPUBuffer : public IGPUBuffer
{
public:
	DX12Context& dxContext;
	DX12SubBuffer* dx12Buffers[DX12Context::backBufferCount] = { NULL };

	DX12GPUBuffer(DX12Context& context, GPUBufferDesc& desc);
	virtual ~DX12GPUBuffer();

	virtual unsigned int bind();
	virtual unsigned int resize(unsigned int size);
	virtual void release();
	virtual unsigned int bindBase(unsigned int index, BufferOption bufferOption = BufferOption());
	virtual unsigned int uploadData(unsigned int size, void* data);
	virtual unsigned int uploadSubData(unsigned int first, unsigned int size, void* data);
	virtual bool readData(void* data);
	virtual bool readSubData(unsigned int first, unsigned int size, void* data);
protected:
	enum struct TaskType : unsigned char
	{
		Release, Resize, Upload
	};

	struct UploadSource
	{
		char* data;
		int life;
	};

	struct Task
	{
		TaskType type;
		int backBufferIndex;
		UploadSource* source;
		int sourceFirst;
		int first;
		int size;
	};

	list<Task> tasks[DX12Context::backBufferCount];

	virtual void addTask(TaskType type, int backBufferIndex, int count, int size = 0, UploadSource* source = NULL, int first = 0);
	virtual void doTask(Task& task, bool skip = false);
	virtual void processTasks(int backBufferIndex, bool skip = false);

	virtual void resizeInternal(int backBufferIndex, unsigned int size);
	virtual void releaseInternal(int backBufferIndex);
	virtual void uploadInternal(int backBufferIndex, unsigned int first, unsigned int size, void* data);
};

#endif // VENDOR_USE_DX12