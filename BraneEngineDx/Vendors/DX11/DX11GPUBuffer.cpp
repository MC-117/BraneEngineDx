#include "DX11GPUBuffer.h"

#ifdef VENDOR_USE_DX11

DX11GPUBuffer::DX11GPUBuffer(DX11Context& context, GPUBufferDesc& desc)
    : dxContext(context), IGPUBuffer(desc)
{
}

DX11GPUBuffer::~DX11GPUBuffer()
{
	release();
}

unsigned int DX11GPUBuffer::bind()
{
	return resize(desc.size);
}

unsigned int DX11GPUBuffer::resize(unsigned int size)
{
	/*if (desc.cellSize % 16 != 0)
		throw runtime_error("GPUBuffer must be aligned by 16 bytes");*/
	if (size == 0) {
		release();
	}
	else if (size > desc.capacity || size < desc.capacity / 2) {
		release();
		desc.capacity = size * 1.5;

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = desc.capacity * desc.cellSize;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.BindFlags = desc.type == GB_Storage ? D3D11_BIND_SHADER_RESOURCE : D3D11_BIND_CONSTANT_BUFFER;
		if (desc.type == GB_Command)
			bd.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
		if (FAILED(dxContext.device->CreateBuffer(&bd, NULL, &dx11Buffer)))
			throw runtime_error("CreateBuffer failed");

		if (desc.type == GB_Storage) {
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = size;
			if (FAILED(dxContext.device->CreateShaderResourceView(dx11Buffer, &srvDesc, &dx11BufferView)))
				throw runtime_error("CreateShaderResourceView failed");
			desc.id = (unsigned int)dx11BufferView;
		}
		else
			desc.id = (unsigned int)dx11Buffer;
	}
	desc.size = size;
	return desc.id;
}

void DX11GPUBuffer::release()
{
	if (dx11BufferView != NULL) {
		dx11BufferView->Release();
		dx11BufferView = NULL;
	}
	if (dx11Buffer != NULL) {
		dx11Buffer->Release();
		dx11Buffer = NULL;
	}
	desc.capacity = 0;
	desc.size = 0;
	desc.id = 0;
}

unsigned int DX11GPUBuffer::bindBase(unsigned int index)
{
	if (dx11BufferView == NULL)
		return 0;
	switch (desc.type)
	{
	case GB_Constant:
		dxContext.deviceContext->VSSetConstantBuffers(index, 1, &dx11Buffer);
		dxContext.deviceContext->PSSetConstantBuffers(index, 1, &dx11Buffer);
		dxContext.deviceContext->GSSetConstantBuffers(index, 1, &dx11Buffer);
		dxContext.deviceContext->HSSetConstantBuffers(index, 1, &dx11Buffer);
		dxContext.deviceContext->DSSetConstantBuffers(index, 1, &dx11Buffer);
		break;
	case GB_Storage:
		dxContext.deviceContext->VSSetShaderResources(index, 1, &dx11BufferView);
		dxContext.deviceContext->PSSetShaderResources(index, 1, &dx11BufferView);
		dxContext.deviceContext->GSSetShaderResources(index, 1, &dx11BufferView);
		dxContext.deviceContext->HSSetShaderResources(index, 1, &dx11BufferView);
		dxContext.deviceContext->DSSetShaderResources(index, 1, &dx11BufferView);
		break;
	default:
		throw runtime_error("Unknown Error");
		break;
	}
    return desc.id;
}

unsigned int DX11GPUBuffer::uploadSubData(unsigned int first, unsigned int size, void* data)
{
	if (size == 0)
		return desc.id;
	resize(size);
	D3D11_MAPPED_SUBRESOURCE mpd;
	dxContext.deviceContext->Map(dx11Buffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mpd);
	memcpy_s((char*)mpd.pData + first * desc.cellSize, size * desc.cellSize, data, size * desc.cellSize);
	dxContext.deviceContext->Unmap(dx11Buffer, 0);
    return desc.id;
}

#endif // VENDOR_USE_DX11