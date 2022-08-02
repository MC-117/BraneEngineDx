#include "GPUBuffer.h"

GPUBuffer::GPUBuffer(GPUBufferType type, unsigned int cellSize, unsigned int size)
{
	desc.type = type;
	desc.cellSize = cellSize;
	if (size != 0)
		resize(size);
}

GPUBuffer::~GPUBuffer()
{
	if (vendorGPUBuffer != NULL)
		delete vendorGPUBuffer;
}

unsigned int GPUBuffer::size() const
{
	return desc.size;
}

unsigned int GPUBuffer::capacity() const
{
	return desc.capacity;
}

bool GPUBuffer::empty() const
{
	return desc.size == 0;
}

unsigned int GPUBuffer::bind()
{
	newVendorGPUBuffer();
	return vendorGPUBuffer->bind();
}

unsigned int GPUBuffer::bindBase(unsigned int index)
{
	newVendorGPUBuffer();
	return vendorGPUBuffer->bindBase(index);
}

unsigned int GPUBuffer::resize(unsigned int size)
{
	newVendorGPUBuffer();
	return vendorGPUBuffer->resize(size);
}

unsigned int GPUBuffer::uploadSubData(unsigned int first, unsigned int size, void* data)
{
	newVendorGPUBuffer();
	return vendorGPUBuffer->uploadSubData(first, size, data);
}

unsigned int GPUBuffer::uploadData(unsigned int size, void* data)
{
	newVendorGPUBuffer();
	return vendorGPUBuffer->uploadData(size, data);
}

bool GPUBuffer::readData(void* data)
{
	newVendorGPUBuffer();
	return vendorGPUBuffer->readData(data);
}

bool GPUBuffer::readSubData(unsigned int first, unsigned int size, void* data)
{
	newVendorGPUBuffer();
	return vendorGPUBuffer->readSubData(first, size, data);
}

IGPUBuffer* GPUBuffer::getVendorGPUBuffer()
{
	return vendorGPUBuffer;
}

void GPUBuffer::newVendorGPUBuffer()
{
	if (vendorGPUBuffer == NULL) {
		vendorGPUBuffer = VendorManager::getInstance().getVendor().newGPUBuffer(desc);
		if (vendorGPUBuffer == NULL) {
			throw runtime_error("Vendor new GPUBuffer failed");
		}
	}
}
