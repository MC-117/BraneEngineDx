#pragma once

#include "../RenderCore/RenderInterface.h"
#include "../RenderCore/MaterialRenderData.h"
#include "../RenderCore/CameraRenderData.h"

struct DeferredLightingTask
{
	struct ExecutionOrder
	{
		bool operator()(const DeferredLightingTask& t0, const DeferredLightingTask& t1) const;
		bool operator()(const DeferredLightingTask* t0, const DeferredLightingTask* t1) const;
	};

	int age = 0;
	SceneRenderData* sceneData = NULL;
	ShaderProgram* program = NULL;
	RenderTarget* gBufferRT = NULL;
	CameraRenderData* cameraRenderData = NULL;
	Material* material = NULL;
	MaterialRenderData* materialRenderData = NULL;
};

class DeferredLightingPass : public RenderPass
{
public:
	set<DeferredLightingTask*, DeferredLightingTask::ExecutionOrder> lightingTask;
	list<MaterialRenderData*> materialRenderDatas;

	virtual bool addTask(DeferredLightingTask& task);

	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();
protected:
	Timer timer;
	static bool isInit;
	static ShaderProgram* blitProgram;
	static ShaderStage* blitFragmentShader;

	static void LoadDefaultShader();

	void blitSceneColor(IRenderContext& context, Texture* gBufferA, Texture* gBufferB);
};