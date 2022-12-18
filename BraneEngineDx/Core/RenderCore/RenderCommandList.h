#pragma once

#include "RenderTask.h"
#include "LightRenderData.h"
#include "ParticleRenderPack.h"
#include "MeshRenderPack.h"
#include "ReflectionProbeRenderData.h"

class Camera;
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

	SceneRenderData();

	void setCamera(Render* cameraRender);
	void setLight(Render* lightRender);
	int setReflectionCapture(Render* captureRender);
	unsigned int setMeshTransform(const Matrix4f& transformMat);
	unsigned int setMeshTransform(const vector<Matrix4f>& transformMats);
	MeshTransformIndex* getMeshPartTransform(MeshPart* meshPart, Material* material);
	MeshTransformIndex* setMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex);
	MeshTransformIndex* setMeshPartTransform(MeshPart* meshPart, Material* material, void* transformIndex);

	unsigned int setStaticMeshTransform(const Matrix4f& transformMat);
	unsigned int setStaticMeshTransform(const vector<Matrix4f>& transformMats);
	MeshTransformIndex* getStaticMeshPartTransform(MeshPart* meshPart, Material* material);
	MeshTransformIndex* setStaticMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex);
	MeshTransformIndex* setStaticMeshPartTransform(MeshPart* meshPart, Material* material, void* transformIndex);
	void cleanStaticMeshTransform(unsigned int base, unsigned int count);
	void cleanStaticMeshPartTransform(MeshPart* meshPart, Material* material);

	void setUpdateStatic();
	bool willUpdateStatic();

	virtual void create();
	virtual void reset();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
};

struct RenderCommandExecutionInfo
{
	IRenderContext& context;
	bool requireClearFrame = false;
	Timer* timer = NULL;
	vector<pair<string, Texture*>>* outputTextures = NULL;
	RenderCommandExecutionInfo(IRenderContext& context);
};

class RenderCommandList
{
public:
	unordered_map<size_t, RenderTask*> taskMap;
	set<RenderTask*, RenderTask::ExecutionOrder> taskSet;

	bool addRenderTask(const IRenderCommand& cmd, RenderTask& task);
	bool setRenderCommand(const IRenderCommand& cmd, ShaderFeature extraFeature = Shader_Default);
	bool setRenderCommand(const IRenderCommand& cmd, vector<ShaderFeature> extraFeatures);

	void excuteCommand(RenderCommandExecutionInfo& executionInfo);
	void resetCommand();
};
