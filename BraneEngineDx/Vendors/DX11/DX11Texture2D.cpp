#include "DX11Texture2D.h"
#include "../../Core/Utility/Utility.h"

#ifdef VENDOR_USE_DX11

DX11Texture2DInfo::DX11Texture2DInfo(const Texture2DInfo & info)
{
	texture2DDesc.Format = toDX11InternalType(info.internalType);
	texture2DDesc.Usage = D3D11_USAGE_DEFAULT;
	if (info.cpuAccessFlag == CAF_Read) {
		texture2DDesc.Usage = D3D11_USAGE_STAGING;
		texture2DDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	}
	else if (info.cpuAccessFlag == CAF_Write) {
		texture2DDesc.Usage = D3D11_USAGE_DYNAMIC;
		texture2DDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if (info.cpuAccessFlag == CAF_ReadWrite) {
		texture2DDesc.Usage = D3D11_USAGE_DYNAMIC;
		texture2DDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	}
	if (texture2DDesc.Usage != D3D11_USAGE_STAGING) {
		texture2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		if (info.internalType == TIT_D32_F)
			texture2DDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
		else if (info.sampleCount > 1)
			texture2DDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		else
			texture2DDesc.BindFlags |= D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
	}
	texture2DDesc.ArraySize = 1;
	texture2DDesc.SampleDesc.Count = info.sampleCount == 0 ? 1 : info.sampleCount;
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
	texture2DDesc.Usage = D3D11_USAGE_DEFAULT;
	if (info.cpuAccessFlag == CAF_Read) {
		texture2DDesc.Usage = D3D11_USAGE_STAGING;
		texture2DDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	}
	else if (info.cpuAccessFlag == CAF_Write) {
		texture2DDesc.Usage = D3D11_USAGE_DYNAMIC;
		texture2DDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if (info.cpuAccessFlag == CAF_ReadWrite) {
		texture2DDesc.Usage = D3D11_USAGE_DYNAMIC;
		texture2DDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	}
	if (texture2DDesc.Usage != D3D11_USAGE_STAGING) {
		texture2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		if (info.internalType == TIT_D32_F)
			texture2DDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
		else if (info.sampleCount > 1)
			texture2DDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		else
			texture2DDesc.BindFlags |= D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
	}
	texture2DDesc.ArraySize = 1;
	texture2DDesc.SampleDesc.Count = info.sampleCount == 0 ? 1 : info.sampleCount;
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
	case TIT_R8_UF:
		return DXGI_FORMAT_R8_UNORM;
	case TIT_R8_F:
		return DXGI_FORMAT_R8_SNORM;
	case TIT_RG8_UF:
		return DXGI_FORMAT_R8G8_UNORM;
	case TIT_RG8_F:
		return DXGI_FORMAT_R8G8_SNORM;
	case TIT_RGBA8_UF:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case TIT_RGBA8_F:
		return DXGI_FORMAT_R8G8B8A8_SNORM;
	case TIT_SRGBA8_UF:
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	case TIT_RGB10A2_UF:
		return DXGI_FORMAT_R10G10B10A2_UNORM;
	case TIT_D32_F:
		return DXGI_FORMAT_R32_TYPELESS;
	case TIT_R32_F:
		return DXGI_FORMAT_R32_FLOAT;
	case TIT_RGBA8_UI:
		return DXGI_FORMAT_R8G8B8A8_UINT;
	case TIT_RGBA8_I:
		return DXGI_FORMAT_R8G8B8A8_SINT;
	}
	return DXGI_FORMAT_UNKNOWN;
}

DXGI_FORMAT DX11Texture2DInfo::toDX11ColorType(const TexInternalType& type)
{
	switch (type)
	{
	case TIT_Default:
		return DXGI_FORMAT_UNKNOWN;
	case TIT_R8_UF:
		return DXGI_FORMAT_R8_UNORM;
	case TIT_R8_F:
		return DXGI_FORMAT_R8_SNORM;
	case TIT_RG8_UF:
		return DXGI_FORMAT_R8G8_UNORM;
	case TIT_RG8_F:
		return DXGI_FORMAT_R8G8_SNORM;
	case TIT_RGBA8_UF:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case TIT_RGBA8_F:
		return DXGI_FORMAT_R8G8B8A8_SNORM;
	case TIT_SRGBA8_UF:
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	case TIT_RGB10A2_UF:
		return DXGI_FORMAT_R10G10B10A2_UNORM;
	case TIT_D32_F:
		return DXGI_FORMAT_R32_FLOAT;
	case TIT_R32_F:
		return DXGI_FORMAT_R32_FLOAT;
	case TIT_RGBA8_UI:
		return DXGI_FORMAT_R8G8B8A8_UINT;
	case TIT_RGBA8_I:
		return DXGI_FORMAT_R8G8B8A8_SINT;
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
	return desc.height > 0 && desc.width > 0 && desc.channel > 0;
}

unsigned long long DX11Texture2D::getTextureID()
{
	if (isValid())
		return (unsigned long long)this;
	else
		return 0;
}

void DX11Texture2D::release()
{
	if (dx11SRV != NULL) {
		dx11SRV.Reset();
	}
	if (dx11RTV != NULL) {
		dx11RTV.Reset();
	}
	if (dx11UAV != NULL) {
		dx11UAV.Reset();
	}
	if (dx11DSV != NULL) {
		dx11DSV.Reset();
	}
	if (dx11Texture2D != NULL) {
		dx11Texture2D.Reset();
	}
	if (dx11Sampler != NULL) {
		dx11Sampler.Reset();
	}
	desc.textureHandle = 0;
}

unsigned int DX11Texture2D::bind()
{
	if (desc.textureHandle && !desc.needUpdate)
		return desc.textureHandle;
	release();
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
		if (desc.data) {
			info.texture2DDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}
		if (desc.info.sampleCount <= 1)
			info.texture2DDesc.MipLevels = 1 + floor(log2(max(desc.width, desc.height)));
		else
			info.texture2DDesc.MipLevels = 1;
		//D3D11_SUBRESOURCE_DATA initData = { desc.data, desc.width * desc.channel * sizeof(unsigned char), 0 };
		if (FAILED(dxContext.device->CreateTexture2D(&info.texture2DDesc, NULL, &dx11Texture2D)))
			throw runtime_error("DX11Texture2D: CreateTexture2D failed");
		if (desc.data) {
			/*if (dx11SRV != NULL)
				dx11SRV->Release();*/
			dxContext.deviceContext->UpdateSubresource(dx11Texture2D.Get(), 0, NULL, desc.data, desc.width * desc.channel * sizeof(unsigned char),
				desc.height * desc.width * desc.channel * sizeof(unsigned char));
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = desc.info.internalType == TIT_D32_F ? DXGI_FORMAT_R32_FLOAT : info.texture2DDesc.Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = -1;
			srvDesc.Texture2D.MostDetailedMip = 0;
			if (FAILED(dxContext.device->CreateShaderResourceView(dx11Texture2D.Get(), &srvDesc, &dx11SRV)))
				throw runtime_error("DX11Tetxture2D: CreateShaderResourceView failed");
			dxContext.deviceContext->GenerateMips(dx11SRV.Get());
		}
		desc.mipLevel = info.texture2DDesc.MipLevels;
	}
	else {
		info.texture2DDesc.MipLevels = desc.mipLevel;
		D3D11_SUBRESOURCE_DATA* initData = NULL;
		if (desc.data != NULL) {
			initData = new D3D11_SUBRESOURCE_DATA[desc.mipLevel];
			int offset = 0;
			int _width = desc.width, _height = desc.height;
			for (int level = 0; level < desc.mipLevel; level++) {
				initData[level].pSysMem = (const void*)(desc.data + offset);
				initData[level].SysMemPitch = _width * desc.channel * sizeof(unsigned char);
				initData[level].SysMemSlicePitch = 0;
				offset += desc.channel * _width * _height * sizeof(char);
				_width /= 2, _height /= 2;
			}
		}
		if (FAILED(dxContext.device->CreateTexture2D(&info.texture2DDesc, initData, &dx11Texture2D)))
			throw runtime_error("DX11Texture2D: CreateTexture2D failed");
		if (initData != NULL)
			delete[] initData;
	}
	desc.textureHandle = (unsigned long long)this;
	desc.needUpdate = false;
	return desc.textureHandle;
}

unsigned int DX11Texture2D::bindBase(unsigned int index)
{
	ComPtr<ID3D11ShaderResourceView> srv = getSRV();
	ComPtr<ID3D11SamplerState> sampler = getSampler();

	if (srv == NULL)
		return 0;

	dxContext.deviceContext->VSSetShaderResources(index, 1, srv.GetAddressOf());
	dxContext.deviceContext->PSSetShaderResources(index, 1, srv.GetAddressOf());
	dxContext.deviceContext->GSSetShaderResources(index, 1, srv.GetAddressOf());
	dxContext.deviceContext->HSSetShaderResources(index, 1, srv.GetAddressOf());
	dxContext.deviceContext->DSSetShaderResources(index, 1, srv.GetAddressOf());

	if (sampler != NULL) {
		dxContext.deviceContext->VSSetSamplers(index, 1, sampler.GetAddressOf());
		dxContext.deviceContext->PSSetSamplers(index, 1, sampler.GetAddressOf());
		dxContext.deviceContext->GSSetSamplers(index, 1, sampler.GetAddressOf());
		dxContext.deviceContext->HSSetSamplers(index, 1, sampler.GetAddressOf());
		dxContext.deviceContext->DSSetSamplers(index, 1, sampler.GetAddressOf());
	}

	return desc.textureHandle;
}

unsigned int DX11Texture2D::resize(unsigned int width, unsigned int height)
{
	if (desc.width == width && desc.height == height && desc.textureHandle != 0)
		return desc.textureHandle;
	ITexture::resize(width, height);
	if (desc.textureHandle != 0) {
		release();
	}
	bind();
	return desc.textureHandle;
}

ComPtr<ID3D11ShaderResourceView> DX11Texture2D::getSRV(const MipOption& mipOption)
{
	if (bind() == 0)
		return NULL;

	bool create = false;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	if (dx11SRV == NULL) {
		create = true;
	}
	else {
		dx11SRV->GetDesc(&srvDesc);
		create = (mipOption.mipCount != 0 &&
			srvDesc.Texture2D.MipLevels != mipOption.mipCount) || 
			srvDesc.Texture2D.MostDetailedMip != mipOption.detailMip;
	}

	if (create) {
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.Format = desc.info.internalType == TIT_D32_F ? DXGI_FORMAT_R32_FLOAT : info.texture2DDesc.Format;
		if (desc.info.sampleCount > 1) {
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
		}
		else {
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels =
				(mipOption.mipCount == 0 | mipOption.mipCount > info.texture2DDesc.MipLevels) ?
				info.texture2DDesc.MipLevels : mipOption.mipCount;
			srvDesc.Texture2D.MostDetailedMip = mipOption.detailMip;
		}
		if (FAILED(dxContext.device->CreateShaderResourceView(dx11Texture2D.Get(), &srvDesc, &dx11SRV)))
			throw runtime_error("DX11Tetxture2D: CreateShaderResourceView failed");
	}
	return dx11SRV;
}

ComPtr<ID3D11RenderTargetView> DX11Texture2D::getRTV(unsigned int mipLevel, bool isMS)
{
	if (bind() == 0)
		return NULL;
	bool create = false;
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;

	if (dx11RTV == NULL) {
		create = true;
	}
	else {
		dx11RTV->GetDesc(&rtvDesc);
		create = rtvDesc.Texture2D.MipSlice != mipLevel;
	}

	if (create) {
		/*if (dx11RTV != NULL) {
			dx11RTV->Release();
		}*/
		ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
		rtvDesc.Format = info.texture2DDesc.Format;
		rtvDesc.Texture2D.MipSlice = mipLevel;
		rtvDesc.ViewDimension = isMS ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
		if (FAILED(dxContext.device->CreateRenderTargetView(dx11Texture2D.Get(), &rtvDesc, &dx11RTV)))
			throw runtime_error("DX11Tetxture2D: CreateRenderTargetView failed");
	}
	return dx11RTV;
}

ComPtr<ID3D11SamplerState> DX11Texture2D::getSampler()
{
	if (bind() == 0)
		return NULL;
	if (dx11Sampler != NULL)
		return dx11Sampler;
	if (FAILED(dxContext.device->CreateSamplerState(&info.samplerDesc, &dx11Sampler)))
		throw runtime_error("DX11 create sampler state failed");
	return dx11Sampler;
}

ComPtr<ID3D11UnorderedAccessView> DX11Texture2D::getUAV(unsigned int mipLevel)
{
	if (bind() == 0)
		return NULL;
	bool create = false;
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

	if (dx11UAV == NULL) {
		create = true;
	}
	else {
		dx11UAV->GetDesc(&uavDesc);
		create = uavDesc.Texture2D.MipSlice != mipLevel;
	}

	if (create) {
		/*if (dx11UAV != NULL)
			dx11UAV->Release();*/
		uavDesc.Format = desc.info.internalType == TIT_D32_F ? DXGI_FORMAT_R32_FLOAT : info.texture2DDesc.Format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = mipLevel;
		if (FAILED(dxContext.device->CreateUnorderedAccessView(dx11Texture2D.Get(), &uavDesc, &dx11UAV)))
			throw runtime_error("DX11Tetxture2D: CreateUnorderedAccessView failed");
	}
	return dx11UAV;
}

ComPtr<ID3D11DepthStencilView> DX11Texture2D::getDSV(unsigned int mipLevel)
{
	if (bind() == 0)
		return NULL;
	bool create = false;
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};

	if (dx11DSV == NULL) {
		create = true;
	}
	else {
		dx11DSV->GetDesc(&dsvDesc);
		create = dsvDesc.Texture2D.MipSlice != mipLevel;
	}

	if (create) {
		/*if (dx11DSV != NULL)
			dx11DSV->Release();*/
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		if (desc.info.sampleCount > 1) {
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		}
		else {
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Texture2D.MipSlice = mipLevel;
		}
		if (FAILED(dxContext.device->CreateDepthStencilView(dx11Texture2D.Get(), &dsvDesc, &dx11DSV)))
			throw runtime_error("DX11Tetxture2D: CreateDepthStencilView failed");
	}
	return dx11DSV;
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
	return internalType;
}

#endif // VENDOR_USE_DX11