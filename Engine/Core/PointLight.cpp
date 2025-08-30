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
	bool isVSMEnable = VirtualShadowMapConfig::isEnable();
	RENDER_THREAD_ENQUEUE_TASK(PointLightUpdate, ([this, isVSMEnable] (RenderThreadContext& context)
	{
		localLightIndex = context.sceneRenderData->setLocalLight(this);
		probeIndex = context.sceneRenderData->lightDataPack.getProbeIndexByLocalLightIndex(localLightIndex);
		if (isVSMEnable) {
			virtualShadowMapLightEntry = context.sceneRenderData->virtualShadowMapRenderData.newLocalLightShadow(
				localLightIndex, index);
			context.sceneRenderData->lightDataPack.addVirtualShadowMapLocalShadow(*virtualShadowMapLightEntry);
		}
		else
			virtualShadowMapLightEntry = NULL;
	}));
}

Serializable* PointLight::instantiate(const SerializationInfo& from)
{
	PointLight* L = new PointLight(from.name);
	ChildrenInstantiate(Object, from, L);
	return L;
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
