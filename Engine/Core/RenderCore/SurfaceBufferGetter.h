#pragma once

#include "../RenderCore/RenderInterface.h"

struct IDebugBufferGetter
{
	virtual Texture* getDebugBuffer() = 0;
	virtual RenderTarget* getDebugRenderTarget() = 0;
};

struct IGBufferGetter
{
	virtual Texture* getGBufferA() = 0;
	virtual Texture* getGBufferB() = 0;
	virtual Texture* getGBufferC() = 0;
	virtual Texture* getGBufferD() = 0;
	virtual Texture* getGBufferE() = 0;
	virtual Texture* getDepthTexture() = 0;
};

struct IHiZBufferGetter
{
	virtual Texture* getHiZTexture() = 0;
};

struct IScreenSpaceReflectionBufferGetter
{
	virtual Texture* getHitDataTexture() = 0;
	virtual Texture* getHitColorTexture() = 0;
	virtual RenderTarget* getTraceRenderTarget() = 0;
	virtual RenderTarget* getResolveRenderTarget() = 0;
};

struct ISceneColorMipsGetter
{
	virtual Texture* getSceneColorMips() = 0;
};
