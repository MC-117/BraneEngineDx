#pragma once
#ifndef _IGPUBUFFER_H_
#define _IGPUBUFFER_H_

#include "Unit.h"

enum GPUBufferType
{
	GB_Constant, GB_Storage, GB_Command, GB_Index, GB_Struct
};

struct GPUBufferDesc
{
	GPUBufferType type = GB_Constant;
	unsigned int cellSize = 0;
	unsigned int size = 0;
	unsigned int capacity = 0;
	unsigned int id = 0;
};

class IGPUBuffer
{
public:
	GPUBufferDesc& desc;

	IGPUBuffer(GPUBufferDesc& desc);
	virtual ~IGPUBuffer();

	virtual unsigned int bind() = 0;
	virtual unsigned int resize(unsigned int size) = 0;
	virtual unsigned int bindBase(unsigned int index) = 0;
	virtual unsigned int uploadData(unsigned int size, void* data) = 0;
	virtual unsigned int uploadSubData(unsigned int first, unsigned int size, void* data) = 0;
};

#endif // !_IGPUBUFFER_H_
