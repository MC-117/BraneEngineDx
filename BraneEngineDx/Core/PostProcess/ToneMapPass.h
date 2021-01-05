#pragma once
#ifndef _TONEMAPPASS_H_
#define _TONEMAPPASS_H_

#include "PostProcessPass.h"

class ToneMapPass : public PostProcessPass
{
public:
	ToneMapPass(const string& name = "ToneMap", Material* material = NULL);

	virtual bool mapMaterialParameter(RenderInfo& info);
	virtual void render(RenderInfo & info);
};

#endif // !_TONEMAPPASS_H_
