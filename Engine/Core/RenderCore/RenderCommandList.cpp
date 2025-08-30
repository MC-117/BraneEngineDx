#include "RenderCommandList.h"
#include "../Console.h"
#include "../Engine.h"
#include "../Utility/RenderUtility.h"

RenderCommandExecutionInfo::RenderCommandExecutionInfo(IRenderContext& context) : context(context)
{
}

void ImmediateRenderCommandWorker::start()
{
}

void ImmediateRenderCommandWorker::stop()
{
}

void ImmediateRenderCommandWorker::setup(IRenderContext* context)
{
	if (context)
		renderContext = context;
	else
		renderContext = VendorManager::getInstance().getVendor().getDefaultRenderContext();
	taskParameter.renderContext = renderContext;
	taskContext = RenderTaskContext();
	taskParameter.taskContext = &taskContext;
}

int ImmediateRenderCommandWorker::getQueuedTaskCount() const
{
	return 0;
}

void ImmediateRenderCommandWorker::submitTask(RenderTask& task)
{
	if (taskContext.cameraData != task.cameraData) {
		if (taskContext.cameraData) {
			taskParameter.renderContext->endEvent();
		}
		char viewName[20];
		sprintf_s(viewName, "%s view %d", task.cameraData->isMainCamera ? "main" : "Sub", task.cameraData->cameraRenderID);
		taskParameter.renderContext->beginEvent(viewName);
	}
	task.execute(taskParameter);
}

void ImmediateRenderCommandWorker::submitContext()
{
}

void ImmediateRenderCommandWorker::waitForComplete()
{
	if (taskContext.cameraData) {
		taskParameter.renderContext->endEvent();
	}
	taskContext = RenderTaskContext();
}

ParallelRenderCommandWorker::~ParallelRenderCommandWorker()
{
	stop();
}

void ParallelRenderCommandWorker::start()
{
	stop();
	workThread = new std::thread(threadEntry, this);
	state = Running;
	workThread->detach();
}

void ParallelRenderCommandWorker::stop()
{
	if (state != Running)
		return;
	state = Pending;
	while (state == Pending)
		std::this_thread::yield();
	if (workThread) {
		delete workThread;
		workThread = NULL;
	}
}

void ParallelRenderCommandWorker::setup(IRenderContext* context)
{
	if (context)
		renderContext = context;
	else
		renderContext = VendorManager::getInstance().getVendor().newRenderContext();
	taskParameter.renderContext = renderContext;
	taskContext = RenderTaskContext();
	taskParameter.taskContext = &taskContext;
}

int ParallelRenderCommandWorker::getQueuedTaskCount() const
{
	return unfinishedTaskCount;
}

void ParallelRenderCommandWorker::submitTask(RenderTask& task)
{
	std::lock_guard<std::mutex> lock(workMutex);
	taskQueue.push(&task);
	unfinishedTaskCount++;
}

void ParallelRenderCommandWorker::executeTask()
{
	{
		std::lock_guard<std::mutex> lock(workMutex);
		while (!taskQueue.empty()) {
			workingQueue.push(taskQueue.front());
			taskQueue.pop();
		}
	}

	if (workingQueue.empty()) {
		std::this_thread::yield();
	}
	else {
		RenderTask* task = workingQueue.front();
		task->execute(taskParameter);
		workingQueue.pop();
		hasFinishedTask = true;
		unfinishedTaskCount--;
	}
}

void ParallelRenderCommandWorker::submitContext()
{
	if (hasFinishedTask) {
		renderContext->submit();
		hasFinishedTask = false;
	}
}

void ParallelRenderCommandWorker::waitForComplete()
{
	while (unfinishedTaskCount > 0)
		this_thread::yield();
	taskContext = RenderTaskContext();
}

void ParallelRenderCommandWorker::threadEntry(ParallelRenderCommandWorker* worker)
{
	while (worker->state == Running) {
		worker->executeTask();
	}
	std::lock_guard<std::mutex> lock(worker->workMutex);
	while (!worker->taskQueue.empty())
		worker->taskQueue.pop();
	while (!worker->workingQueue.empty())
		worker->workingQueue.pop();
	worker->unfinishedTaskCount = 0;
	worker->state = Stopped;
}

RenderCommandWorkerPool::RenderCommandWorkerPool(const Parameter& parameter) : parameter(parameter)
{
	if (parameter.parallelWorker) {
		workers.resize(parameter.workerCount);
		for (auto& worker : workers) {
			worker = new ParallelRenderCommandWorker();
		}
	}
	else {
		workers.resize(1);
		workers[0] = new ImmediateRenderCommandWorker();
	}
}

RenderCommandWorkerPool::~RenderCommandWorkerPool()
{
	for (auto& worker : workers) {
		delete worker;
	}
	workers.clear();
}

void RenderCommandWorkerPool::init()
{
	for (auto& worker : workers) {
		worker->setup(NULL);
		worker->start();
	}
}

void RenderCommandWorkerPool::submitContext()
{
	for (auto& worker : workers) {
		worker->submitContext();
	}
}

void RenderCommandWorkerPool::dispatchTask(RenderTask& task, int maxTaskNumPerWorker)
{
	for (auto& worker : workers) {
		if (worker->getQueuedTaskCount() < maxTaskNumPerWorker) {
			worker->submitTask(task);
			break;
		}
	}
}

void RenderCommandWorkerPool::waitForComplete()
{
	for (auto& worker : workers) {
		worker->waitForComplete();
	}
}

RenderCommandWorkerPool& RenderCommandWorkerPool::instance()
{
	static bool init = false;
	static int parallelRenderCommandExec = 0;
	static RenderCommandWorkerPool::Parameter parallelParameter = { true, 8, 0 };
	static RenderCommandWorkerPool parallelRenderCommandWorkerPool(parallelParameter);
	static RenderCommandWorkerPool::Parameter immediateParameter = { false, 1, 0 };
	static RenderCommandWorkerPool immediateRenderCommandWorkerPool(immediateParameter);
	if (!init) {
		Engine::engineConfig.configInfo.get("parallelRenderCommandExec", parallelRenderCommandExec);
		init = true;
	}
	return parallelRenderCommandExec ? parallelRenderCommandWorkerPool : immediateRenderCommandWorkerPool;
}

int RenderCommandWorkerPool::calMaxTaskNumPerWorker(int taskCount) const
{
	if (!parameter.parallelWorker ||
		taskCount < parameter.workerCount)
		return taskCount;
	int avgTaskPerWorker = ceilf(taskCount / (float)parameter.workerCount);
	return max(parameter.minTaskNumPerWorker, avgTaskPerWorker);
}

bool RenderCommandList::addRenderTask(const IRenderCommand& cmd, RenderTask& task)
{
	assert(task.materialVariant != NULL);
	assert(task.graphicsPipelineState != NULL);
	task.cameraData->applyValidViewCullingContext(task.batchDrawData);
	task.hashCode = RenderTask::Hasher()(task);

	RenderTask* pTask = NULL;
	auto taskIter = taskSet.find(&task);
	if (taskIter == taskSet.end()) {
		pTask = new RenderTask(task);
		//taskMap.insert(make_pair(task.hashCode, pTask));
		taskSet.insert(pTask);
	}
	else {
		pTask = *taskIter;
		task.renderPack = pTask->renderPack;
		*pTask = task;
	}

	if (pTask->age > 0) {
		if (pTask->renderPack != NULL) {
			pTask->renderPack->reset();
		}
	}

	assert(task.materialVariant != NULL);

	if (pTask->renderPack == NULL) {
		task.renderPack = pTask->renderPack = cmd.createRenderPack(*cmd.sceneData, *this);
	}

	if (!pTask->renderPack->setRenderCommand(cmd, *pTask))
		return false;

	pTask->age = 0;
	frameTaskCount++;
	return true;
}

bool RenderCommandList::setRenderCommand(const IRenderCommand& cmd, IRenderDataCollector& collector, ShaderFeature extraFeature)
{
	if (!cmd.isValid())
		return false;
	Enum<ShaderFeature> shaderFeature = cmd.getShaderFeature();
	shaderFeature |= extraFeature;
	IMaterial* materialVariant = cmd.materialRenderData->getVariant(shaderFeature);
	if (materialVariant == NULL) {
		Console::warn("Shader %s don't have mode %s",
			cmd.materialRenderData->getShaderName().c_str(),
			getShaderFeatureNames(shaderFeature).c_str());
		return false;
	}

	if (!materialVariant->init())
		return false;

	MaterialRenderData* materialRenderData = cmd.materialRenderData;
	if (materialRenderData == NULL)
		return false;

	MeshData* meshData = cmd.mesh == NULL ? NULL : cmd.mesh->meshData;
	if (meshData)
		meshData->init();

	RenderTask task;
	task.age = 0;
	task.sceneData = cmd.sceneData;
	task.batchDrawData = cmd.batchDrawData;
	task.shaderProgram = materialVariant->program;
	task.materialVariant = materialVariant;
	task.meshData = meshData;
	task.extraData = cmd.bindings;

	bool success = true;

	for (auto& cameraRenderData : cmd.sceneData->cameraRenderDatas) {
		task.renderMode = cmd.getRenderMode(Name::none, cameraRenderData);
		task.cameraData = cameraRenderData;
		task.surface = cameraRenderData->surface;
		success &= addRenderTask(cmd, task);
	}

	return success;
}

bool RenderCommandList::setRenderCommand(const IRenderCommand& cmd, IRenderDataCollector& collector, vector<ShaderFeature> extraFeatures)
{
	if (!cmd.isValid())
		return false;

	MeshData* meshData = cmd.mesh == NULL ? NULL : cmd.mesh->meshData;
	if (meshData)
		meshData->init();

	bool success = true;

	for (auto extraFeature : extraFeatures) {
		Enum<ShaderFeature> shaderFeature = cmd.getShaderFeature();
		shaderFeature |= extraFeature;
		IMaterial* materialVariant = cmd.materialRenderData->getVariant(shaderFeature);
		if (materialVariant == NULL) {
			Console::warn("Shader %s don't have mode %d", cmd.materialRenderData->getShaderName().c_str(), shaderFeature.enumValue);
			success &= false;
			continue;
		}

		if (!materialVariant->init()) {
			success &= false;
			continue;
		}

		MaterialRenderData* materialRenderData = cmd.materialRenderData;
		if (materialRenderData == NULL) {
			success &= false;
			continue;
		}

		RenderTask task;
		task.age = 0;
		task.sceneData = cmd.sceneData;
		task.batchDrawData = cmd.batchDrawData;
		task.shaderProgram = materialVariant->program;
		task.materialVariant = materialVariant;
		task.meshData = meshData;
		task.extraData = cmd.bindings;

		for (auto& cameraRenderData : cmd.sceneData->cameraRenderDatas) {
			task.renderMode = cmd.getRenderMode(Name::none, cameraRenderData);
			task.cameraData = cameraRenderData;
			task.surface = cameraRenderData->surface;
			success &= addRenderTask(cmd, task);
		}
	}

	return success;
}

/*
		 |   0   |       10       |   500   |      750      |  1000  |    2450    |     2500    |  5000   |
 Forward | Light | Depth Pre-Pass |         Geomtry         | Opaque | Alpha Mask | Transparent | Overlay |
Deferred | Light | Depth Pre-Pass | Geomtry | Alpha Geomtry | Pixel  |            |             |         |
*/

void RenderCommandList::excuteCommand(RenderCommandExecutionInfo& executionInfo)
{
	RenderCommandWorkerPool& workerPool = RenderCommandWorkerPool::instance();
	IRenderContext& context = executionInfo.context;

	int maxTaskNumPerWorker = workerPool.calMaxTaskNumPerWorker(frameTaskCount);

	RenderTarget* renderTarget = NULL;
	//Time setupTime, uploadInsTime, execTime;
	for (auto item : taskSet) {
		RenderTask& task = *item;
		task.age++;
		if (task.age > 1)
			continue;
		if (renderTarget != task.surface.renderTarget) {
			renderTarget = task.surface.renderTarget;

			task.surface.bindAndClear(context, executionInfo.plusClearFlags, executionInfo.minusClearFlags);

			if (executionInfo.outputTextures) {
				IRenderTarget* renderTargetVendor = renderTarget->getVendorRenderTarget();
				for (auto& tex : renderTargetVendor->desc.textureList) {
					executionInfo.outputTextures->push_back(make_pair(tex.name, tex.texture));
				}
				if (renderTargetVendor->desc.depthTexure) {
					executionInfo.outputTextures->push_back(make_pair("depthMap", renderTargetVendor->desc.depthTexure));
				}
			}
		}
		workerPool.dispatchTask(task, maxTaskNumPerWorker);
	}

	workerPool.waitForComplete();
	workerPool.submitContext();

	// context.setCullState(CullType::Cull_Back);
	context.setStencilRef(0);

	/*if (executionInfo.timer) {
		executionInfo.timer->setIntervalMode(true);
		executionInfo.timer->reset();
		executionInfo.timer->record("Setup", setupTime);
		executionInfo.timer->record("Instance", uploadInsTime);
		executionInfo.timer->record("Exec", execTime);
	}*/
}

void RenderCommandList::resetCommand()
{
	frameTaskCount = 0;
	// for (auto b = taskSet.begin(); b != taskSet.end(); b++) {
	// 	RenderTask* task = *b;
	// 	if (task->age > 1)
	// 	{
	// 		//taskMap.erase(task->hashCode);
	// 		if (task->renderPack) {
	// 			delete task->renderPack;
	// 			task->renderPack = NULL;
	// 		}
	// 		//b = taskSet.erase(b);
	// 		//delete task;
	// 	}
	// 	//else b++;
	// }
}
