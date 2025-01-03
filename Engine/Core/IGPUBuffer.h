#pragma once
#ifndef _IGPUBUFFER_H_
#define _IGPUBUFFER_H_

#include "GraphicType.h"

class ENGINE_API IBufferBinding
{
public:
	virtual void updateBuffer() = 0;
	virtual void bindBuffer() = 0;
};

struct GPUBufferDesc
{
	GPUBufferType type = GB_Constant;
	GPUBufferFormat format = GBF_Float;
	CPUAccessFlag cpuAccess = CAF_None;
	GPUAccessFlag gpuAccess = GAF_Read;
	unsigned int cellSize = 0;
	unsigned int size = 0;
	unsigned int capacity = 0;
	unsigned int id = 0;
};

class ENGINE_API IGPUBuffer
{
public:
	GPUBufferDesc& desc;

	IGPUBuffer(GPUBufferDesc& desc);
	virtual ~IGPUBuffer();

	virtual unsigned int bind() = 0;
	virtual unsigned int resize(unsigned int size) = 0;
	virtual unsigned int bindBase(unsigned int index, BufferOption bufferOption = BufferOption()) = 0;
	virtual unsigned int uploadData(unsigned int size, void* data, bool discard = false) = 0;
	virtual unsigned int uploadSubData(unsigned int first, unsigned int size, void* data) = 0;
	virtual bool readData(void* data) = 0;
	virtual bool readSubData(unsigned int first, unsigned int size, void* data) = 0;
};

#endif // !_IGPUBUFFER_H_
