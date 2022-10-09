#pragma once

#include "RenderTask.h"
#include "LightRenderData.h"
#include "ParticleRenderPack.h"
#include "MeshRenderPack.h"

class Camera;
class Render;

class SceneRenderData
{
public:
	MeshTransformRenderData meshTransformDataPack;
	ParticleRenderData particleDataPack;
	LightRenderData lightDataPack;

	void setLight(Render* lightRender);
	unsigned int setMeshTransform(const Matrix4f& transformMat);
	unsigned int setMeshTransform(const vector<Matrix4f>& transformMats);
	void* getMeshPartTransform(MeshPart* meshPart, Material* material);
	void* setMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex);
	void* setMeshPartTransform(MeshPart* meshPart, Material* material, void* transformIndex);

	unsigned int setStaticMeshTransform(const Matrix4f& transformMat);
	unsigned int setStaticMeshTransform(const vector<Matrix4f>& transformMats);
	void* getStaticMeshPartTransform(MeshPart* meshPart, Material* material);
	void* setStaticMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex);
	void* setStaticMeshPartTransform(MeshPart* meshPart, Material* material, void* transformIndex);
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

class RenderCommandList
{
public:
	unordered_map<size_t, RenderTask*> taskMap;
	set<RenderTask*, RenderTask::ExecutionOrder> taskSet;

	bool addRenderTask(const IRenderCommand& cmd, RenderTask& task);
	bool setRenderCommand(const IRenderCommand& cmd, ShaderFeature extraFeature = Shader_Default);
	bool setRenderCommand(const IRenderCommand& cmd, vector<ShaderFeature> extraFeatures);

	void prepareCommand();
	void excuteCommand();
	void resetCommand();
};
