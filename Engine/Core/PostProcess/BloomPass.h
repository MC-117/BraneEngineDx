#pragma once
#ifndef _BLOOMPASS_H_
#define _BLOOMPASS_H_

#include "PostProcessPass.h"

class BloomPass : public PostProcessPass
{
public:
	Texture2D bloomSampleMap = Texture2D(size.x, size.y, 4, false, { TW_Clamp, TW_Clamp, TF_Linear_Mip_Point, TF_Linear_Mip_Point, TIT_RGBA16_FF });
	vector<RenderTarget*> bloomSampleRenderTargets;
	unsigned int bloomLevel = 6;
	BloomPass(const Name& name = "Bloom", Material* material = NULL);

	virtual void prepare();
	virtual void execute(IRenderContext& context);

	virtual bool loadDefaultResource();
	virtual void render(RenderInfo& info);
	virtual void resize(const Unit2Di& size);
protected:
	GraphicsPipelineState* additivePipelineState = NULL;
	void resizeBloomLevel(int levels);
};

#endif // !_BLOOMPASS_H_
