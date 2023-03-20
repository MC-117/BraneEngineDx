#pragma once

#include "../RenderCore/RenderInterface.h"

struct IGBufferGetter
{
	virtual Texture* getGBufferA() = 0;
	virtual Texture* getGBufferB() = 0;
	virtual Texture* getGBufferC() = 0;
	virtual Texture* getGBufferD() = 0;
	virtual Texture* getGBufferE() = 0;
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