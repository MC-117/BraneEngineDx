#include "PointLight.h"
#include "Geometry.h"
#include "RenderCore/RenderCore.h"

SerializeInstance(PointLight);

unsigned int PointLight::pointLightCount = 1;

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

int PointLight::getLocalLightIndex() const
{
	return localLightIndex;
}

int PointLight::getProbeIndex() const
{
	return probeIndex;
}

void PointLight::preRender(PreRenderInfo& info)
{
}

void PointLight::render(RenderInfo & info)
{
	localLightIndex = info.sceneData->setLocalLight(this);
	probeIndex = info.sceneData->lightDataPack.getProbeIndexByLocalLightIndex(localLightIndex);
	if (VirtualShadowMapConfig::isEnable()) {
		virtualShadowMapLightEntry = info.sceneData->virtualShadowMapRenderData.newLocalLightShadow(
			localLightIndex, index);
		info.sceneData->lightDataPack.addVirtualShadowMapLocalShadow(*virtualShadowMapLightEntry);
	}
	else
		virtualShadowMapLightEntry = NULL;
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
