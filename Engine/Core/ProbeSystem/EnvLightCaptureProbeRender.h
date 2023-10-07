#pragma once

#include "CaptureProbeRender.h"
#include "../SceneCaptureCube.h"
#include "../RenderCore/EnvLightData.h"
#include "../Utility/SHMath.h"

class ENGINE_API EnvLightCaptureProbeRender : public CaptureProbeRender
{
public:
	Serialize(EnvLightCaptureProbeRender, CaptureProbeRender);

	TextureCube* lightCubeMap = NULL;
	Color tintColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
	float falloff = 5;
	float cutoff = 0;
	SHCoeff3RGB shCoeff3RGB;

	EnvLightCaptureProbeRender();

	virtual TextureCube* getLightCubeMap() const;
	virtual void setLightCubeMap(TextureCube* lightCubeMap);
	virtual void setWorldPosition(const Vector3f& position);
	virtual void updateCapture();
	virtual int getProbeIndex() const;
	virtual int getCaptureIndex() const;

	virtual void render(RenderInfo& info);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	bool update = false;
	SceneCaptureCube sceneCaptureCube;
	int captureIndex = -1;
	int lightDataIndex = -1;
};