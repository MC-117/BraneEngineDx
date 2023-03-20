#pragma once

#include "LightRenderData.h"
#include "ParticleRenderPack.h"
#include "MeshRenderPack.h"
#include "ReflectionProbeRenderData.h"
#include "EnvLightData.h"
#include "VirtualShadowMap/VirtualShadowMapRenderData.h"

class Render;

class SceneRenderData
{
public:
	vector<CameraRenderData*> cameraRenderDatas;
	MeshTransformRenderData meshTransformDataPack;
	MeshTransformRenderData staticMeshTransformDataPack;
	ParticleRenderData particleDataPack;
	LightRenderData lightDataPack;
	ReflectionProbeRenderData reflectionProbeDataPack;
	EnvLightProbeRenderData envLightProbeDataPack;
	EnvLightRenderData envLightDataPack;
	VirtualShadowMapRenderData virtualShadowMapRenderData;

	SceneRenderData();

	void setCamera(Render* cameraRender);
	void setLight(Render* lightRender);
	int setReflectionCapture(Render* captureRender);
	int setEnvLightCapture(Render* captureRender);
	int setEnvLightData(Render* captureRender);
	unsigned int setMeshTransform(const MeshTransformData& data);
	unsigned int setMeshTransform(const vector<MeshTransformData>& datas);
	MeshTransformIndex* getMeshPartTransform(MeshPart* meshPart, Material* material);
	MeshTransformIndex* setMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex, unsigned int transformCount = 1);

	unsigned int setStaticMeshTransform(const MeshTransformData& data);
	unsigned int setStaticMeshTransform(const vector<MeshTransformData>& datas);
	MeshTransformIndex* getStaticMeshPartTransform(MeshPart* meshPart, Material* material);
	MeshTransformIndex* setStaticMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex, unsigned int transformCount = 1);
	void cleanStaticMeshTransform(unsigned int base, unsigned int count);
	void cleanStaticMeshPartTransform(MeshPart* meshPart, Material* material);

	void setUpdateStatic();
	bool willUpdateStatic();

	bool frustumCulling(const BoundBox& bound, const Matrix4f& mat) const;

	virtual void create();
	virtual void reset();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
};