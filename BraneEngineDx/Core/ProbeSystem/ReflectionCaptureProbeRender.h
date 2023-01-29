#pragma once

#include "CaptureProbeRender.h"
#include "../SceneCaptureCube.h"

class ReflectionCaptureProbeRender : public CaptureProbeRender
{
public:
	Serialize(ReflectionCaptureProbeRender, CaptureProbeRender);

	TextureCube* reflectionCubeMap = NULL;
	Color tintColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
	float falloff = 5;
	float cutoff = 0;

	ReflectionCaptureProbeRender();

	virtual TextureCube* getProbeCubeMap() const;
	virtual void setProbeCubeMap(TextureCube* reflectionCubeMap);
	virtual void setWorldPosition(const Vector3f& position);
	virtual void updateCapture();
	virtual int getProbeIndex() const;

	virtual void render(RenderInfo& info);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	bool update = false;
	SceneCaptureCube sceneCaptureCube;
	int probeIndex = -1;
};