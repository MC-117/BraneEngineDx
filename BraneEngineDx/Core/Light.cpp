#include "Light.h"

SerializeInstance(Light);

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

Serializable* Light::instantiate(const SerializationInfo& from)
{
	return nullptr;
}

bool Light::deserialize(const SerializationInfo& from)
{
	if (!Transform::deserialize(from))
		return false;
	from.get("intensity", intensity);
	from.get("attenuation", attenuation);
	SColor scolor = color;
	if (from.get("color", scolor))
		color = scolor;
	return true;
}

bool Light::serialize(SerializationInfo& to)
{
	if (!Transform::serialize(to))
		return false;
	to.set("intensity", intensity);
	to.set("attenuation", attenuation);
	to.set("color", (SColor)color);
	return true;
}
