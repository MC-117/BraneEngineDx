#pragma once
#ifndef _TONEMAPPASS_H_
#define _TONEMAPPASS_H_

#include "PostProcessPass.h"

class ToneMapPass : public PostProcessPass
{
public:
	Texture2D screenMap = Texture2D(size.x, size.y, 4, false, { TW_Clamp_Edge, TW_Clamp_Edge, TF_Linear, TF_Linear, TIT_RGB10A2_UF });
	RenderTarget screenRenderTarget = RenderTarget(size.x, size.y, 4);

	ToneMapPass(const string& name = "ToneMap", Material* material = NULL);

	virtual void prepare();
	virtual void execute(IRenderContext& context);

	virtual bool loadDefaultResource();
	virtual void render(RenderInfo & info);
	virtual void resize(const Unit2Di& size);
};

#endif // !_TONEMAPPASS_H_
