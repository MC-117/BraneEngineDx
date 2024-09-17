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
	SurfaceData surface;
	RenderTarget* gBufferRT = NULL;
	CameraRenderData* cameraRenderData = NULL;
	IMaterial* materialVariant = NULL;
};

class DeferredLightingPass : public RenderPass
{
public:
	set<DeferredLightingTask*, DeferredLightingTask::ExecutionOrder> lightingTask;
	list<MaterialRenderData*> materialRenderDatas;

	virtual bool loadDefaultResource();

	virtual bool addTask(DeferredLightingTask& task);

	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();
protected:
	Timer timer;
	static ShaderProgram* blitProgram;
	static ShaderStage* blitFragmentShader;

	void blitSceneColor(IRenderContext& context, RenderTarget& target, Texture* gBufferA, Texture* gBufferB);
};