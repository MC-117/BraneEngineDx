#pragma once

#include "ProbeGridRenderData.h"
#include "../RenderTarget.h"
#include "CameraRenderData.h"

class Light;
class VirtualShadowMapClipmap;
struct VirtualShadowMapLightEntry;

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

struct LightRenderData : public IRenderData
{
	ProbePoolRenderData& probePool;
	MainLightData mainLightData;
	vector<VirtualShadowMapClipmap*> mainLightClipmaps;
	vector<VirtualShadowMapLightEntry*> localLightShadows;
	vector<int> pointLightProbeIndices;
	GPUBuffer mainLightBuffer = GPUBuffer(GB_Constant, GBF_Struct, sizeof(MainLightData));

	RenderTarget* shadowTarget = NULL;
	CameraRenderData shadowCameraRenderData;

	LightRenderData(ProbePoolRenderData& probePool);

	void setMainLight(Light* light);
	int setLocalLight(Light* light);
	void addVirtualShadowMapClipmap(VirtualShadowMapClipmap& clipmap);
	void addVirtualShadowMapLocalShadow(VirtualShadowMapLightEntry& localShadow);

	int getLocalLightCount() const;
	int getProbeIndexByLocalLightIndex(int localLightIndex) const;
	LocalLightData& getLocalLightData(int localLightIndex);
	const LocalLightData& getLocalLightData(int localLightIndex) const;

	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
	void clean();
};