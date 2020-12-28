#pragma once
#include "../../Core/IRenderTarget.h"

#ifdef VENDOR_USE_DX11

#ifndef _DX11RENDERTARGET_H_
#define _DX11RENDERTARGET_H_

#include "DX11.h"
#include "DX11Texture2D.h"

class DX11RenderTarget : public IRenderTarget
{
public:
	static unsigned int nextDxFrameID;
	static DX11RenderTarget* currentRenderTarget;
	unsigned int dxFrameID = 0;
	const DX11Context& dxContext;
	vector<ID3D11RenderTargetView*> dx11RTVs;
	ID3D11DepthStencilView* dx11DSV = NULL;
	Texture2DDesc dx11DepthTexDesc;
	DX11Texture2D* dx11DepthTex = NULL;

	DX11RenderTarget(const DX11Context& context, RenderTargetDesc& desc);
	virtual ~DX11RenderTarget();

	virtual ITexture2D* getInternalDepthTexture();
	virtual unsigned int bindFrame();
	virtual void resize(unsigned int width, unsigned int height);
	virtual void SetMultisampleFrame();
	virtual void clearColor(const Color& color);
	virtual void clearColors(const vector<Color>& colors);
	virtual void clearDepth(float depth);
	virtual void clearStencil(unsigned char stencil);
};

#endif // !_DX11RENDERTARGET_H_

#endif // VENDOR_USE_DX11