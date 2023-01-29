#pragma once
#include "RenderInterface.h"
#include "../GPUBuffer.h"
#include "../RenderTarget.h"
#include "CameraRenderData.h"

class Light;

struct MainLightData
{
	Vector3f direction;
	float intensity = 0;
	Matrix4f lightSpaceMat;
	Vector4f shadowBias;
	Vector3f color;
	unsigned int pointLightCount = 0;
};

struct LocalLightData
{
	Vector3f position;
	float intensity = 0;
	Vector3f color;
	float radius;
};

struct LightRenderData : public IRenderData
{
	MainLightData mainLightData;
	vector<LocalLightData> pointLightDatas;
	GPUBuffer mainLightBuffer = GPUBuffer(GB_Constant, GBF_Struct, sizeof(MainLightData));
	GPUBuffer localLightBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(LocalLightData));

	RenderTarget* shadowTarget = NULL;
	CameraRenderData shadowCameraRenderData;

	void setLight(Light* light);
	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
	void clean();
};