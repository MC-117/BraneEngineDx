#pragma once
#ifndef _GPUBUFFER_H_
#define _GPUBUFFER_H_

#include "IGPUBuffer.h"

class GPUBuffer
{
public:
	unsigned int type;
	unsigned int cellSize = 1;
	unsigned int size = 0;
	unsigned int capacity = 0;

	GPUBuffer(unsigned int type, unsigned int cellSize, unsigned int size = 0);
	~GPUBuffer();
	unsigned int bind();
	unsigned int bindBase(unsigned int index);
	void unbind();
	unsigned int resize(unsigned int size);
	unsigned int uploadSubData(unsigned int first, unsigned int size, void* data);
	unsigned int uploadData(unsigned int size, void* data);
protected:
	unsigned int id = 0;
};

#endif // !_GPUBUFFER_H_
