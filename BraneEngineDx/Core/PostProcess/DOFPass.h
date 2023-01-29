#pragma once
#ifndef _DOFPASS_H_
#define _DOFPASS_H_

#include "PostProcessPass.h"

class DOFPass : public PostProcessPass
{
public:
	Texture2D dofMap = Texture2D(size.x, size.y, 4, false);
	RenderTarget dofRenderTarget = RenderTarget(size.x, size.y, 4);

	DOFPass(const string& name = "DOF", Material* material = NULL);

	virtual void prepare();
	virtual void execute(IRenderContext& context);

	virtual bool mapMaterialParameter(RenderInfo& info);
	virtual void render(RenderInfo& info);
	virtual void onGUI(EditorInfo& info);
	virtual void resize(const Unit2Di& size);
protected:
	int screenMapSlot = -1;
	int screenMapSamplerSlot = -1;
	int depthMapSlot = -1;
	int depthMapSamplerSlot = -1;

	bool autoFocus = false;
	float focusLengthOffset = 0;
};

#endif // !_DOFPASS_H_
