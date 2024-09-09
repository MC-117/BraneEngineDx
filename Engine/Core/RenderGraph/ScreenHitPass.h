#pragma once

#include "MeshPass.h"

class ScreenHitPass : public MeshPass
{
public:
	Texture2D hitTexture = Texture2D(1280, 720, 4, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RGBA8_I });
	RenderTarget hitRenderTarget = RenderTarget(1280, 720, 4);

	virtual bool setRenderCommand(const IRenderCommand& cmd);

	virtual bool loadDefaultResource();
	virtual void execute(IRenderContext& context);
	virtual void reset();
protected:
	MaterialRenderData* materialRenderData = NULL;
	unordered_set<ScreenHitData*> hitDatas;
};