#include "Light.h"

Color Light::ambient = { 255, 255, 255, 255 };

Light::Light(const string& name, Color color, float intensity, float attenuation, Shape boundShape) : Transform(name), boundShape(boundShape)
{
	this->color = color;
	this->intensity = intensity;
	this->attenuation = attenuation;
	renderOrder = 0;
}

void Light::setBaseColor(Color color)
{
	this->color = color;
}

Color Light::getBaseColor()
{
	return color;
}

Render * Light::getRender()
{
	return this;
}

unsigned int Light::getRenders(vector<Render*>& renders)
{
	renders.push_back(this);
	return 1;
}

void Light::preRender()
{
	Render::transformMat = Transform::transformMat;
}

void Light::render(RenderInfo & info)
{
}

IRendering::RenderType Light::getRenderType() const
{
	return IRendering::RenderType::Light;
}

Shape * Light::getShape()
{
	return &boundShape;
}
