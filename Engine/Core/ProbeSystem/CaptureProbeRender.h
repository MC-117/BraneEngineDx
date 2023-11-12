#pragma once

#include "../Render.h"
#include "../TextureCube.h"

class ENGINE_API CaptureProbeRender : public Serializable, public Render
{
public:
	Serialize(CaptureProbeRender,);

	float localRadius = 0;
	BoundBox bound;
	int resolution = 128;

	CaptureProbeRender() = default;

	virtual TextureCube* getProbeCubeMap() const;
	virtual void setProbeCubeMap(TextureCube* probeCubeMap);
	virtual BoundBox getWorldBound() const;
	virtual Vector3f getWorldPosition() const;
	virtual float getWorldScale() const;
	virtual void setWorldPositionAndScale(const Vector3f& position, float scale);
	virtual float getRadius() const;
	virtual float getWorldRadius() const;
	virtual void setRadius(float radius);
	virtual void setWorldRadius(float radius);
	virtual int getResolution() const;
	virtual void setResolution(int resolution);
	virtual void updateCapture();
	virtual int getProbeIndex() const;

	virtual RenderType getRenderType() const;
	virtual void render(RenderInfo& info);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
};