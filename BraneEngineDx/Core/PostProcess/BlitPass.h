#pragma once
#ifndef _BLITPASS_H_
#define _BLITPASS_H_

#include "PostProcessPass.h"

class BlitPass : public PostProcessPass
{
public:
	BlitPass(const string& name = "Blit", Material* material = NULL);

	virtual void prepare();
	virtual void execute(IRenderContext& context);

	virtual bool mapMaterialParameter(RenderInfo& info);
	virtual void render(RenderInfo& info);
protected:
	int screenMapSlot = -1;
};

#endif // !_BLITPASS_H_
