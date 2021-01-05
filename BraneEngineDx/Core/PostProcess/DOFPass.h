#pragma once
#ifndef _DOFPASS_H_
#define _DOFPASS_H_

#include "PostProcessPass.h"

class DOFPass : public PostProcessPass
{
public:
	Texture2D dofMap = Texture2D(size.x, size.y, 4, true);
	Texture2D screenMap = Texture2D(size.x, size.y, 4, true);
	RenderTarget dofRenderTarget = RenderTarget(size.x, size.y, 4);
	RenderTarget screenRenderTarget = RenderTarget(size.x, size.y, 4);

	DOFPass(const string& name = "DOF", Material* material = NULL);

	virtual bool mapMaterialParameter(RenderInfo& info);
	virtual void render(RenderInfo& info);
	virtual void resize(const Unit2Di& size);
};

#endif // !_DOFPASS_H_
