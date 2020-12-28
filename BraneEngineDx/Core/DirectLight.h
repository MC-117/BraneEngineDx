#pragma once
#ifndef _DIRECTLIGHT_H_
#define _DIRECTLIGHT_H_

#include "Light.h"

class DirectLight : public Light
{
public:
	ShadowCamera shadowCamera;

	DirectLight(const string& name, Color color = { 255, 255, 255, 255 }, float intensity = 1);

	virtual Matrix4f getLightSpaceMatrix() const;
	virtual RenderTarget* getShadowRenderTarget() const;

	virtual void afterTick();

	virtual void preRender();
	virtual void render(RenderInfo & info);
protected:
	int directionIdx = -1;
};

#endif // !_DIRECTLIGHT_H_
