#pragma once
#include "PostProcessPass.h"

class VolumetricLightPass : public PostProcessPass
{
protected:
	float screenScale = 0.5f;
	Texture** temp1Map = NULL;
	Texture** temp2Map = NULL;
public:
	Texture2D passAMap = Texture2D(size.x * screenScale, size.y * screenScale, 4, true, { TW_Clamp_Edge, TW_Clamp_Edge, TF_Linear, TF_Linear });
	Texture2D passBMap = Texture2D(size.x * screenScale, size.y * screenScale, 4, true, { TW_Clamp_Edge, TW_Clamp_Edge, TF_Linear, TF_Linear });

	RenderTarget passARenderTarget = RenderTarget(size.x * screenScale, size.y * screenScale, 4);
	RenderTarget passBRenderTarget = RenderTarget(size.x * screenScale, size.y * screenScale, 4);

	VolumetricLightPass(const string& name = "VolumetricLight", Material* material = NULL);

	virtual bool mapMaterialParameter(RenderInfo& info);
	virtual void render(RenderInfo& info);
	virtual void resize(const Unit2Di& size);
	virtual void onGUI(GUIRenderInfo& info);

	void setScreenScale(float scale);
	float getScreenScale();
};

