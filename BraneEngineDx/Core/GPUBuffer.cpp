#include "GPUBuffer.h"

GPUBuffer::GPUBuffer(unsigned int type, unsigned int cellSize, unsigned int size) : type(type), cellSize(cellSize), size(size)
{
	if (size != 0)
		resize(size);
}

GPUBuffer::~GPUBuffer()
{
	if (id != 0) {
		unbind();
		glDeleteBuffers(1, &id);
	}
}

unsigned int GPUBuffer::bind()
{
	resize(size);
	glBindBuffer(type, id);
	return id;
}

unsigned int GPUBuffer::bindBase(unsigned int index)
{
	if (id == 0)
		return 0;
	glBindBufferBase(type, index, id);
	return id;
}

void GPUBuffer::unbind()
{
	glBindBuffer(type, 0);
}

unsigned int GPUBuffer::resize(unsigned int size)
{
	if (size == 0) {
		capacity = 0;
		unbind();
		if (id != 0) {
			glDeleteBuffers(1, &id);
			id = 0;
		}
	}
	else if (size > capacity || size < capacity / 2) {
		capacity = size * 1.5;
		unbind();
		if (id != 0)
			glDeleteBuffers(1, &id);
		glGenBuffers(1, &id);
		glBindBuffer(type, id);
		glBufferData(type, capacity * cellSize, NULL, GL_DYNAMIC_DRAW);
	}
	this->size = size;
	return id;
}

unsigned int GPUBuffer::uploadSubData(unsigned int first, unsigned int size, void* data)
{
	if (first + size > this->size) {
		throw overflow_error("GL buffer access overflow");
	}
	bind();
	glBufferSubData(type, first * cellSize, size * cellSize, data);
	return id;
}

unsigned int GPUBuffer::uploadData(unsigned int size, void* data)
{
	resize(size);
	bind();
	glBufferSubData(type, 0, size * cellSize, data);
	return id;
}
