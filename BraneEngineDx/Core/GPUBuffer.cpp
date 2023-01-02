#include "GPUBuffer.h"
#include "Utility/RenderUtility.h"

GPUBuffer::GPUBuffer(GPUBufferType type, GPUBufferFormat format, unsigned int structSize, GPUAccessFlag gpuAccess, CPUAccessFlag cpuAccess)
{
	desc.type = type;
	desc.format = format;
	if (format == GBF_Struct)
		desc.cellSize = structSize;
	else
		desc.cellSize = getGPUBufferFormatCellSize(format);
	desc.gpuAccess = gpuAccess;
	desc.cpuAccess = cpuAccess;
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
