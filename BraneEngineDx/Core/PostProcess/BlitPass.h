#pragma once
#ifndef _BLITPASS_H_
#define _BLITPASS_H_

#include "PostProcessPass.h"

class BlitPass : public PostProcessPass
{
public:
	BlitPass(const string& name = "Blit", Material* material = NULL);

	virtual bool mapMaterialParameter(RenderInfo& info);
	virtual void render(RenderInfo& info);
};

#endif // !_BLITPASS_H_
