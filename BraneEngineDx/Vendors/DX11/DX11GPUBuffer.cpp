#include "DX11GPUBuffer.h"

DX11GPUBuffer::DX11GPUBuffer(const DX11Context& context, GPUBufferDesc& desc)
    : dxContext(context), IGPUBuffer(desc)
{
}

DX11GPUBuffer::~DX11GPUBuffer()
{
	release();
}

unsigned int DX11GPUBuffer::resize(unsigned int size)
{
	if (size == 0) {
		release();
	}
	else if (size > desc.capacity || size < desc.capacity / 2) {
		release();
		desc.capacity = size * 1.5;

		D3D11_BUFFER_DESC bd;
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = desc.capacity * desc.cellSize;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.MiscFlags = 0;
		bd.StructureByteStride = desc.cellSize;
		dxContext.device->CreateBuffer(&bd, NULL, &dx11Buffer);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = desc.size;
		dxContext.device->CreateShaderResourceView(dx11Buffer, &srvDesc, &dx11BufferView);
		desc.id = (unsigned int)dx11BufferView;
	}
	desc.size = size;
	return desc.id;
}

unsigned int DX11GPUBuffer::release()
{
	if (dx11Buffer != 0) {
		dx11BufferView->Release();
		dx11BufferView = NULL;
		dx11Buffer->Release();
		dx11Buffer = NULL;
		desc.capacity = 0;
		desc.size = 0;
	}
}

unsigned int DX11GPUBuffer::bindBase(unsigned int index)
{
	if (dx11BufferView == NULL)
		return 0;
	dxContext.deviceContext->VSSetShaderResources(index, 1, &dx11BufferView);
    return desc.id;
}

unsigned int DX11GPUBuffer::uploadSubData(unsigned int first, unsigned int size, void* data)
{
    return 0;
}
