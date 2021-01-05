#pragma once
#ifndef _BLOOMPASS_H_
#define _BLOOMPASS_H_

#include "PostProcessPass.h"

class BloomPass : public PostProcessPass
{
public:
	Texture2D bloomMap = Texture2D(size.x, size.y, 4, true, { TW_Clamp, TW_Clamp, TF_Linear_Mip_Point, TF_Linear_Mip_Point });
	Texture2D screenMap = Texture2D(size.x, size.y, 4, true, { TW_Clamp, TW_Clamp, TF_Linear_Mip_Point, TF_Linear_Mip_Point });
	RenderTarget bloomRenderTarget = RenderTarget(size.x, size.y, 4);
	RenderTarget screenRenderTarget = RenderTarget(size.x, size.y, 4);
	unsigned int bloomLevel = 5;
	BloomPass(const string& name = "Bloom", Material* material = NULL);

	virtual bool mapMaterialParameter(RenderInfo& info);
	virtual void render(RenderInfo& info);
	virtual void resize(const Unit2Di& size);
};

#endif // !_BLOOMPASS_H_
