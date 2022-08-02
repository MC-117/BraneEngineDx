#pragma once
#include "../../Core/IGPUBuffer.h"

#ifdef VENDOR_USE_DX11

#ifndef _DX11GPUBUFFER_H_
#define _DX11GPUBUFFER_H_

#include "DX11.h"

class DX11GPUBuffer : public IGPUBuffer
{
public:
	DX11Context& dxContext;
	ComPtr<ID3D11Buffer> dx11Buffer;
	ComPtr<ID3D11ShaderResourceView> dx11BufferView;

	DX11GPUBuffer(DX11Context& context, GPUBufferDesc& desc);
	virtual ~DX11GPUBuffer();

	virtual unsigned int bind();
	virtual unsigned int resize(unsigned int size);
	virtual void release();
	virtual unsigned int bindBase(unsigned int index);
	virtual unsigned int uploadData(unsigned int size, void* data);
	virtual unsigned int uploadSubData(unsigned int first, unsigned int size, void* data);
	virtual bool readData(void* data);
	virtual bool readSubData(unsigned int first, unsigned int size, void* data);
};

#endif // !_DX11GPUBUFFER_H_

#endif // VENDOR_USE_DX11

