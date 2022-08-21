#pragma once
#include "RenderInterface.h"
#include "../GPUBuffer.h"
#include "../RenderTarget.h"

class Light;

struct DirectLightData
{
	Vector3f direction;
	float intensity = 0;
	Matrix4f lightSpaceMat;
	Vector3f color;
	unsigned int pointLightCount = 0;
};

struct PointLightData
{
	Vector3f position;
	float intensity = 0;
	Vector3f color;
	float radius;
};

struct LightRenderData : public IRenderData
{
	DirectLightData directLightData;
	vector<PointLightData> pointLightDatas;
	GPUBuffer directLightBuffer = GPUBuffer(GB_Constant, sizeof(DirectLightData));
	GPUBuffer pointLightBuffer = GPUBuffer(GB_Struct, sizeof(PointLightData));

	RenderTarget* shadowTarget = NULL;

	void setLight(Light* light);
	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
	void clean();
};