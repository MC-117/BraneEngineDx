#pragma once

#include "LightRenderData.h"
#include "ParticleRenderPack.h"
#include "MeshRenderPack.h"
#include "ReflectionProbeRenderData.h"
#include "EnvLightData.h"
#include "DebugRenderData.h"
#include "VirtualShadowMap/VirtualShadowMapRenderData.h"

class Render;

class ENGINE_API SceneRenderData
{
public:
	vector<CameraRenderData*> cameraRenderDatas;
	
	MeshTransformRenderData meshTransformRenderData;
	MeshTransformRenderData staticMeshTransformRenderData;
	MeshBatchDrawCommandArray meshBatchDrawCommandArray;
	MeshBatchDrawCommandArray staticMeshBatchDrawCommandArray;
	
	ParticleRenderData particleDataPack;
	ProbePoolRenderData probePoolPack;
	LightRenderData lightDataPack;
	ReflectionProbeRenderData reflectionProbeDataPack;
	EnvLightProbeRenderData envLightProbeDataPack;
	EnvLightRenderData envLightDataPack;
	VirtualShadowMapRenderData virtualShadowMapRenderData;
	DebugRenderData debugRenderData;

	SceneRenderData();

	void setCamera(Render* cameraRender);
	void setMainLight(Render* lightRender);
	int setLocalLight(Render* lightRender);
	int setReflectionCapture(Render* captureRender);
	int setEnvLightCapture(Render* captureRender);
	int setEnvLightData(Render* captureRender);
	unsigned int setMeshTransform(const MeshTransformData& data);
	unsigned int setMeshTransform(const vector<MeshTransformData>& datas);
	MeshBatchDrawCall* getMeshPartTransform(const MeshBatchDrawKey& key);
	MeshBatchDrawCall* setMeshPartTransform(const MeshBatchDrawKey& key, unsigned int transformIndex, unsigned int transformCount = 1);

	unsigned int setStaticMeshTransform(const MeshTransformData& data);
	unsigned int setStaticMeshTransform(const vector<MeshTransformData>& datas);
	MeshBatchDrawCall* getStaticMeshPartTransform(const MeshBatchDrawKey& key);
	MeshBatchDrawCall* setStaticMeshPartTransform(const MeshBatchDrawKey& key, unsigned int transformIndex, unsigned int transformCount = 1);
	void cleanStaticMeshTransform(unsigned int base, unsigned int count);
	void cleanStaticMeshPartTransform(MeshPart* meshPart, Material* material);

	void setUpdateStatic();
	bool willUpdateStatic();

	MeshBatchDrawData getBatchDrawData(bool isStatic);
	ViewCulledMeshBatchDrawData getViewCulledBatchDrawData(Render* cameraRender, bool isStatic);

	void executeViewCulling(IRenderContext& context);

	bool frustumCulling(const BoundBox& bound, const Matrix4f& mat) const;

	virtual void create();
	virtual void reset();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
};