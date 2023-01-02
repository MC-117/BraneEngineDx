#pragma once
#ifndef _GPUBUFFER_H_
#define _GPUBUFFER_H_

#include "IVendor.h"

class GPUBuffer
{
public:
	GPUBuffer(GPUBufferType type, GPUBufferFormat format, unsigned int structSize = 0, GPUAccessFlag gpuAccess = GAF_Read, CPUAccessFlag cpuAccess = CAF_Write);
	~GPUBuffer();

	unsigned int size() const;
	unsigned int capacity() const;
	bool empty() const;

	unsigned int bind();
	unsigned int bindBase(unsigned int index);
	unsigned int resize(unsigned int size);
	unsigned int uploadSubData(unsigned int first, unsigned int size, void* data);
	unsigned int uploadData(unsigned int size, void* data);
	bool readData(void* data);
	bool readSubData(unsigned int first, unsigned int size, void* data);

	IGPUBuffer* getVendorGPUBuffer();
protected:
	GPUBufferDesc desc;
	IGPUBuffer* vendorGPUBuffer = NULL;

	void newVendorGPUBuffer();
};

#endif // !_GPUBUFFER_H_
