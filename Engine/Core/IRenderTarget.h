#pragma once
#ifndef _IRENDERTARGET_H_
#define _IRENDERTARGET_H_

#include "Texture.h"

struct RTInfo
{
	unsigned int index;
	string name;
	unsigned int mipLevel;
	unsigned int arrayBase;
	unsigned int arrayCount;
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

class ENGINE_API IRenderTarget
{
public:
	RenderTargetDesc& desc;

	IRenderTarget(RenderTargetDesc& desc);
	virtual ~IRenderTarget();

	virtual void setDepthOnly(Texture* depthTex);
	virtual ITexture2D* getInternalDepthTexture();
	virtual unsigned int bindFrame();
	virtual void clearBind();
	virtual void resize(unsigned int width, unsigned int height);
	virtual void SetMultisampleFrame();

	virtual void clearColor(const Color& color);
	virtual void clearColors(const vector<Color>& colors);
	virtual void clearDepth(float depth);
	virtual void clearStencil(unsigned int stencil);
};

struct DeviceSurfaceDesc
{
	int width = 0, height = 0, multisampleLevel = 1;
	TexInternalType type = TIT_BGRA8_UF;
	void* windowHandle = NULL;
	bool inited = false;
};

class ENGINE_API IDeviceSurface
{
public:
	DeviceSurfaceDesc& desc;

	IDeviceSurface(DeviceSurfaceDesc& desc);
	virtual ~IDeviceSurface();

	virtual void bindSurface();
	virtual void resize(unsigned int width, unsigned int height, unsigned int multisampleLevel = 0);
	virtual void clearColor(const Color& color);
	virtual void swapBuffer(bool vsync, int maxFps);
	virtual void frameFence();
};

#endif // !_IRENDERTARGET_H_
