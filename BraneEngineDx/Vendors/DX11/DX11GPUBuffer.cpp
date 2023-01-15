#include "DX11GPUBuffer.h"
#include "../DXGI_Helper.h"

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
	else if (size > desc.capacity /*|| size < desc.capacity / 2*/) {
		release();
		desc.capacity = size;// *1.5;
		unsigned int alignedSize = ceil(desc.capacity * desc.cellSize / 16.0f) * 16;

		//alignedSize = max(64, alignedSize);

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));

		if (desc.cpuAccess == CAF_None)
			bd.Usage = desc.gpuAccess == GAF_ReadWrite ? D3D11_USAGE_DEFAULT : D3D11_USAGE_IMMUTABLE;
		else if (desc.cpuAccess == CAF_Write && desc.gpuAccess == GAF_Read)
			bd.Usage = D3D11_USAGE_DYNAMIC;
		else if (desc.cpuAccess & CAF_Read)
			bd.Usage = D3D11_USAGE_STAGING;
		else
			throw runtime_error("Unknown GPU CPU Access Flag Combination");

		if (desc.cpuAccess & CAF_Read)
			bd.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;

		if (desc.cpuAccess & CAF_Write)
			bd.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;

		if (bd.Usage != D3D11_USAGE_STAGING) {
			switch (desc.type)
			{
			case GB_Constant:
				bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				break;
			case GB_Vertex:
				bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				break;
			case GB_Index:
				bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
				break;
			case GB_Storage:
				bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				bd.MiscFlags = desc.format == GBF_Struct ? D3D11_RESOURCE_MISC_BUFFER_STRUCTURED : 0;
				break;
			case GB_Command:
				bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				bd.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
				break;
			default:
				throw runtime_error("Unknown Buffer Type");
				break;
			}

			if (desc.gpuAccess == GAF_ReadWrite) {
				bd.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
			}

			if (desc.format == GBF_Struct) {
				bd.StructureByteStride = desc.cellSize;
			}

			if (desc.format == GBF_Raw) {
				if (desc.type == GB_Constant)
					throw runtime_error("Constant Buffer cannot use raw format");
				else {
					bd.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
				}
			}
		}

		bd.ByteWidth = alignedSize;
		bd.StructureByteStride = desc.cellSize;

		if (FAILED(dxContext.device->CreateBuffer(&bd, NULL, &dx11Buffer)))
			throw runtime_error("CreateBuffer failed");

		if (desc.cpuAccess & CAF_Read ||
			desc.type == GB_Constant ||
			desc.type == GB_Vertex ||
			desc.type == GB_Index) {
			desc.id = (unsigned int)dx11Buffer.Get();
		}
		else {
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

			if (desc.format == GBF_Raw) {
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
				srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
				srvDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
				srvDesc.BufferEx.FirstElement = 0;
				srvDesc.BufferEx.NumElements = desc.format == GBF_Struct ?
					desc.capacity : alignedSize / desc.cellSize;
			}
			else {
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
				srvDesc.Format = getDXGIFormat(desc.format);
				srvDesc.Buffer.FirstElement = 0;
				srvDesc.Buffer.NumElements = desc.format == GBF_Struct ?
					desc.capacity : alignedSize / desc.cellSize;
			}

			if (FAILED(dxContext.device->CreateShaderResourceView(dx11Buffer.Get(), &srvDesc, &dx11BufferSRV)))
				throw runtime_error("CreateShaderResourceView failed");
			desc.id = (unsigned int)dx11BufferSRV.Get();
		}

		if (!(desc.cpuAccess & CAF_Read) && desc.gpuAccess == GAF_ReadWrite) {
			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
			if (desc.format == GBF_Raw) {
				uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
				uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
				uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
				uavDesc.Buffer.FirstElement = 0;
				uavDesc.Buffer.NumElements = desc.format == GBF_Struct ?
					desc.capacity : alignedSize / desc.cellSize;
			}
			else {
				uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
				uavDesc.Format = getDXGIFormat(desc.format);
				uavDesc.Buffer.FirstElement = 0;
				uavDesc.Buffer.NumElements = desc.format == GBF_Struct ?
					desc.capacity : alignedSize / desc.cellSize;
				uavDesc.Buffer.Flags = 0;
			}
			if (FAILED(dxContext.device->CreateUnorderedAccessView(dx11Buffer.Get(), &uavDesc, &dx11BufferUAV)))
				throw runtime_error("CreateUnorderedAccessView failed");
		}
	}
	desc.size = size;
	return desc.id;
}

void DX11GPUBuffer::release()
{
	if (dx11BufferSRV != NULL) {
		dx11BufferSRV.Reset();
	}
	if (dx11BufferUAV != NULL) {
		dx11BufferUAV.Reset();
	}
	if (dx11Buffer != NULL) {
		dx11Buffer.Reset();
	}
	desc.capacity = 0;
	desc.size = 0;
	desc.id = 0;
}

unsigned int DX11GPUBuffer::bindBase(unsigned int index, BufferOption bufferOption)
{
	if (bufferOption.output && desc.gpuAccess != GAF_ReadWrite)
		throw runtime_error("Buffer with GAF_Read cannot be binded on output");
	if (bufferOption.output) {
		dxContext.deviceContext->OMSetRenderTargetsAndUnorderedAccessViews(D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL,
			NULL, NULL, index, 1, dx11BufferUAV.GetAddressOf(), NULL);
		dxContext.deviceContext->CSSetUnorderedAccessViews(index, 1, dx11BufferUAV.GetAddressOf(), NULL);
	}
	else {
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
			dxContext.deviceContext->VSSetShaderResources(index, 1, dx11BufferSRV.GetAddressOf());
			dxContext.deviceContext->PSSetShaderResources(index, 1, dx11BufferSRV.GetAddressOf());
			dxContext.deviceContext->GSSetShaderResources(index, 1, dx11BufferSRV.GetAddressOf());
			dxContext.deviceContext->HSSetShaderResources(index, 1, dx11BufferSRV.GetAddressOf());
			dxContext.deviceContext->DSSetShaderResources(index, 1, dx11BufferSRV.GetAddressOf());
			dxContext.deviceContext->CSSetShaderResources(index, 1, dx11BufferSRV.GetAddressOf());
			break;
		default:
			throw runtime_error("Unknown Error");
			break;
		}
	}
    return desc.id;
}

unsigned int DX11GPUBuffer::uploadData(unsigned int size, void* data, bool discard)
{
	resize(size);
	if (size == 0)
		return desc.id;
	if ((desc.cpuAccess & CAF_Read) || (desc.cpuAccess == CAF_Write && desc.gpuAccess == GAF_Read)) {
		D3D11_MAPPED_SUBRESOURCE mpd;
		dxContext.deviceContext->Map(dx11Buffer.Get(), 0, desc.type == GB_Constant || discard ?
			D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mpd);
		memcpy_s((char*)mpd.pData, size * desc.cellSize, data, size * desc.cellSize);
		dxContext.deviceContext->Unmap(dx11Buffer.Get(), 0);
	}
	else {
		D3D11_BOX destRegion;
		destRegion.left = 0;
		destRegion.right = desc.cellSize * size;
		destRegion.top = 0;
		destRegion.bottom = 1;
		destRegion.front = 0;
		destRegion.back = 1;
		dxContext.deviceContext->UpdateSubresource(dx11Buffer.Get(), 0, &destRegion, data, desc.cellSize, 0);
	}
	return desc.id;
}

unsigned int DX11GPUBuffer::uploadSubData(unsigned int first, unsigned int size, void* data)
{
	if (size == 0)
		return desc.id;
	if ((desc.cpuAccess & CAF_Read) || (desc.cpuAccess == CAF_Write && desc.gpuAccess == GAF_Read)) {
		D3D11_MAPPED_SUBRESOURCE mpd;
		dxContext.deviceContext->Map(dx11Buffer.Get(), 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mpd);
		memcpy_s((char*)mpd.pData + first * desc.cellSize, size * desc.cellSize, data, size * desc.cellSize);
		dxContext.deviceContext->Unmap(dx11Buffer.Get(), 0);
	}
	else {
		D3D11_BOX destRegion;
		destRegion.left = desc.cellSize * first;
		destRegion.right = desc.cellSize * (first + size);
		destRegion.top = 0;
		destRegion.bottom = 1;
		destRegion.front = 0;
		destRegion.back = 1;
		dxContext.deviceContext->UpdateSubresource(dx11Buffer.Get(), 0, &destRegion, data, desc.cellSize, 0);
	}
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