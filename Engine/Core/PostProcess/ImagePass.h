#pragma once

#include "PostProcessPass.h"

class ImagePass : public PostProcessPass
{
public:
	Texture2D screenMap = Texture2D(size.x, size.y, 4, false);
	RenderTarget screenRenderTarget = RenderTarget(size.x, size.y, 4);

	ImagePass(const Name& name = "Image", Material* material = NULL);

	virtual void prepare();
	virtual void execute(IRenderContext& context);

	virtual bool loadDefaultResource();
	virtual void render(RenderInfo& info);
	virtual void resize(const Unit2Di& size);
};
