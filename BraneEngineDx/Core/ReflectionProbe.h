#pragma once

#include "ReflectionCapture.h"
#include "Transform.h"

class ReflectionProbe : public Transform
{
public:
	Serialize(ReflectionProbe, Transform);

	ReflectionCapture capture;

	ReflectionProbe(const string& name = "ReflectionProbe");

	virtual void prerender(SceneRenderData& sceneData);
	virtual Render* getRender();
	virtual unsigned int getRenders(vector<Render*>& renders);

	virtual void setHidden(bool value);
	virtual bool isHidden();

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
};