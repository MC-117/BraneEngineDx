#include "RenderCommandList.h"
#include "../Console.h"
#include "../Utility/RenderUtility.h"

RenderCommandExecutionInfo::RenderCommandExecutionInfo(IRenderContext& context) : context(context)
{
}

bool RenderCommandList::addRenderTask(const IRenderCommand& cmd, RenderTask& task)
{
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

	if (pTask->renderPack == NULL) {
		task.renderPack = pTask->renderPack = cmd.createRenderPack(*cmd.sceneData, *this);
	}

	if (!pTask->renderPack->setRenderCommand(cmd))
		return false;

	pTask->age = 0;
	return true;
}

bool RenderCommandList::setRenderCommand(const IRenderCommand& cmd, ShaderFeature extraFeature)
{
	if (!cmd.isValid())
		return false;
	Enum<ShaderFeature> shaderFeature = cmd.getShaderFeature();
	shaderFeature |= extraFeature;
	ShaderProgram* shader = cmd.material->getShader()->getProgram(shaderFeature);
	if (shader == NULL) {
		Console::warn("Shader %s don't have mode %s",
			cmd.material->getShaderName().c_str(),
			getShaderFeatureNames(shaderFeature).c_str());
		return false;
	}

	if (!shader->init())
		return false;

	MaterialRenderData* materialRenderData = dynamic_cast<MaterialRenderData*>(cmd.material->getRenderData());
	if (materialRenderData == NULL)
		return false;
	if (materialRenderData->usedFrame < (long long)Time::frames()) {
		materialRenderData->program = shader;
		materialRenderData->create();
		materialRenderData->upload();
		materialRenderData->usedFrame = Time::frames();
	}

	for (auto binding : cmd.bindings) {
		if (binding->usedFrame < (long long)Time::frames()) {
			binding->create();
			binding->upload();
			binding->usedFrame = Time::frames();
		}
	}

	MeshData* meshData = cmd.mesh == NULL ? NULL : cmd.mesh->meshData;
	if (meshData)
		meshData->init();

	RenderTask task;
	task.age = 0;
	task.sceneData = cmd.sceneData;
	task.transformData = cmd.transformData;
	task.shaderProgram = shader;
	task.renderMode = cmd.getRenderMode();
	task.materialData = materialRenderData;
	task.meshData = meshData;
	task.extraData = cmd.bindings;

	bool success = true;

	for (auto& cameraRenderData : cmd.sceneData->cameraRenderDatas) {
		task.cameraData = cameraRenderData;
		task.surface = cameraRenderData->surface;
		success &= addRenderTask(cmd, task);
	}

	return success;
}

bool RenderCommandList::setRenderCommand(const IRenderCommand& cmd, vector<ShaderFeature> extraFeatures)
{
	if (!cmd.isValid())
		return false;

	for (auto binding : cmd.bindings) {
		if (binding->usedFrame < (long long)Time::frames()) {
			binding->create();
			binding->upload();
			binding->usedFrame = Time::frames();
		}
	}

	MeshData* meshData = cmd.mesh == NULL ? NULL : cmd.mesh->meshData;
	if (meshData)
		meshData->init();

	bool success = true;

	for (auto extraFeature : extraFeatures) {
		Enum<ShaderFeature> shaderFeature = cmd.getShaderFeature();
		shaderFeature |= extraFeature;
		ShaderProgram* shader = cmd.material->getShader()->getProgram(shaderFeature);
		if (shader == NULL) {
			Console::warn("Shader %s don't have mode %d", cmd.material->getShaderName().c_str(), shaderFeature.enumValue);
			success &= false;
			continue;
		}

		if (!shader->init()) {
			success &= false;
			continue;
		}

		MaterialRenderData* materialRenderData = dynamic_cast<MaterialRenderData*>(cmd.material->getRenderData());
		if (materialRenderData == NULL) {
			success &= false;
			continue;
		}
		if (materialRenderData->usedFrame < (long long)Time::frames()) {
			materialRenderData->program = shader;
			materialRenderData->create();
			materialRenderData->upload();
			materialRenderData->usedFrame = Time::frames();
		}

		RenderTask task;
		task.age = 0;
		task.sceneData = cmd.sceneData;
		task.transformData = cmd.transformData;
		task.shaderProgram = shader;
		task.renderMode = cmd.getRenderMode();
		task.materialData = materialRenderData;
		task.meshData = meshData;
		task.extraData = cmd.bindings;

		for (auto& cameraRenderData : cmd.sceneData->cameraRenderDatas) {
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
	IRenderContext& context = executionInfo.context;
	RenderTaskContext taskContext = { 0 };
	//Time setupTime, uploadInsTime, execTime;
	for (auto item : taskSet) {
		RenderTask& task = *item;
		task.age++;
		if (task.age > 1)
			continue;
		//Time t = Time::now();

		bool shaderSwitch = false;

		if (taskContext.renderTarget != task.surface.renderTarget) {
			taskContext.renderTarget = task.surface.renderTarget;

			task.surface.bind(context, executionInfo.plusClearFlags, executionInfo.minusClearFlags);
			IRenderTarget* renderTarget = task.surface.renderTarget->getVendorRenderTarget();
			if (executionInfo.outputTextures) {
				for (auto& tex : renderTarget->desc.textureList) {
					executionInfo.outputTextures->push_back(make_pair(tex.name, tex.texture));
				}
				if (renderTarget->desc.depthTexure) {
					executionInfo.outputTextures->push_back(make_pair("depthMap", renderTarget->desc.depthTexure));
				}
			}
		}

		if (taskContext.cameraData != task.cameraData) {
			taskContext.cameraData = task.cameraData;

			context.setViewport(0, 0, task.cameraData->data.viewSize.x(), task.cameraData->data.viewSize.y());
			task.cameraData->bind(context);
		}

		if (taskContext.shaderProgram != task.shaderProgram) {
			taskContext.shaderProgram = task.shaderProgram;
			context.bindShaderProgram(task.shaderProgram);

			shaderSwitch = true;
			task.sceneData->bind(context);
		}

		if (taskContext.cameraData != task.cameraData || shaderSwitch) {
			taskContext.cameraData = task.cameraData;

			context.setViewport(0, 0, task.cameraData->data.viewSize.x(), task.cameraData->data.viewSize.y());
			task.cameraData->bind(context);

		}

		if (taskContext.sceneData != task.sceneData || shaderSwitch) {
			taskContext.sceneData = task.sceneData;

			task.sceneData->bind(context);
		}

		if (taskContext.transformData != task.transformData || shaderSwitch) {
			taskContext.transformData = task.transformData;

			if (task.transformData)
				task.transformData->bind(context);
		}

		if (taskContext.renderMode != task.renderMode) {
			uint16_t stage = task.renderMode.getRenderStage();
			if (stage < RenderStage::RS_Opaque)
				context.setRenderPreState();
			else if (stage < RenderStage::RS_Aplha)
				context.setRenderOpaqueState();
			else if (stage < RenderStage::RS_Transparent)
				context.setRenderAlphaState();
			else if (stage < RenderStage::RS_Post)
				context.setRenderTransparentState();
			else {
				BlendMode blendMode = task.renderMode.getBlendMode();
				switch (blendMode)
				{
				case BM_Default:
					context.setRenderPostState();
					break;
				case BM_Additive:
					context.setRenderPostAddState();
					break;
				case BM_Multipy:
					context.setRenderPostMultiplyState();
					break;
				case BM_PremultiplyAlpha:
					context.setRenderPostPremultiplyAlphaState();
					break;
				case BM_Mask:
					context.setRenderPostMaskState();
					break;
				default:
					throw runtime_error("Invalid blend mode");
					break;
				}
			}
		}

		for (auto data : task.extraData) {
			data->bind(context);
		}
		//setupTime = setupTime + Time::now() - t;

		//t = Time::now();
		if (taskContext.meshData != task.meshData) {
			taskContext.meshData = task.meshData;
			if (task.meshData)
				context.bindMeshData(task.meshData);
		}

		task.renderPack->excute(context, taskContext);

		//execTime = execTime + Time::now() - t;
	}

	context.setCullState(CullType::Cull_Back);

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
	//for (auto b = taskSet.begin(); b != taskSet.end();) {
	//	RenderTask* task = *b;
	//	if (task->age > 2) {
	//		//taskMap.erase(task->hashCode);
	//		if (task->renderPack) {
	//			delete task->renderPack;
	//			task->renderPack = NULL;
	//		}
	//		b = taskSet.erase(b);
	//		delete task;
	//	}
	//	else b++;
	//}
}
