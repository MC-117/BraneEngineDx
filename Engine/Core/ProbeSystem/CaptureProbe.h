#pragma once

#include "ReflectionCaptureProbeRender.h"
#include "../Transform.h"

class ENGINE_API CaptureProbe : public Transform
{
public:
	Serialize(CaptureProbe, Transform);

	CaptureProbeRender* captureProbeRender = NULL;

	CaptureProbe(const string& name = "CaptureProbe");

	virtual CaptureProbeRender* createCaptureProbeRender(const Serialization& serialization);
	virtual CaptureProbeRender* getCaptureProbeRender();

	virtual void begin();

	virtual void prerender(SceneRenderData& sceneData);
	virtual Render* getRender();
	virtual unsigned int getRenders(vector<Render*>& renders);

	virtual void setHidden(bool value);
	virtual bool isHidden();

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
};