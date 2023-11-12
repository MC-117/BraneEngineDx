#pragma once
#ifndef _POINTLIGHT_H_
#define _POINTLIGHT_H_

#include "Light.h"

struct VirtualShadowMapLightEntry;

class ENGINE_API PointLight : public Light
{
public:
	Serialize(PointLight, Light);

	PointLight(const string& name, Color color = { 255, 255, 255, 255 }, float intensity = 1, float attenuation = 1, float radius = 10);

	void setRadius(float radius);
	float getRadius() const;

	int getLocalLightIndex() const;
	int getProbeIndex() const;

	virtual void preRender(PreRenderInfo& info);
	virtual void render(RenderInfo & info);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	unsigned int index = -1;
	int localLightIndex = -1;
	int probeIndex = -1;
	VirtualShadowMapLightEntry* virtualShadowMapLightEntry = NULL;
	static unsigned int pointLightCount;
};

#endif // !_POINTLIGHT_H_
