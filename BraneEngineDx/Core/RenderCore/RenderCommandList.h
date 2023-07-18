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

class IRenderCommandWorker
{
public:
	virtual void start() = 0;
	virtual void stop() = 0;

	virtual void setup(IRenderContext* context) = 0;

	virtual int getQueuedTaskCount() const = 0;

	virtual void submitTask(RenderTask& task) = 0;
	virtual void submitContext() = 0;

	virtual void waitForComplete() = 0;
};

class ImmediateRenderCommandWorker : public IRenderCommandWorker
{
public:
	IRenderContext* renderContext = NULL;
	RenderTaskContext taskContext = { 0 };
	RenderTaskParameter taskParameter = { 0 };

	virtual void start();
	virtual void stop();

	virtual void setup(IRenderContext* context);

	virtual int getQueuedTaskCount() const;

	virtual void submitTask(RenderTask& task);
	virtual void submitContext();

	virtual void waitForComplete();
};

class ParallelRenderCommandWorker : public IRenderCommandWorker
{
public:
	enum State {
		Stopped,
		Running,
		Pending
	} state = Stopped;

	queue<RenderTask*> taskQueue;
	queue<RenderTask*> workingQueue;
	IRenderContext* renderContext = NULL;
	RenderTaskContext taskContext = { 0 };
	RenderTaskParameter taskParameter = { 0 };
	thread* workThread = NULL;
	mutex workMutex;
	atomic<int> unfinishedTaskCount = 0;
	bool hasFinishedTask = false;

	ParallelRenderCommandWorker() = default;
	virtual ~ParallelRenderCommandWorker();

	virtual void start();
	virtual void stop();

	virtual void setup(IRenderContext* context);

	virtual int getQueuedTaskCount() const;

	virtual void submitTask(RenderTask& task);
	virtual void executeTask();
	virtual void submitContext();

	virtual void waitForComplete();

	static void threadEntry(ParallelRenderCommandWorker* worker);
};

class RenderCommandWorkerPool
{
public:
	struct Parameter
	{
		bool parallelWorker = false;
		int workerCount;
		int minTaskNumPerWorker;
	};

	Parameter parameter;

	vector<IRenderCommandWorker*> workers;

	RenderCommandWorkerPool(const Parameter& parameter);
	virtual ~RenderCommandWorkerPool();

	void init();

	int calMaxTaskNumPerWorker(int taskCount) const;

	void dispatchTask(RenderTask& task, int maxTaskNumPerWorker);

	void submitContext();

	void waitForComplete();

	static RenderCommandWorkerPool& instance();
};

class RenderCommandList
{
public:
	unordered_map<size_t, RenderTask*> taskMap;
	set<RenderTask*, RenderTask::ExecutionOrder> taskSet;
	int frameTaskCount = 0;

	bool addRenderTask(const IRenderCommand& cmd, RenderTask& task);
	bool setRenderCommand(const IRenderCommand& cmd, IRenderDataCollector& collector, ShaderFeature extraFeature = Shader_Default);
	bool setRenderCommand(const IRenderCommand& cmd, IRenderDataCollector& collector, vector<ShaderFeature> extraFeatures);

	void excuteCommand(RenderCommandExecutionInfo& executionInfo);
	void resetCommand();
};
