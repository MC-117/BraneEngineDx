#pragma once
#include "../../Core/ITexture.h"

#ifdef VENDOR_USE_DX11

#ifndef _DX11TEXTURE2D_H_
#define _DX11TEXTURE2D_H_

#include "DX11.h"

struct DX11TextureInfo
{
	D3D11_SAMPLER_DESC samplerDesc;

	virtual void set(const TextureInfo& info);

	static D3D11_TEXTURE_ADDRESS_MODE toDX11WrapType(const TexWrapType& type);
	static D3D11_FILTER toDX11FilterType(const TexFilter& minType, const TexFilter& magType);
	static DXGI_FORMAT toDX11InternalType(const TexInternalType& type);
	static DXGI_FORMAT toDX11ColorType(const TexInternalType& type);
};

struct DX11Texture2DInfo : DX11TextureInfo
{
	D3D11_TEXTURE2D_DESC texture2DDesc;
	DX11Texture2DInfo() = default;
	DX11Texture2DInfo(const TextureInfo& info);

	DX11Texture2DInfo& operator=(const TextureInfo& info);

	virtual void set(const TextureInfo& info);
};

struct DX11Texture3DInfo : DX11TextureInfo
{
	D3D11_TEXTURE3D_DESC texture3DDesc;
	DX11Texture3DInfo() = default;
	DX11Texture3DInfo(const TextureInfo& info);

	DX11Texture3DInfo& operator=(const TextureInfo& info);

	virtual void set(const TextureInfo& info);
};

class DX11Texture
{
public:
	DX11Context& dxContext;
	TextureDesc& desc;

	DXViewSet<D3D11_SHADER_RESOURCE_VIEW_DESC, ID3D11ShaderResourceView> dx11SRVs;
	DXViewSet<D3D11_RENDER_TARGET_VIEW_DESC, ID3D11RenderTargetView> dx11RTVs;
	DXViewSet<D3D11_UNORDERED_ACCESS_VIEW_DESC, ID3D11UnorderedAccessView> dx11UAVs;
	DXViewSet<D3D11_DEPTH_STENCIL_VIEW_DESC, ID3D11DepthStencilView> dx11DSVs;

	ComPtr<ID3D11SamplerState> dx11Sampler = NULL;

	DX11Texture(DX11Context& context, TextureDesc& desc);
	virtual ~DX11Texture() = default;

	virtual bool isValid() const = 0;
	virtual unsigned long long getTextureID() = 0;

	virtual unsigned int bind() = 0;

	virtual ID3D11Resource* getDX11Resource() = 0;
	virtual DXGI_FORMAT getDXFormat() const = 0;
	virtual const D3D11_SAMPLER_DESC& getDxSamplerDesc() const = 0;

	virtual void release();
	virtual unsigned int bindBase(unsigned int index);
	
	virtual ComPtr<ID3D11ShaderResourceView> getSRV(const MipOption& mipOption = MipOption());
	virtual ComPtr<ID3D11RenderTargetView> getRTV(const RTOption& rtOption);
	virtual ComPtr<ID3D11SamplerState> getSampler();
	virtual ComPtr<ID3D11UnorderedAccessView> getUAV(const RWOption& rwOption);
	virtual ComPtr<ID3D11DepthStencilView> getDSV(unsigned int mipLevel, DepthStencilAccessFlag flag);
protected:
	static DXGI_FORMAT getColorType(unsigned int channel, DXGI_FORMAT internalType);
	static D3D_SRV_DIMENSION getSrvDimension(TexDimension dimension, bool isMS);
	static D3D11_RTV_DIMENSION getRtvDimension(TexDimension dimension, bool isMS);
	static D3D11_UAV_DIMENSION getUavDimension(TexDimension dimension);
};

class DX11Texture2D : public DX11Texture, public ITexture2D
{
public:
	DX11Texture2DInfo info;
	ComPtr<ID3D11Texture2D> dx11Texture2D = NULL;
	DX11Texture2D(DX11Context& context, TextureDesc& desc);
	virtual ~DX11Texture2D();

	virtual bool isValid() const;
	virtual unsigned long long getTextureID();

	void release();
	
	virtual unsigned int bind();
	virtual ID3D11Resource* getDX11Resource();
	virtual DXGI_FORMAT getDXFormat() const;
	virtual const D3D11_SAMPLER_DESC& getDxSamplerDesc() const;

	virtual unsigned int resize(unsigned int width, unsigned int height);
	virtual unsigned int resize(unsigned int width, unsigned int height, unsigned int depth);

	virtual TextureDesc& getDesc() { return desc; }
	virtual const TextureDesc& getDesc() const { return desc; }
};

class DX11Texture3D : public DX11Texture, public ITexture3D
{
public:
	DX11Texture3DInfo info;
	ComPtr<ID3D11Texture3D> dx11Texture3D = NULL;
	DX11Texture3D(DX11Context& context, TextureDesc& desc);
	virtual ~DX11Texture3D();

	virtual bool isValid() const;
	virtual unsigned long long getTextureID();

	void release();
	
	virtual unsigned int bind();
	virtual ID3D11Resource* getDX11Resource();
	virtual DXGI_FORMAT getDXFormat() const;
	virtual const D3D11_SAMPLER_DESC& getDxSamplerDesc() const;

	virtual unsigned int resize(unsigned int width, unsigned int height);
	virtual unsigned int resize(unsigned int width, unsigned int height, unsigned int depth);
	
	virtual TextureDesc& getDesc() { return desc; }
	virtual const TextureDesc& getDesc() const { return desc; }
};

#endif // !_DX11TEXTURE2D_H_

#endif // VENDOR_USE_DX11