#pragma once

#include "Render.h"
#include "SceneCaptureCube.h"

class ReflectionCapture : public Render
{
public:
	TextureCube* reflectionCubeMap = NULL;
	Range<Vector3f> bound;
	int resolution = 128;
	Color tintColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
	float falloff = 5;
	float cutoff = 0;

	ReflectionCapture(const string& name = "ReflectionCapture");

	TextureCube* getReflectionCubeMap() const;
	void setReflectionCubeMap(TextureCube* reflectionCubeMap);
	Range<Vector3f> getWorldBound() const;
	Vector3f getWorldPosition() const;
	void setWorldPosition(const Vector3f& position);
	float getRadius() const;
	void setRadius(float radius);
	int getResolution() const;
	void setResolution(int resolution);
	void updateCapture();
	int getProbeIndex() const;

	virtual void render(RenderInfo& info);
protected:
	bool update = false;
	SceneCaptureCube sceneCaptureCube;
	int probeIndex = -1;
};