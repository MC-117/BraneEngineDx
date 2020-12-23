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
	virtual unsigned int bindFrame();
	virtual void addTexture(const string& name, Texture& texture, unsigned int mipLevel = 0);
	virtual Texture* getTexture(const string& name);
	virtual Texture* getTexture(unsigned int index);
	virtual unsigned int getTextureCount();
	virtual bool setTextureMipLevel(const string& name, unsigned int mipLevel);
	virtual bool setTextureMipLevel(unsigned int index, unsigned int mipLevel);
	virtual void setMultisampleLevel(unsigned int level);
	virtual unsigned int getMultisampleLevel();
	virtual void setDepthTexture(Texture& depthTexture);
	virtual Texture2D* getInternalDepthTexture();
	virtual void setTexture(Material& mat);
	virtual void SetMultisampleFrame();
	unsigned int getFBO();
	virtual void resize(unsigned int width, unsigned int height);

	virtual void blitFrom(const RenderTarget& renderTarget, GLbitfield bit = GL_COLOR_BUFFER_BIT);

protected:
	unsigned int multisampleIndex = 0, depthIndex = 0, multisampleDepthIndex = 0;
	unsigned int multisampleFbo = 0, fbo = 0;
	unsigned int rbo = 0;
	int width = 0, height = 0, channel = 0;
	bool depthOnly = false;
	bool withDepthStencil = false;
	int multisampleLevel = 0;
	bool inited = false;

	struct RTInfo
	{
		unsigned int index;
		string name;
		unsigned int mipLevel;
		Texture* texture;
	};

	map<string, int> textures;
	vector<RTInfo> textureList;

	Texture* depthTexure = NULL;
	Texture2D internalDepthTexure;
	Texture2D internalMultisampleDepthTexure;

	static unsigned int currentFbo;
};

#endif // !_RENDERTARGET_H_
