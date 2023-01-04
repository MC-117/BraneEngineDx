#pragma once
#include "../../Core/ITexture.h"

#ifdef VENDOR_USE_DX11

#ifndef _DX11TEXTURE2D_H_
#define _DX11TEXTURE2D_H_

#include "DX11.h"

struct DX11Texture2DInfo
{
	D3D11_TEXTURE2D_DESC texture2DDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	DX11Texture2DInfo() = default;
	DX11Texture2DInfo(const Texture2DInfo& info);

	DX11Texture2DInfo& operator=(const Texture2DInfo& info);

	static D3D11_TEXTURE_ADDRESS_MODE toDX11WrapType(const TexWrapType& type);
	static D3D11_FILTER toDX11FilterType(const TexFilter& minType, const TexFilter& magType);
	static DXGI_FORMAT toDX11InternalType(const TexInternalType& type);
	static DXGI_FORMAT toDX11ColorType(const TexInternalType& type);
};

class DX11Texture2D : public ITexture2D
{
public:
	DX11Context& dxContext;
	DX11Texture2DInfo info;
	ComPtr<ID3D11Texture2D> dx11Texture2D = NULL;

	DXViewSet<D3D11_SHADER_RESOURCE_VIEW_DESC, ID3D11ShaderResourceView> dx11SRVs;
	DXViewSet<D3D11_RENDER_TARGET_VIEW_DESC, ID3D11RenderTargetView> dx11RTVs;
	DXViewSet<D3D11_UNORDERED_ACCESS_VIEW_DESC, ID3D11UnorderedAccessView> dx11UAVs;
	DXViewSet<D3D11_DEPTH_STENCIL_VIEW_DESC, ID3D11DepthStencilView> dx11DSVs;

	ComPtr<ID3D11SamplerState> dx11Sampler = NULL;

	DX11Texture2D(DX11Context& context, Texture2DDesc& desc);
	virtual ~DX11Texture2D();

	virtual bool isValid() const;
	virtual unsigned long long getTextureID();

	virtual void release();

	virtual unsigned int bind();
	virtual unsigned int bindBase(unsigned int index);
	virtual unsigned int resize(unsigned int width, unsigned int height);
	virtual ComPtr<ID3D11ShaderResourceView> getSRV(const MipOption& mipOption = MipOption());
	virtual ComPtr<ID3D11RenderTargetView> getRTV(const RTOption& rtOption);
	virtual ComPtr<ID3D11SamplerState> getSampler();
	virtual ComPtr<ID3D11UnorderedAccessView> getUAV(const RWOption& rwOption);
	virtual ComPtr<ID3D11DepthStencilView> getDSV(unsigned int mipLevel);
protected:
	static DXGI_FORMAT getColorType(unsigned int channel, DXGI_FORMAT internalType);
	static D3D_SRV_DIMENSION getSrvDimension(TexDimension dimension, bool isMS);
	static D3D11_RTV_DIMENSION getRtvDimension(TexDimension dimension, bool isMS);
	static D3D11_UAV_DIMENSION getUavDimension(TexDimension dimension);
};

#endif // !_DX11TEXTURE2D_H_

#endif // VENDOR_USE_DX11