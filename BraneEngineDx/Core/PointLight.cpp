#include "PointLight.h"
#include "Geometry.h"

SerializeInstance(PointLight);

unsigned int PointLight::pointLightCount = 0;

PointLight::PointLight(const string& name, Color color, float intensity, float attenuation, float radius) : Light::Light(name, color, intensity, attenuation, Sphere(radius))
{
	index = pointLightCount++;
}

void PointLight::setRadius(float radius)
{
	((Sphere*)&boundShape)->setRadius(radius);
}

float PointLight::getRadius() const
{
	return boundShape.getRadius();
}

void PointLight::preRender()
{
}

void PointLight::render(RenderInfo & info)
{
}

Serializable* PointLight::instantiate(const SerializationInfo& from)
{
	return new PointLight(from.name);
}

bool PointLight::deserialize(const SerializationInfo& from)
{
	if (!Light::deserialize(from))
		return false;
	float radius;
	if (from.get("radius", radius))
		setRadius(radius);
	return true;
}

bool PointLight::serialize(SerializationInfo& to)
{
	if (!Light::serialize(to))
		return false;
	to.set("radius", getRadius());
	return true;
}
