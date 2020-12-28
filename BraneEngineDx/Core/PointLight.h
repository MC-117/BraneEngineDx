#pragma once
#ifndef _POINTLIGHT_H_
#define _POINTLIGHT_H_

#include "Light.h"

class PointLight : public Light
{
public:
	PointLight(const string& name, Color color = { 255, 255, 255, 255 }, float intensity = 1, float attenuation = 1, float radius = 10);

	void setRadius(float radius);

	virtual void preRender();
	virtual void render(RenderInfo & info);
protected:
	unsigned int index = -1;
	static unsigned int pointLightCount;
};

#endif // !_POINTLIGHT_H_
