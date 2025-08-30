#pragma once
#include "../../Core/IRenderTarget.h"

#ifdef VENDOR_USE_DX11

#ifndef _DX11RENDERTARGET_H_
#define _DX11RENDERTARGET_H_

#include "DX11.h"
#include "DX11Texture2D.h"
#include "DX11ShaderStage.h"

class DX11RenderTarget : public IRenderTarget
{
public:
	static unsigned int nextDxFrameID;
	static DX11RenderTarget* currentRenderTarget;
	unsigned int dxFrameID = 0;
	DX11Context& dxContext;
	vector<ComPtr<ID3D11RenderTargetView>> dx11RTVs;
	ComPtr<ID3D11DepthStencilView> dx11DSV = NULL;
	TextureDesc dx11DepthTexDesc;
	DX11Texture2D* dx11DepthTex = NULL;

	TextureDesc multisampleDepthTexDesc;
	DX11Texture2D* multisampleDepthTex = NULL;

	static string depthBlitName;
	static ShaderStageDesc depthBlitDesc;
	static DX11ShaderStage* depthBlitCSStage;
	static DX11ShaderProgram* depthBlitCSShader;
	static bool depthBlitInit;

	struct MSTex
	{
		TextureDesc desc;
		DX11Texture2D* tex = NULL;
	};

	vector<MSTex> multisampleTexs;

	DX11RenderTarget(DX11Context& context, RenderTargetDesc& desc);
	virtual ~DX11RenderTarget();

	DX11Texture2D* getValidDepthTexture() const;

	virtual ITexture2D* getInternalDepthTexture();
	virtual unsigned int bindFrame();
	virtual void clearBind();
	virtual void resize(unsigned int width, unsigned int height);
	virtual void SetMultisampleFrame();

	virtual void clearColor(const Color& color);
	virtual void clearColors(const vector<Color>& colors);
	virtual void clearDepth(float depth);
	virtual void clearStencil(unsigned char stencil);

	static void initDepthBlit(DX11Context& dxContext);
};

#endif // !_DX11RENDERTARGET_H_

#endif // VENDOR_USE_DX11