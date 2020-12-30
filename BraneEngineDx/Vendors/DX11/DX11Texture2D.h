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
	ID3D11Texture2D* dx11Texture2D = NULL;
	ID3D11View* dx11Texture2DView = NULL;
	ID3D11SamplerState* dx11Sampler = NULL;

	DX11Texture2D(DX11Context& context, Texture2DDesc& desc);
	~DX11Texture2D();

	virtual bool isValid() const;
	virtual unsigned long long getTextureID();

	virtual void release();

	virtual unsigned int bind();
	virtual unsigned int resize(unsigned int width, unsigned int height);
	virtual ID3D11ShaderResourceView* getSRV();
	virtual ID3D11SamplerState* getSampler();
	virtual ID3D11UnorderedAccessView* getUAV(unsigned int mipLevel);
	virtual ID3D11DepthStencilView* getDSV(unsigned int mipLevel);

	virtual bool assign(unsigned int width, unsigned int height, unsigned channel, const Texture2DInfo& info, unsigned int texHandle, unsigned int bindType);
protected:
	static DXGI_FORMAT getColorType(unsigned int channel, DXGI_FORMAT internalType);
};

#endif // !_DX11TEXTURE2D_H_

#endif // VENDOR_USE_DX11