#pragma once
#ifndef _IRENDERTARGET_H_
#define _IRENDERTARGET_H_

#include "Texture.h"

struct RTInfo
{
	unsigned int index;
	string name;
	unsigned int mipLevel;
	Texture* texture;
};

struct RenderTargetDesc
{
	int width = 0, height = 0, channel = 0;
	bool depthOnly = false;
	bool withDepthStencil = false;
	int multisampleLevel = 0;
	bool inited = false;
	unsigned int frameID = 0;
	
	map<string, int> textures;
	vector<RTInfo> textureList;

	Texture* depthTexure = NULL;
};

class IRenderTarget
{
public:
	RenderTargetDesc& desc;
	static RenderTargetDesc* defaultRenderTargetDesc;
	static IRenderTarget* defaultRenderTarget;

	IRenderTarget(RenderTargetDesc& desc);
	virtual ~IRenderTarget();

	virtual bool isDefault() const;

	virtual void setDepthOnly(Texture* depthTex);
	virtual ITexture2D* getInternalDepthTexture();
	virtual unsigned int bindFrame();
	virtual void resize(unsigned int width, unsigned int height);
	virtual void SetMultisampleFrame();

	virtual void clearColor(const Color& color);
	virtual void clearColors(const vector<Color>& colors);
	virtual void clearDepth(float depth);
	virtual void clearStencil(unsigned int stencil);
};

#endif // !_IRENDERTARGET_H_
