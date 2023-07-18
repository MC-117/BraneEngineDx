#pragma once
#include "RenderInterface.h"
#include "../GPUBuffer.h"
#include "../RenderTarget.h"
#include "CameraRenderData.h"

class Light;
class VirtualShadowMapClipmap;

struct MainLightData
{
	Matrix4f worldToLightView;
	Matrix4f viewOriginToLightView;
	Matrix4f viewToLightClip;
	Matrix4f worldToLightClip;
	Vector3f direction;
	float intensity = 0;
	Vector4f shadowBias;
	int vsmID;
	float pad[3];
	Vector3f color;
	unsigned int pointLightCount = 0;
};

struct LocalLightData
{
	Vector3f position;
	float intensity = 0;
	Vector3f color;
	float radius;
	int vsmID;
	float pad[3];
};

struct LightRenderData : public IRenderData
{
	MainLightData mainLightData;
	vector<VirtualShadowMapClipmap*> mainLightClipmaps;
	vector<LocalLightData> pointLightDatas;
	GPUBuffer mainLightBuffer = GPUBuffer(GB_Constant, GBF_Struct, sizeof(MainLightData));
	GPUBuffer localLightBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(LocalLightData));

	RenderTarget* shadowTarget = NULL;
	CameraRenderData shadowCameraRenderData;

	void setLight(Light* light);
	void addVirtualShadowMapClipmap(VirtualShadowMapClipmap& clipmap);
	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
	void clean();
};