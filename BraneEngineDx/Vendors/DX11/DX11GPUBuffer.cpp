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
		unsigned int alignedSize = ceil(desc.capacity * desc.cellSize / 16.0f) * 16;

		alignedSize = max(64, alignedSize);

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		if (desc.type == GB_ReadBack)
			bd.Usage = D3D11_USAGE_STAGING;
		else
			bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = alignedSize;
		if (desc.type == GB_ReadBack)
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		else
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		if (desc.type == GB_Constant)
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		else if (desc.type == GB_Vertex)
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		else if (desc.type == GB_Index)
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		else if (desc.type != GB_ReadBack)
			bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		bd.StructureByteStride = desc.cellSize;
		if (desc.type == GB_Struct)
			bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		else if (desc.type == GB_Command)
			bd.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
		if (FAILED(dxContext.device->CreateBuffer(&bd, NULL, &dx11Buffer)))
			throw runtime_error("CreateBuffer failed");

		if (desc.type != GB_Constant && desc.type != GB_Vertex && desc.type != GB_Index && desc.type != GB_ReadBack) {
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			if (desc.type == GB_Struct) {
				srvDesc.Format = DXGI_FORMAT_UNKNOWN;
				srvDesc.Buffer.NumElements = desc.capacity;
			}
			else {
				if (desc.cellSize == sizeof(float)) {
					srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
					srvDesc.Buffer.NumElements = alignedSize / 4;
				}
				else {
					srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
					srvDesc.Buffer.NumElements = alignedSize / 16;
				}
			}
			if (FAILED(dxContext.device->CreateShaderResourceView(dx11Buffer.Get(), &srvDesc, &dx11BufferView)))
				throw runtime_error("CreateShaderResourceView failed");
			desc.id = (unsigned int)dx11BufferView.Get();
		}
		else
			desc.id = (unsigned int)dx11Buffer.Get();
	}
	desc.size = size;
	return desc.id;
}

void DX11GPUBuffer::release()
{
	if (dx11BufferView != NULL) {
		dx11BufferView.Reset();
	}
	if (dx11Buffer != NULL) {
		dx11Buffer.Reset();
	}
	desc.capacity = 0;
	desc.size = 0;
	desc.id = 0;
}

unsigned int DX11GPUBuffer::bindBase(unsigned int index)
{
	switch (desc.type)
	{
	case GB_Constant:
		if (dx11Buffer == NULL)
			return 0;
		dxContext.deviceContext->VSSetConstantBuffers(index, 1, dx11Buffer.GetAddressOf());
		dxContext.deviceContext->PSSetConstantBuffers(index, 1, dx11Buffer.GetAddressOf());
		dxContext.deviceContext->GSSetConstantBuffers(index, 1, dx11Buffer.GetAddressOf());
		dxContext.deviceContext->HSSetConstantBuffers(index, 1, dx11Buffer.GetAddressOf());
		dxContext.deviceContext->DSSetConstantBuffers(index, 1, dx11Buffer.GetAddressOf());
		break;
	case GB_Vertex:
	{
		if (dx11Buffer == NULL)
			return 0;
		unsigned int strides = desc.cellSize;
		unsigned int offset = 0;
		dxContext.deviceContext->IASetVertexBuffers(index, 1, dx11Buffer.GetAddressOf(), &strides, &offset);
		break;
	}
	case GB_Index:
	{
		if (dx11Buffer == NULL)
			return 0;
		unsigned int strides = sizeof(unsigned int);
		unsigned int offset = 0;
		dxContext.deviceContext->IASetIndexBuffer(dx11Buffer.Get(), DXGI_FORMAT_R32_UINT, offset);
		break;
	}
	case GB_Storage:
	case GB_Struct:
		if (dx11BufferView == NULL)
			return 0;
		dxContext.deviceContext->VSSetShaderResources(index, 1, dx11BufferView.GetAddressOf());
		dxContext.deviceContext->PSSetShaderResources(index, 1, dx11BufferView.GetAddressOf());
		dxContext.deviceContext->GSSetShaderResources(index, 1, dx11BufferView.GetAddressOf());
		dxContext.deviceContext->HSSetShaderResources(index, 1, dx11BufferView.GetAddressOf());
		dxContext.deviceContext->DSSetShaderResources(index, 1, dx11BufferView.GetAddressOf());
		break;
	default:
		throw runtime_error("Unknown Error");
		break;
	}
    return desc.id;
}

unsigned int DX11GPUBuffer::uploadData(unsigned int size, void* data)
{
	resize(size);
	if (size == 0)
		return desc.id;
	D3D11_MAPPED_SUBRESOURCE mpd;
	dxContext.deviceContext->Map(dx11Buffer.Get(), 0, desc.type == GB_Constant ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mpd);
	memcpy_s((char*)mpd.pData, size * desc.cellSize, data, size * desc.cellSize);
	dxContext.deviceContext->Unmap(dx11Buffer.Get(), 0);
	return desc.id;
}

unsigned int DX11GPUBuffer::uploadSubData(unsigned int first, unsigned int size, void* data)
{
	if (size == 0)
		return desc.id;
	D3D11_MAPPED_SUBRESOURCE mpd;
	dxContext.deviceContext->Map(dx11Buffer.Get(), 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mpd);
	memcpy_s((char*)mpd.pData + first * desc.cellSize, size * desc.cellSize, data, size * desc.cellSize);
	dxContext.deviceContext->Unmap(dx11Buffer.Get(), 0);
    return desc.id;
}

bool DX11GPUBuffer::readData(void* data)
{
	if (desc.size == 0 || data == NULL)
		return false;
	D3D11_MAPPED_SUBRESOURCE mpd;
	dxContext.deviceContext->Map(dx11Buffer.Get(), 0, D3D11_MAP_READ, 0, &mpd);
	if (mpd.pData == NULL)
		return false;
	memcpy_s(data, desc.size * desc.cellSize, (char*)mpd.pData, desc.size * desc.cellSize);
	dxContext.deviceContext->Unmap(dx11Buffer.Get(), 0);
	return true;
}

bool DX11GPUBuffer::readSubData(unsigned int first, unsigned int size, void* data)
{
	if (desc.size == 0 || data == NULL)
		return false;
	D3D11_MAPPED_SUBRESOURCE mpd;
	dxContext.deviceContext->Map(dx11Buffer.Get(), 0, D3D11_MAP_READ, 0, &mpd);
	if (mpd.pData == NULL)
		return false;
	memcpy_s(data, size * desc.cellSize, (char*)mpd.pData + first * desc.cellSize, size * desc.cellSize);
	dxContext.deviceContext->Unmap(dx11Buffer.Get(), 0);
	return true;
}

#endif // VENDOR_USE_DX11