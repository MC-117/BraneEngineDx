#pragma once
#include "PostProcessPass.h"

class VolumetricLightPass : public PostProcessPass
{
protected:
	Texture* directShadowMap = NULL;
	float screenScale = 0.5f;
	int temp1MapSlot = -1;
	int temp1MapSamplerSlot = -1;
	int temp2MapSlot = -1;
	int temp2MapSamplerSlot = -1;
public:
	Texture2D passAMap = Texture2D(size.x * screenScale, size.y * screenScale, 4, true, { TW_Clamp_Edge, TW_Clamp_Edge, TF_Linear, TF_Linear, TIT_RGB10A2_UF });
	Texture2D passBMap = Texture2D(size.x * screenScale, size.y * screenScale, 4, true, { TW_Clamp_Edge, TW_Clamp_Edge, TF_Linear, TF_Linear, TIT_RGB10A2_UF });

	RenderTarget passARenderTarget = RenderTarget(size.x * screenScale, size.y * screenScale, 4);
	RenderTarget passBRenderTarget = RenderTarget(size.x * screenScale, size.y * screenScale, 4);

	VolumetricLightPass(const string& name = "VolumetricLight", Material* material = NULL);

	virtual void prepare();
	virtual void execute(IRenderContext& context);

	virtual bool mapMaterialParameter(RenderInfo& info);
	virtual void render(RenderInfo& info);
	virtual void resize(const Unit2Di& size);
	virtual void onGUI(GUIRenderInfo& info);

	void setScreenScale(float scale);
	float getScreenScale();
};

