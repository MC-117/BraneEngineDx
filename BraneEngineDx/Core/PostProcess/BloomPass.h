#pragma once
#ifndef _BLOOMPASS_H_
#define _BLOOMPASS_H_

#include "PostProcessPass.h"

class BloomPass : public PostProcessPass
{
public:
	Texture2D bloomMap = Texture2D(size.x, size.y, 4, true, { TW_Clamp, TW_Clamp, TF_Linear_Mip_Point, TF_Linear_Mip_Point, TIT_RGB10A2_UF });
	Texture2D screenMap = Texture2D(size.x, size.y, 4, true, { TW_Clamp, TW_Clamp, TF_Linear_Mip_Point, TF_Linear_Mip_Point, TIT_RGB10A2_UF });
	vector<RenderTarget*> bloomRenderTargets;
	vector<RenderTarget*> screenRenderTargets;
	unsigned int bloomLevel = 5;
	BloomPass(const string& name = "Bloom", Material* material = NULL);

	virtual void prepare();
	virtual void execute(IRenderContext& context);

	virtual bool mapMaterialParameter(RenderInfo& info);
	virtual void render(RenderInfo& info);
	virtual void resize(const Unit2Di& size);
protected:
	int sampleMapSlot = -1;
	int imageMapSlot = -1;

	void resizeBloomLevel(int levels);
};

#endif // !_BLOOMPASS_H_
