#pragma once
#ifndef _SSAOPASS_H_
#define _SSAOPASS_H_

#include "PostProcessPass.h"

class SSAOPass : public PostProcessPass
{
protected:
	float screenScale = 1.0f;
public:
	Texture2D gtaoMap = Texture2D(size.x * screenScale, size.y * screenScale, 1, false, { TW_Clamp_Edge, TW_Clamp_Edge, TF_Linear, TF_Linear });
	Texture2D screenMap = Texture2D(size.x, size.y, 4, false);

	RenderTarget gtaoRenderTarget = RenderTarget(size.x * screenScale, size.y * screenScale, 1);
	RenderTarget screenRenderTarget = RenderTarget(size.x, size.y, 4);

	SSAOPass(const string& name = "SSAO", Material* material = NULL);

	virtual void prepare();
	virtual void execute(IRenderContext& context);

	virtual bool loadDefaultResource();
	virtual void render(RenderInfo& info);
	virtual void resize(const Unit2Di& size);
	virtual void onGUI(EditorInfo& info);

	void setScreenScale(float scale);
	float getScreenScale();
};

#endif // !_SSAOPASS_H_
