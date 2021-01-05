#pragma once
#ifndef _SSAOPASS_H_
#define _SSAOPASS_H_

#include "PostProcessPass.h"

class SSAOPass : public PostProcessPass
{
public:
	Texture2D ssaoMap = Texture2D(size.x, size.y, 4, true, { TW_Clamp_Edge, TW_Clamp_Edge, TF_Linear, TF_Linear });
	Texture2D screenMap = Texture2D(size.x, size.y, 4, true, { TW_Clamp_Edge, TW_Clamp_Edge, TF_Linear, TF_Linear });
	RenderTarget ssaoRenderTarget = RenderTarget(size.x, size.y, 4);
	RenderTarget screenRenderTarget = RenderTarget(size.x, size.y, 4);

	SSAOPass(const string& name = "SSAO", Material* material = NULL);

	virtual bool mapMaterialParameter(RenderInfo& info);
	virtual void render(RenderInfo& info);

protected:
	float* pScreenScale = NULL;
};

#endif // !_SSAOPASS_H_
