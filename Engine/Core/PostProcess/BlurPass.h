#pragma once

#include "PostProcessPass.h"

class BlurPass : public PostProcessPass
{
public:
	Texture2D blurXMap = Texture2D(size.x, size.y, 4, false, { TW_Clamp, TW_Clamp, TF_Linear, TF_Linear });
	Texture2D blurYMap = Texture2D(size.x, size.y, 4, false, { TW_Clamp, TW_Clamp, TF_Linear, TF_Linear });
	BlurPass(const Name& name = "Blur", Material* material = NULL);

	virtual bool loadDefaultResource();
	virtual void render(RenderInfo& info);
	virtual void resize(const Unit2Di& size);
};