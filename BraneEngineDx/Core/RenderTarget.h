#pragma once
#ifndef _RENDERTARGET_H_
#define _RENDERTARGET_H_

#include "Material.h"

class RenderTarget
{
public:
	static RenderTarget defaultRenderTarget;


	RenderTarget();
	RenderTarget(int width, int height, int channel, bool withDepthStencil = false, int multisampleLevel = 0);
	RenderTarget(int width, int height, Texture& depthTexture);
	virtual ~RenderTarget();

	virtual bool isValid();
	bool isDefault();
	bool isDepthOnly();
	void init();
	virtual unsigned int bindFrame();
	virtual void clearBind();
	virtual void addTexture(const string& name, Texture& texture, unsigned int mipLevel = 0, unsigned int arrayBase = 0, unsigned int arrayCount = 1);
	virtual Texture* getTexture(const string& name);
	virtual Texture* getTexture(unsigned int index);
	virtual unsigned int getTextureCount();
	virtual bool setTextureMipLevel(const string& name, unsigned int mipLevel);
	virtual bool setTextureMipLevel(unsigned int index, unsigned int mipLevel);
	virtual void setMultisampleLevel(unsigned int level);
	virtual unsigned int getMultisampleLevel();
	virtual void setDepthTexture(Texture& depthTexture);
	virtual Texture2D* getDepthTexture();
	virtual Texture2D* getInternalDepthTexture();
	virtual void setTexture(Material& mat);
	virtual void SetMultisampleFrame();
	unsigned int getFBO();
	virtual void resize(unsigned int width, unsigned int height);
	virtual void clearColor(const Color& color);
	virtual void clearColors(const vector<Color>& colors);
	virtual void clearDepth(float depth);
	virtual void clearStencil(unsigned char stencil);

	IRenderTarget* getVendorRenderTarget();
protected:
	RenderTargetDesc desc;
	IRenderTarget* vendorRenderTarget = NULL;

	Texture2D* internalDepthTexure = NULL;

	static unsigned int currentFbo;
	void newVendorRenderTarget();
};

#endif // !_RENDERTARGET_H_
