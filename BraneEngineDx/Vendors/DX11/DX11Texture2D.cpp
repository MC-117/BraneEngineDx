#include "DX11Texture2D.h"
#include "../../Core/Utility.h"

#ifdef VENDOR_USE_DX11

DX11Texture2DInfo::DX11Texture2DInfo(const Texture2DInfo & info)
{
	texture2DDesc.Format = toDX11InternalType(info.internalType);
	texture2DDesc.BindFlags = info.internalType == TIT_Depth ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_SHADER_RESOURCE;
	texture2DDesc.Usage = D3D11_USAGE_DEFAULT;
	texture2DDesc.ArraySize = 1;
	texture2DDesc.SampleDesc.Count = info.sampleCount;
	texture2DDesc.SampleDesc.Quality = 0;
	samplerDesc.Filter = toDX11FilterType(info.minFilterType, info.magFilterType);
	samplerDesc.AddressU = toDX11WrapType(info.wrapSType);
	samplerDesc.AddressV = toDX11WrapType(info.wrapTType);
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	memcpy(samplerDesc.BorderColor, &info.borderColor, sizeof(float) * 4);
}

DX11Texture2DInfo& DX11Texture2DInfo::operator=(const Texture2DInfo& info)
{
	texture2DDesc.Format = toDX11InternalType(info.internalType);
	texture2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	if (info.internalType == TIT_Depth)
		texture2DDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
	else
		texture2DDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	texture2DDesc.Usage = D3D11_USAGE_DEFAULT;
	texture2DDesc.ArraySize = 1;
	texture2DDesc.SampleDesc.Count = info.sampleCount;
	texture2DDesc.SampleDesc.Quality = 0;
	samplerDesc.Filter = toDX11FilterType(info.minFilterType, info.magFilterType);
	samplerDesc.AddressU = toDX11WrapType(info.wrapSType);
	samplerDesc.AddressV = toDX11WrapType(info.wrapTType);
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	memcpy(samplerDesc.BorderColor, &info.borderColor, sizeof(float) * 4);
	return *this;
}

D3D11_TEXTURE_ADDRESS_MODE DX11Texture2DInfo::toDX11WrapType(const TexWrapType & type)
{
	switch (type)
	{
	case TW_Repeat:
		return D3D11_TEXTURE_ADDRESS_WRAP;
	case TW_Mirror:
		return D3D11_TEXTURE_ADDRESS_MIRROR;
	case TW_Clamp:
		return D3D11_TEXTURE_ADDRESS_CLAMP;
	case TW_Clamp_Edge:
		return D3D11_TEXTURE_ADDRESS_CLAMP;
	case TW_Border:
		return D3D11_TEXTURE_ADDRESS_BORDER;
	case TW_Mirror_Once:
		return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
	}
	return D3D11_TEXTURE_ADDRESS_WRAP;
}

D3D11_FILTER DX11Texture2DInfo::toDX11FilterType(const TexFilter& minType, const TexFilter& magType)
{
	D3D11_FILTER_TYPE minFT = D3D11_FILTER_TYPE_POINT,
		magFT = D3D11_FILTER_TYPE_POINT,
		mipFT = D3D11_FILTER_TYPE_POINT;
	switch (minType)
	{
	case TF_Point:
		break;
	case TF_Linear:
		minFT = D3D11_FILTER_TYPE_LINEAR;
		break;
	case TF_Point_Mip_Point:
		break;
	case TF_Linear_Mip_Point:
		minFT = D3D11_FILTER_TYPE_LINEAR;
		break;
	case TF_Point_Mip_Linear:
		mipFT = D3D11_FILTER_TYPE_LINEAR;
		break;
	case TF_Linear_Mip_Linear:
		minFT = D3D11_FILTER_TYPE_LINEAR;
		mipFT = D3D11_FILTER_TYPE_LINEAR;
		break;
	}
	switch (magType)
	{
	case TF_Point:
		break;
	case TF_Linear:
		magFT = D3D11_FILTER_TYPE_LINEAR;
		break;
	case TF_Point_Mip_Point:
		break;
	case TF_Linear_Mip_Point:
		magFT = D3D11_FILTER_TYPE_LINEAR;
		break;
	case TF_Point_Mip_Linear:
		mipFT = D3D11_FILTER_TYPE_LINEAR;
		break;
	case TF_Linear_Mip_Linear:
		magFT = D3D11_FILTER_TYPE_LINEAR;
		mipFT = D3D11_FILTER_TYPE_LINEAR;
		break;
	}
	return D3D11_ENCODE_BASIC_FILTER(minFT, magFT, mipFT, 0);
}

DXGI_FORMAT DX11Texture2DInfo::toDX11InternalType(const TexInternalType & type)
{
	switch (type)
	{
	case TIT_Default:
		return DXGI_FORMAT_UNKNOWN;
	case TIT_R:
		return DXGI_FORMAT_R8_UNORM;
	case TIT_RG:
		return DXGI_FORMAT_R8G8_UNORM;
	case TIT_RGBA:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case TIT_SRGBA:
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	case TIT_Depth:
		return DXGI_FORMAT_R32_TYPELESS;
	}
	return DXGI_FORMAT_UNKNOWN;
}

DXGI_FORMAT DX11Texture2DInfo::toDX11ColorType(const TexInternalType& type)
{
	switch (type)
	{
	case TIT_Default:
		return DXGI_FORMAT_UNKNOWN;
	case TIT_R:
		return DXGI_FORMAT_R8_UNORM;
	case TIT_RG:
		return DXGI_FORMAT_R8G8_UNORM;
	case TIT_RGBA:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case TIT_SRGBA:
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	case TIT_Depth:
		return DXGI_FORMAT_R32_FLOAT;
	}
	return DXGI_FORMAT_UNKNOWN;
}

DX11Texture2D::DX11Texture2D(DX11Context& context, Texture2DDesc & desc)
	: dxContext(context), ITexture2D(desc)
{
}

DX11Texture2D::~DX11Texture2D()
{
	release();
}

bool DX11Texture2D::isValid() const
{
	return false;
}

unsigned long long DX11Texture2D::getTextureID()
{
	return (unsigned long long)getSRV();
}

void DX11Texture2D::release()
{
	if (dx11Texture2DView != NULL) {
		dx11Texture2DView->Release();
		dx11Texture2DView = NULL;
	}
	if (dx11Texture2D != NULL) {
		dx11Texture2D->Release();
		dx11Texture2DView = NULL;
	}
	desc.textureHandle = 0;
}

unsigned int DX11Texture2D::bind()
{
	if (desc.textureHandle)
		return desc.textureHandle;
	info = desc.info;
	info.texture2DDesc.Width = desc.width;
	info.texture2DDesc.Height = desc.height;
	if (desc.info.sampleCount > 1) {
		unsigned int q = 0;
		dxContext.device->CheckMultisampleQualityLevels(info.texture2DDesc.Format, desc.info.sampleCount, &q);
		if (q != 0)
			--q;
		info.texture2DDesc.SampleDesc.Quality = q;
	}
	if (info.texture2DDesc.Format == DXGI_FORMAT_UNKNOWN)
		info.texture2DDesc.Format = getColorType(desc.channel, DXGI_FORMAT_UNKNOWN);
	if (desc.autoGenMip) {
		info.texture2DDesc.MipLevels = 1;
		D3D11_SUBRESOURCE_DATA initData = { desc.data, desc.width * desc.channel * sizeof(unsigned char), 0 };
		if (FAILED(dxContext.device->CreateTexture2D(&info.texture2DDesc, desc.data ? &initData : NULL, &dx11Texture2D)))
			throw runtime_error("DX11Texture2D: CreateTexture2D failed");
		desc.mipLevel = 1 + floor(log2(max(desc.width, desc.height)));
	}
	else {
		info.texture2DDesc.MipLevels = desc.mipLevel;
		D3D11_SUBRESOURCE_DATA* initData = new D3D11_SUBRESOURCE_DATA[desc.mipLevel];
		int offset = 0;
		int _width = desc.width, _height = desc.height;
		for (int level = 0; level < desc.mipLevel; level++) {
			initData[level].pSysMem = (const void*)(desc.data + offset);
			initData[level].SysMemPitch = _width * desc.channel * sizeof(unsigned char);
			initData[level].SysMemSlicePitch = 0;
			offset += desc.channel * _width * _height * sizeof(char);
			_width /= 2, _height /= 2;
		}
		if (FAILED(dxContext.device->CreateTexture2D(&info.texture2DDesc, initData, &dx11Texture2D)))
			throw runtime_error("DX11Texture2D: CreateTexture2D failed");
		if (initData != NULL)
			delete[] initData;
	}
	if (dx11Texture2DView != NULL)
		dx11Texture2DView->Release();
	if (dx11Sampler != NULL)
		dx11Sampler->Release();
	desc.textureHandle = (unsigned int)dx11Texture2D;
	return desc.textureHandle;
}

unsigned int DX11Texture2D::resize(unsigned int width, unsigned int height)
{
	if (desc.width == width && desc.height == height && desc.textureHandle != 0)
		return desc.textureHandle;
	if (desc.data == NULL) {
		ITexture::resize(width, height);
		if (desc.textureHandle != 0) {
			release();
		}
		bind();
	}
	return desc.textureHandle;
}

ID3D11ShaderResourceView* DX11Texture2D::getSRV()
{
	if (bind() == 0)
		return NULL;
	ID3D11ShaderResourceView* q = NULL;
	if (dx11Texture2DView != NULL)
		dx11Texture2DView->QueryInterface(IID_PPV_ARGS(&q));
	if (q == NULL) {
		if (dx11Texture2DView != NULL)
			dx11Texture2DView->Release();
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = desc.info.internalType == TIT_Depth ? DXGI_FORMAT_R32_FLOAT : info.texture2DDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = info.texture2DDesc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		if (FAILED(dxContext.device->CreateShaderResourceView(dx11Texture2D, &srvDesc, (ID3D11ShaderResourceView**)&dx11Texture2DView)))
			throw runtime_error("DX11Tetxture2D: CreateShaderResourceView failed");
	}
	return (ID3D11ShaderResourceView*)dx11Texture2DView;
}

ID3D11SamplerState* DX11Texture2D::getSampler()
{
	if (bind() == 0)
		return NULL;
	if (dx11Sampler != NULL)
		return dx11Sampler;
	if (FAILED(dxContext.device->CreateSamplerState(&info.samplerDesc, &dx11Sampler)))
		throw runtime_error("DX11 create sampler state failed");
	return dx11Sampler;
}

ID3D11UnorderedAccessView* DX11Texture2D::getUAV(unsigned int mipLevel)
{
	if (bind() == 0)
		return NULL;
	ID3D11UnorderedAccessView* q = NULL;
	if (dx11Texture2DView != NULL)
		dx11Texture2DView->QueryInterface(IID_PPV_ARGS(&q));
	if (q == NULL) {
		if (dx11Texture2DView != NULL)
			dx11Texture2DView->Release();
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = desc.info.internalType == TIT_Depth ? DXGI_FORMAT_R32_FLOAT : info.texture2DDesc.Format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = mipLevel;
		if (FAILED(dxContext.device->CreateUnorderedAccessView(dx11Texture2D, &uavDesc, (ID3D11UnorderedAccessView**)&dx11Texture2DView)))
			throw runtime_error("DX11Tetxture2D: CreateUnorderedAccessView failed");
	}
	else {
		ID3D11UnorderedAccessView* uav = (ID3D11UnorderedAccessView*)dx11Texture2DView;
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uav->GetDesc(&uavDesc);
		if (uavDesc.Texture2D.MipSlice != mipLevel) {
			uav->Release();
			uavDesc.Format = desc.info.internalType == TIT_Depth ? DXGI_FORMAT_R32_FLOAT : info.texture2DDesc.Format;
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = mipLevel;
			if (FAILED(dxContext.device->CreateUnorderedAccessView(dx11Texture2D, &uavDesc, (ID3D11UnorderedAccessView**)&dx11Texture2DView)))
				throw runtime_error("DX11Tetxture2D: CreateUnorderedAccessView failed");
		}
	}
	return (ID3D11UnorderedAccessView*)dx11Texture2DView;
}

ID3D11DepthStencilView* DX11Texture2D::getDSV(unsigned int mipLevel)
{
	if (bind() == 0)
		return NULL;
	ID3D11DepthStencilView* q = NULL;
	if (dx11Texture2DView != NULL)
		dx11Texture2DView->QueryInterface(IID_PPV_ARGS(&q));
	if (q == NULL) {
		if (dx11Texture2DView != NULL)
			dx11Texture2DView->Release();
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = mipLevel;
		if (FAILED(dxContext.device->CreateDepthStencilView(dx11Texture2D, &dsvDesc, (ID3D11DepthStencilView**)&dx11Texture2DView)))
			throw runtime_error("DX11Tetxture2D: CreateDepthStencilView failed");
	}
	return (ID3D11DepthStencilView*)dx11Texture2DView;
}

bool DX11Texture2D::assign(unsigned int width, unsigned int height, unsigned channel, const Texture2DInfo & info, unsigned int texHandle, unsigned int bindType)
{
	if (desc.data != NULL)
		return false;
	desc.width = width;
	desc.height = height;
	desc.channel = channel;
	desc.info = info;
	this->info = info;
	desc.bindType = bindType;
	desc.textureHandle = texHandle;
	dx11Texture2D = (ID3D11Texture2D*)texHandle;
	dx11Texture2DView = NULL;
	return true;
}

DXGI_FORMAT DX11Texture2D::getColorType(unsigned int channel, DXGI_FORMAT internalType)
{
	if (internalType == DXGI_FORMAT_UNKNOWN) {
		switch (channel)
		{
		case 1:
			internalType = DXGI_FORMAT_R8_UNORM;
			break;
		case 2:
			internalType = DXGI_FORMAT_R8G8_UNORM;
			break;
		case 4:
			internalType = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		}
		return internalType;
	}
	else if (internalType == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	return internalType;
}

#endif // VENDOR_USE_DX11