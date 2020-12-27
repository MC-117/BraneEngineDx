#pragma once
#ifndef _GPUBUFFER_H_
#define _GPUBUFFER_H_

#include "IVendor.h"

class GPUBuffer
{
public:
	GPUBuffer(GPUBufferType type, unsigned int cellSize, unsigned int size = 0);
	~GPUBuffer();

	unsigned int size() const;
	bool empty() const;

	unsigned int bind();
	unsigned int bindBase(unsigned int index);
	unsigned int resize(unsigned int size);
	unsigned int uploadSubData(unsigned int first, unsigned int size, void* data);
	unsigned int uploadData(unsigned int size, void* data);
protected:
	GPUBufferDesc desc;
	IGPUBuffer* vendorGPUBuffer = NULL;

	void newVendorGPUBuffer();
};

#endif // !_GPUBUFFER_H_
