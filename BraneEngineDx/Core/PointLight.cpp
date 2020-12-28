#include "PointLight.h"
#include "Geometry.h"

unsigned int PointLight::pointLightCount = 0;

PointLight::PointLight(const string& name, Color color, float intensity, float attenuation, float radius) : Light::Light(name, color, intensity, attenuation, Sphere(radius))
{
	index = pointLightCount++;
}

void PointLight::setRadius(float radius)
{
	((Sphere*)&boundShape)->setRadius(radius);
}

void PointLight::preRender()
{
}

void PointLight::render(RenderInfo & info)
{
}
