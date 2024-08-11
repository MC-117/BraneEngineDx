#include "DX11Material.h"

#ifdef VENDOR_USE_DX11

DX11Material::DX11Material(DX11Context& context, MaterialDesc& desc)
	: dxContext(context), IMaterial(desc)
{
}

DX11Material::~DX11Material()
{
	release();
}

void DX11Material::uploadAttribute(const Name& name, unsigned int size, void* data)
{
	if (matInsBufHost == NULL || program == NULL)
		return;
	const DX11ShaderProgram::AttributeDesc* desc = program->getAttributeOffset(name);
	if (desc == NULL)
		return;
	const ShaderProperty* prop = desc->getParameter();
	if (prop == NULL)
		return;
	memcpy_s(matInsBufHost + prop->offset, prop->size, data, size);
}

void DX11Material::uploadTexture(const Name& name, ComPtr<ID3D11ShaderResourceView> tex, ComPtr<ID3D11SamplerState> sample)
{
	if (program == NULL || tex == NULL)
		return;
	((DX11ShaderProgram*)program)->bindSRVWithSampler(dxContext.deviceContext, name, tex, sample);
}

void DX11Material::uploadImage(const Name& name, ComPtr<ID3D11UnorderedAccessView> tex)
{
	if (program == NULL || tex == NULL)
		return;
	((DX11ShaderProgram*)program)->bindUAV(dxContext.deviceContext, name, tex);
}

void DX11Material::preprocess()
{
	//dxContext.clearUAV();
	//dxContext.clearSRV();

	int bufSize = program->getMaterialBufferSize();
	if (bufSize > 0 && matInsBufSize != bufSize) {
		if (matInsBuf != NULL)
			matInsBuf.Reset();
		if (matInsBufHost)
			delete[] matInsBufHost;
		matInsBufSize = bufSize;
		D3D11_BUFFER_DESC cbDesc;
		ZeroMemory(&cbDesc, sizeof(D3D11_BUFFER_DESC));
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.ByteWidth = matInsBufSize;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		dxContext.device->CreateBuffer(&cbDesc, NULL, &matInsBuf);
		matInsBufHost = new unsigned char[matInsBufSize];
	}
}

void DX11Material::processBaseData()
{
}

void DX11Material::processScalarData()
{
	if (program == NULL)
		return;
	for (auto b = desc.scalarField.begin(), e = desc.scalarField.end(); b != e; b++)
		uploadAttribute(b->first, sizeof(float), &b->second.val);
}

void DX11Material::processCountData()
{
	if (program == NULL)
		return;
	for (auto b = desc.countField.begin(), e = desc.countField.end(); b != e; b++)
		uploadAttribute(b->first, sizeof(int), &b->second.val);
}

void DX11Material::processColorData()
{
	if (program == NULL)
		return;
	for (auto b = desc.colorField.begin(), e = desc.colorField.end(); b != e; b++)
		uploadAttribute(b->first, sizeof(Color), &b->second.val);
}

void DX11Material::processMatrixData()
{
	if (program == NULL)
		return;
	for (auto b = desc.matrixField.begin(), e = desc.matrixField.end(); b != e; b++)
		uploadAttribute(b->first, sizeof(Matrix4f), b->second.val.data());
}

void DX11Material::processTextureData()
{
	if (program == NULL)
		return;
	for (auto b = desc.textureField.begin(), e = desc.textureField.end(); b != e; b++) {
		if (b->second.val == NULL || b->second.val->bind() == 0)
			continue;
		DX11Texture2D* tex = (DX11Texture2D*)b->second.val->getVendorTexture();
		uploadTexture(b->first, tex->getSRV(), tex->getSampler());
	}
}

void DX11Material::processImageData()
{
	if (program == NULL)
		return;
	if (!desc.imageField.empty())
		dxContext.clearRTV();
	for (auto b = desc.imageField.begin(), e = desc.imageField.end(); b != e; b++) {
		if (!b->second.val.isValid())
			continue;
		if (b->second.val.texture->bind() == 0)
			continue;
		DX11Texture2D* tex = (DX11Texture2D*)b->second.val.texture->getVendorTexture();
		RWOption rwOption;
		rwOption.dimension = b->second.val.dimension;
		rwOption.mipLevel = b->second.val.level;
		rwOption.arrayBase = b->second.val.arrayBase;
		rwOption.arrayCount = b->second.val.arrayCount;
		uploadImage(b->first, tex->getUAV(rwOption));
	}
}

void DX11Material::postprocess()
{
	if (program == NULL)
		return;
	if (matInsBuf) {
		D3D11_MAPPED_SUBRESOURCE mpd;
		dxContext.deviceContext->Map(matInsBuf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mpd);
		memcpy_s(mpd.pData, matInsBufSize, matInsBufHost, matInsBufSize);
		dxContext.deviceContext->Unmap(matInsBuf.Get(), 0);
		((DX11ShaderProgram*)program)->bindCBV(dxContext.deviceContext, DX11ShaderStage::materialParameterBufferName, matInsBuf);
	}
}

void DX11Material::release()
{
	if (matInsBuf)
		matInsBuf.Reset();
	if (matInsBufHost)
		delete[] matInsBufHost;
	dx11UAVs.clear();
}

#endif // VENDOR_USE_DX11
