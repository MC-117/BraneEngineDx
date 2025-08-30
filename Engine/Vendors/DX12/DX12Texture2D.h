#pragma once
#include "../../Core/ITexture.h"

#ifdef VENDOR_USE_DX11

#include "DX12Context.h"

struct DX12Texture2DInfo
{
	D3D12_RESOURCE_DESC texture2DDesc;
	D3D12_SAMPLER_DESC samplerDesc;
	DX12Texture2DInfo() = default;
	DX12Texture2DInfo(const TextureInfo& info);

	DX12Texture2DInfo& operator=(const TextureInfo& info);

	static D3D12_TEXTURE_ADDRESS_MODE toDX12WrapType(const TexWrapType& type);
	static D3D12_FILTER toDX12FilterType(const TexFilter& minType, const TexFilter& magType);
	static DXGI_FORMAT toDX12InternalType(const TexInternalType& type);
	static DXGI_FORMAT toDX12ColorType(const TexInternalType& type);
};

class DX12Texture2D : public ITexture2D
{
public:
	DX12Context& dxContext;
	TextureDesc& desc;
	DX12Texture2DInfo info;
	DX12Buffer* dx12Texture2D = NULL;
	DX12Buffer* dx12Texture2DUpload = NULL;

	DX12DescriptorHeap rtvHeap = DX12DescriptorHeap(1);
	DX12Descriptor dx12RTV;
	int rtvMipLevel = 0;

	DX12DescriptorHeap dsvHeap = DX12DescriptorHeap(1);
	DX12Descriptor dx12DSV;
	int dsvMipLevel = 0;

	DX12Texture2D(DX12Context& context, TextureDesc& desc);
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

	virtual TextureDesc& getDesc() { return desc; }
	virtual const TextureDesc& getDesc() const { return desc; }
protected:
	static DXGI_FORMAT getColorType(unsigned int channel, DXGI_FORMAT internalType);
};

#endif // VENDOR_USE_DX12

