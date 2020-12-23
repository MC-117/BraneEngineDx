#pragma once
#include "../../Core/IGPUBuffer.h"
#include "../../Core/IGPUBuffer.h"

#ifdef VENDOR_USE_DX11

#ifndef _DX11GPUBUFFER_H_
#define _DX11GPUBUFFER_H_

#include "DX11.h"

class DX11GPUBuffer : public IGPUBuffer
{
public:
	const DX11Context& dxContext;
	ID3D11Buffer* dx11Buffer;
	ID3D11ShaderResourceView* dx11BufferView;

	DX11GPUBuffer(const DX11Context& context, GPUBufferDesc& desc);
	virtual ~DX11GPUBuffer();

	virtual unsigned int resize(unsigned int size);
	virtual unsigned int release();
	virtual unsigned int bindBase(unsigned int index);
	virtual unsigned int uploadSubData(unsigned int first, unsigned int size, void* data);
};

#endif // !_DX11GPUBUFFER_H_

#endif // VENDOR_USE_DX11

