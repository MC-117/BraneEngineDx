#pragma once
#ifndef _SSAOPASS_H_
#define _SSAOPASS_H_

#include "PostProcessPass.h"

class SSAOPass : public PostProcessPass
{
protected:
	float screenScale = 1.0f;
	int depthMapSlot = -1;
	int depthMapSamplerSlot = -1;
	int ssaoMapSlot = -1;
	int ssaoMapSamplerSlot = -1;
	int screenMapSlot = -1;
	int screenMapSamplerSlot = -1;
public:
	Texture2D gtaoMap = Texture2D(size.x * screenScale, size.y * screenScale, 1, true, { TW_Clamp_Edge, TW_Clamp_Edge, TF_Linear, TF_Linear });
	Texture2D screenMap = Texture2D(size.x, size.y, 4, true, { TW_Clamp, TW_Clamp, TF_Linear_Mip_Point, TF_Linear_Mip_Point });

	RenderTarget gtaoRenderTarget = RenderTarget(size.x * screenScale, size.y * screenScale, 1);
	RenderTarget screenRenderTarget = RenderTarget(size.x, size.y, 4);

	SSAOPass(const string& name = "SSAO", Material* material = NULL);

	virtual void prepare();
	virtual void execute(IRenderContext& context);

	virtual bool mapMaterialParameter(RenderInfo& info);
	virtual void render(RenderInfo& info);
	virtual void resize(const Unit2Di& size);
	virtual void onGUI(GUIRenderInfo& info);

	void setScreenScale(float scale);
	float getScreenScale();
};

#endif // !_SSAOPASS_H_
