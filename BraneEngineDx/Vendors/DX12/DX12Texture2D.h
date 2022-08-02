#pragma once
#include "../../Core/ITexture.h"

#ifdef VENDOR_USE_DX11

#include "DX12Context.h"

struct DX12Texture2DInfo
{
	D3D12_RESOURCE_DESC texture2DDesc;
	D3D12_SAMPLER_DESC samplerDesc;
	DX12Texture2DInfo() = default;
	DX12Texture2DInfo(const Texture2DInfo& info);

	DX12Texture2DInfo& operator=(const Texture2DInfo& info);

	static D3D12_TEXTURE_ADDRESS_MODE toDX12WrapType(const TexWrapType& type);
	static D3D12_FILTER toDX12FilterType(const TexFilter& minType, const TexFilter& magType);
	static DXGI_FORMAT toDX12InternalType(const TexInternalType& type);
	static DXGI_FORMAT toDX12ColorType(const TexInternalType& type);
};

class DX12Texture2D : public ITexture2D
{
public:
	DX12Context& dxContext;
	DX12Texture2DInfo info;
	DX12Buffer* dx12Texture2D = NULL;
	DX12Buffer* dx12Texture2DUpload = NULL;

	DX12DescriptorHeap rtvHeap = DX12DescriptorHeap(1);
	DX12Descriptor dx12RTV;
	int rtvMipLevel = 0;

	DX12DescriptorHeap dsvHeap = DX12DescriptorHeap(1);
	DX12Descriptor dx12DSV;
	int dsvMipLevel = 0;

	DX12Texture2D(DX12Context& context, Texture2DDesc& desc);
	virtual ~DX12Texture2D();

	virtual bool isValid() const;
	virtual unsigned long long getTextureID();

	virtual void release();

	virtual unsigned int bind();
	virtual unsigned int bindBase(unsigned int index);
	virtual unsigned int resize(unsigned int width, unsigned int height);
	virtual DX12ResourceView getSRV();
	virtual DX12Descriptor getRTV(unsigned int mipLevel, bool isMS);
	virtual DX12ResourceView getSampler();
	virtual DX12ResourceView getUAV(unsigned int mipLevel);
	virtual DX12Descriptor getDSV(unsigned int mipLevel);
protected:
	static DXGI_FORMAT getColorType(unsigned int channel, DXGI_FORMAT internalType);
};

#endif // VENDOR_USE_DX12

