#pragma once

#include "RenderTask.h"
#include "SceneRenderData.h"

struct RenderCommandExecutionInfo
{
	IRenderContext& context;
	Enum<ClearFlags> plusClearFlags = Clear_None;
	Enum<ClearFlags> minusClearFlags = Clear_None;
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
