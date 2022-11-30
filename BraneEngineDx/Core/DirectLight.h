#pragma once
#ifndef _DIRECTLIGHT_H_
#define _DIRECTLIGHT_H_

#include "Light.h"

struct DirectLightShadowData
{
	float left = 0;
	float right = 0;
	float bottom = 0;
	float top = 0;
	float shadowBiasDepthScale = 1;
	float shadowBiasNormalScale = 1;
	CameraData cameraData;
};

class DirectLight : public Light
{
public:
	Serialize(DirectLight, Light);

	Texture2D depthTex;
	RenderTarget depthRenderTarget;

	DirectLightShadowData shadowData;

	DirectLight(const string& name, Color color = { 255, 255, 255, 255 }, float intensity = 1);

	void resizeShadowMap(Unit2Di size);
	void scaleShadowResolution(float scalar);

	void setShadowBiasDepthScale(float scale);
	void setShadowBiasNormalScale(float scale);
	float getShadowBiasDepthScale() const;
	float getShadowBiasNormalScale() const;

	virtual Matrix4f getLightSpaceMatrix() const;
	virtual Vector4f getShadowBias() const;
	virtual RenderTarget* getShadowRenderTarget() const;

	virtual void afterTick();

	virtual void preRender();
	virtual void render(RenderInfo & info);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	int directionIdx = -1;
	Matrix4f lightSpaceMatrix = Matrix4f::Identity();
};

#endif // !_DIRECTLIGHT_H_
