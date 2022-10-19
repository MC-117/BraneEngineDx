#include "DX12Texture2D.h"

DX12Texture2DInfo::DX12Texture2DInfo(const Texture2DInfo& info)
{
	texture2DDesc.Format = toDX12InternalType(info.internalType);
	texture2DDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	if (info.internalType == TIT_D32_F)
		texture2DDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	else if (info.sampleCount > 1)
		texture2DDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	else
		texture2DDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	texture2DDesc.DepthOrArraySize = 1;
	texture2DDesc.SampleDesc.Count = info.sampleCount == 0 ? 1 : info.sampleCount;
	texture2DDesc.SampleDesc.Quality = 0;
	texture2DDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	samplerDesc.Filter = toDX12FilterType(info.minFilterType, info.magFilterType);
	samplerDesc.AddressU = toDX12WrapType(info.wrapSType);
	samplerDesc.AddressV = toDX12WrapType(info.wrapTType);
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	memcpy(samplerDesc.BorderColor, &info.borderColor, sizeof(float) * 4);
}

DX12Texture2DInfo& DX12Texture2DInfo::operator=(const Texture2DInfo& info)
{
	texture2DDesc.Format = toDX12InternalType(info.internalType);
	texture2DDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	if (info.internalType == TIT_D32_F)
		texture2DDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	else if (info.sampleCount > 1)
		texture2DDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	else
		texture2DDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET |
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
	texture2DDesc.DepthOrArraySize = 1;
	texture2DDesc.SampleDesc.Count = info.sampleCount == 0 ? 1 : info.sampleCount;
	texture2DDesc.SampleDesc.Quality = 0;
	texture2DDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	samplerDesc.Filter = toDX12FilterType(info.minFilterType, info.magFilterType);
	samplerDesc.AddressU = toDX12WrapType(info.wrapSType);
	samplerDesc.AddressV = toDX12WrapType(info.wrapTType);
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	memcpy(samplerDesc.BorderColor, &info.borderColor, sizeof(float) * 4);
	return *this;
}

D3D12_TEXTURE_ADDRESS_MODE DX12Texture2DInfo::toDX12WrapType(const TexWrapType& type)
{
	switch (type)
	{
	case TW_Repeat:
		return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	case TW_Mirror:
		return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
	case TW_Clamp:
		return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	case TW_Clamp_Edge:
		return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	case TW_Border:
		return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	case TW_Mirror_Once:
		return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
	}
	return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
}

D3D12_FILTER DX12Texture2DInfo::toDX12FilterType(const TexFilter& minType, const TexFilter& magType)
{
	D3D12_FILTER_TYPE minFT = D3D12_FILTER_TYPE_POINT,
		magFT = D3D12_FILTER_TYPE_POINT,
		mipFT = D3D12_FILTER_TYPE_POINT;
	switch (minType)
	{
	case TF_Point:
		break;
	case TF_Linear:
		minFT = D3D12_FILTER_TYPE_LINEAR;
		break;
	case TF_Point_Mip_Point:
		break;
	case TF_Linear_Mip_Point:
		minFT = D3D12_FILTER_TYPE_LINEAR;
		break;
	case TF_Point_Mip_Linear:
		mipFT = D3D12_FILTER_TYPE_LINEAR;
		break;
	case TF_Linear_Mip_Linear:
		minFT = D3D12_FILTER_TYPE_LINEAR;
		mipFT = D3D12_FILTER_TYPE_LINEAR;
		break;
	}
	switch (magType)
	{
	case TF_Point:
		break;
	case TF_Linear:
		magFT = D3D12_FILTER_TYPE_LINEAR;
		break;
	case TF_Point_Mip_Point:
		break;
	case TF_Linear_Mip_Point:
		magFT = D3D12_FILTER_TYPE_LINEAR;
		break;
	case TF_Point_Mip_Linear:
		mipFT = D3D12_FILTER_TYPE_LINEAR;
		break;
	case TF_Linear_Mip_Linear:
		magFT = D3D12_FILTER_TYPE_LINEAR;
		mipFT = D3D12_FILTER_TYPE_LINEAR;
		break;
	}
	return D3D12_ENCODE_BASIC_FILTER(minFT, magFT, mipFT, 0);
}

DXGI_FORMAT DX12Texture2DInfo::toDX12InternalType(const TexInternalType& type)
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

DXGI_FORMAT DX12Texture2DInfo::toDX12ColorType(const TexInternalType& type)
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

DX12Texture2D::DX12Texture2D(DX12Context& context, Texture2DDesc& desc)
	: dxContext(context), ITexture2D(desc)
{
}

DX12Texture2D::~DX12Texture2D()
{
	release();
}

bool DX12Texture2D::isValid() const
{
	return false;
}

unsigned long long DX12Texture2D::getTextureID()
{
	return desc.textureHandle;
}

void DX12Texture2D::release()
{
	dx12RTV.reset();
	dx12DSV.reset();
	if (dx12Texture2D) {
		dx12Texture2D->release();
		dx12Texture2D = NULL;
	}
	desc.textureHandle = 0;
}

unsigned int DX12Texture2D::bind()
{
	if (desc.textureHandle && !desc.needUpdate)
		return desc.textureHandle;
	info = desc.info;
	info.texture2DDesc.Width = desc.width;
	info.texture2DDesc.Height = desc.height;
	/*if (desc.info.sampleCount > 1) {
		unsigned int q = 0;
		dxContext.device->CheckMultisampleQualityLevels(info.texture2DDesc.Format, desc.info.sampleCount, &q);
		if (q != 0)
			--q;
		info.texture2DDesc.SampleDesc.Quality = q;
	}*/
	if (info.texture2DDesc.Format == DXGI_FORMAT_UNKNOWN)
		info.texture2DDesc.Format = getColorType(desc.channel, DXGI_FORMAT_UNKNOWN);
	if (desc.autoGenMip) {

		if (desc.info.sampleCount <= 1) {
			info.texture2DDesc.MipLevels = 1 + floor(log2(max(desc.width, desc.height)));
		}
		else {
			info.texture2DDesc.MipLevels = 1;
		}

		//info.texture2DDesc.Alignment = D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT;
		auto allocationInfo = dxContext.device->GetResourceAllocationInfo(1, 1, &info.texture2DDesc);
		//D3D11_SUBRESOURCE_DATA initData = { desc.data, desc.width * desc.channel * sizeof(unsigned char), 0 };

		/*if (info.texture2DDesc.Alignment != D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT) {
			info.texture2DDesc.Alignment = 0;
			allocationInfo = dxContext.device->GetResourceAllocationInfo(1, 1, &info.texture2DDesc);
		}*/

		if (desc.info.cpuAccessFlag == CAF_Read)
			dx12Texture2D = dxContext.readBackBufferPool.allocate(allocationInfo.SizeInBytes);
		else if (allocationInfo.Alignment == dxContext.textureBufferPool.getAlignment())
			dx12Texture2D = dxContext.textureBufferPool.allocate(allocationInfo.SizeInBytes);
		/*else if (allocationInfo.Alignment == D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT)
			dx12Texture2D = dxContext.textureBufferPool.allocate(allocationInfo.SizeInBytes);*/
		else if (allocationInfo.Alignment == dxContext.textureMSBufferPool.getAlignment())
			dx12Texture2D = dxContext.textureMSBufferPool.allocate(allocationInfo.SizeInBytes);
		else
			throw runtime_error("allocate alignment mismatch");

		desc.mipLevel = info.texture2DDesc.MipLevels;

		if (desc.data) {

			if (!dx12Texture2D->CreateResource(
				info.texture2DDesc,
				D3D12_RESOURCE_STATE_COPY_DEST))
				throw runtime_error("DX12Texture2D: CreateResource on textrue heap failed");

			D3D12_SUBRESOURCE_DATA initData;
			initData.pData = (const void*)desc.data;
			initData.RowPitch = desc.width * desc.channel * sizeof(unsigned char);
			initData.SlicePitch = initData.RowPitch * desc.height;
			dxContext.resourceUploader.uploadResource(dx12Texture2D, 0, 1, &initData,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			dxContext.resourceUploader.generateMips(dx12Texture2D, info.texture2DDesc.Format,
				desc.width, desc.height, desc.mipLevel);
		}
		else {
			if (!dx12Texture2D->CreateResource(
				info.texture2DDesc,
				desc.info.internalType == TexInternalType::TIT_D32_F ?
				D3D12_RESOURCE_STATE_DEPTH_WRITE :
				(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)))
				throw runtime_error("DX12Texture2D: CreateResource on textrue heap failed");
		}
	}
	else {
		info.texture2DDesc.MipLevels = desc.mipLevel;

		auto allocationInfo = dxContext.device->GetResourceAllocationInfo(1, 1, &info.texture2DDesc);

		if (desc.info.cpuAccessFlag == CAF_Read)
			dx12Texture2D = dxContext.readBackBufferPool.allocate(allocationInfo.SizeInBytes);
		else if (allocationInfo.Alignment == dxContext.textureBufferPool.getAlignment())
			dx12Texture2D = dxContext.textureBufferPool.allocate(allocationInfo.SizeInBytes);
		else if (allocationInfo.Alignment == dxContext.textureMSBufferPool.getAlignment())
			dx12Texture2D = dxContext.textureMSBufferPool.allocate(allocationInfo.SizeInBytes);
		else
			throw runtime_error("allocate alignment mismatch");

		if (desc.data) {
			if (!dx12Texture2D->CreateResource(
				info.texture2DDesc,
				D3D12_RESOURCE_STATE_COPY_DEST))
				throw runtime_error("DX12Texture2D: CreateResource on textrue heap failed");

			D3D12_SUBRESOURCE_DATA* initData = new D3D12_SUBRESOURCE_DATA[desc.mipLevel];
			int offset = 0;
			int _width = desc.width, _height = desc.height;
			for (int level = 0; level < desc.mipLevel; level++) {
				initData[level].pData = (const void*)(desc.data + offset);
				initData[level].RowPitch = _width * desc.channel * sizeof(unsigned char);
				initData[level].SlicePitch = 0;
				offset += desc.channel * _width * _height * sizeof(char);
				_width /= 2, _height /= 2;
			}
			dxContext.resourceUploader.uploadResource(dx12Texture2D, 0, desc.mipLevel, initData,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			if (initData != NULL)
				delete[] initData;
		}
		else {
			if (!dx12Texture2D->CreateResource(
				info.texture2DDesc,
				desc.info.internalType == TexInternalType::TIT_D32_F ?
				D3D12_RESOURCE_STATE_DEPTH_WRITE :
				(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)))
				throw runtime_error("DX12Texture2D: CreateResource on textrue heap failed");
		}
	}

	/*if (!dx12SRV.isActive()) {
		if (!srv_uavHeap.isInited()) {
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			desc.NodeMask = 1;
			srv_uavHeap.init(dxContext.device, desc);
		}
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = desc.info.internalType == TIT_Depth ? DXGI_FORMAT_R32_FLOAT : info.texture2DDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.mipLevel;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		dx12SRV = srv_uavHeap.updateSRV(dx12Texture2D->get(), srvDesc, 0);
	}*/

	desc.textureHandle = (unsigned long long)this;
	desc.needUpdate = false;
	return desc.textureHandle;
}

unsigned int DX12Texture2D::bindBase(unsigned int index)
{
	DX12ResourceView srv = getSRV();
	DX12ResourceView sampler = getSampler();

	if (!srv.isValid())
		return 0;

	dxContext.graphicContext.bindDefaultSRV(index, srv);

	if (sampler.isValid()) {
		dxContext.graphicContext.bindDefaultSRV(index, sampler);
	}

	return desc.textureHandle;
}

unsigned int DX12Texture2D::resize(unsigned int width, unsigned int height)
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

DX12ResourceView DX12Texture2D::getSRV()
{
	if (bind() == 0)
		return DX12ResourceView();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = desc.info.internalType == TIT_D32_F ? DXGI_FORMAT_R32_FLOAT : info.texture2DDesc.Format;
	if (desc.info.sampleCount > 1) {
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
	}
	else {
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = info.texture2DDesc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
	}
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	return DX12ResourceView(*dx12Texture2D, srvDesc);
}

DX12Descriptor DX12Texture2D::getRTV(unsigned int mipLevel, bool isMS)
{
	if (bind() == 0)
		return DX12Descriptor();
	bool create = false;
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;

	if (dx12RTV.isActive()) {
		create = rtvMipLevel != mipLevel;
	}
	else {
		create = true;
	}

	if (create) {
		if (!rtvHeap.isInited()) {
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			desc.NodeMask = 1;
			rtvHeap.init(dxContext.device, desc);
		}
		rtvMipLevel = mipLevel;
		ZeroMemory(&rtvDesc, sizeof(D3D12_RENDER_TARGET_VIEW_DESC));
		rtvDesc.Format = info.texture2DDesc.Format;
		rtvDesc.Texture2D.MipSlice = mipLevel;
		rtvDesc.ViewDimension = isMS ? D3D12_RTV_DIMENSION_TEXTURE2DMS : D3D12_RTV_DIMENSION_TEXTURE2D;
		dx12RTV = rtvHeap.updateRTV(*dx12Texture2D, rtvDesc, 0);
	}
	return dx12RTV;
}

DX12ResourceView DX12Texture2D::getSampler()
{
	if (bind() == 0)
		return DX12ResourceView();
	return DX12ResourceView(info.samplerDesc);
}

DX12ResourceView DX12Texture2D::getUAV(unsigned int mipLevel)
{
	if (bind() == 0)
		return DX12ResourceView();
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = desc.info.internalType == TIT_D32_F ? DXGI_FORMAT_R32_FLOAT : info.texture2DDesc.Format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = mipLevel;

	return DX12ResourceView(*dx12Texture2D, uavDesc);
}

DX12Descriptor DX12Texture2D::getDSV(unsigned int mipLevel)
{
	if (bind() == 0)
		return DX12Descriptor();
	bool create = false;
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};

	if (dx12DSV.isActive()) {
		create = dsvMipLevel != mipLevel;
	}
	else {
		create = true;
	}
	if (create) {
		if (!dsvHeap.isInited()) {
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			desc.NodeMask = 1;
			dsvHeap.init(dxContext.device, desc);
		}
		dsvMipLevel = mipLevel;
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		if (desc.info.sampleCount > 1) {
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
		}
		else {
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Texture2D.MipSlice = mipLevel;
		}
		dx12DSV = dsvHeap.updateDSV(*dx12Texture2D, dsvDesc, 0);
	}
	return dx12DSV;
}

DXGI_FORMAT DX12Texture2D::getColorType(unsigned int channel, DXGI_FORMAT internalType)
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
