#include "ScreenHitPass.h"
#include "../Asset.h"

bool ScreenHitPass::setRenderCommand(const IRenderCommand& cmd)
{
	if (material == NULL) {
		material = getAssetByPath<Material>("Engine/Shaders/Editor/ScreenHit.mat");
	}
	if (material == NULL)
		return false;
	const ScreenHitRenderCommand* screenHitRenderCommand = dynamic_cast<const ScreenHitRenderCommand*>(&cmd);
	if (screenHitRenderCommand == NULL)
		return false;
	IMeshBatchDrawCommandArray* drawCommandArray = dynamic_cast<IMeshBatchDrawCommandArray*>(screenHitRenderCommand->batchDrawData.batchDrawCommandArray);
	const MeshBatchDrawKey renderKey(screenHitRenderCommand->mesh, material, screenHitRenderCommand->reverseCullMode);
	MeshBatchDrawCall* batchDrawCall = drawCommandArray->setMeshBatchDrawCall(renderKey, screenHitRenderCommand->instanceID, screenHitRenderCommand->instanceIDCount);

	ScreenHitRenderCommand command;
	command.instanceID = screenHitRenderCommand->instanceID;
	command.instanceIDCount = screenHitRenderCommand->instanceIDCount;
	command.reverseCullMode = screenHitRenderCommand->reverseCullMode;
	command.sceneData = cmd.sceneData;
	command.batchDrawData = cmd.batchDrawData;
	command.material = material;
	command.mesh = screenHitRenderCommand->mesh;
	command.meshBatchDrawCall = batchDrawCall;
	command.bindings = cmd.bindings;

	ShaderProgram* program = material->getShader()->getProgram(cmd.getShaderFeature());
	if (program == NULL || !program->init())
		return false;

	MaterialRenderData* materialRenderData = dynamic_cast<MaterialRenderData*>(material->getRenderData());
	if (materialRenderData == NULL)
		return false;
	if (materialRenderData->usedFrame < (long long)Time::frames()) {
		materialRenderData->program = program;
		materialRenderData->create();
		renderGraph->getRenderDataCollector()->add(*materialRenderData);
		materialRenderData->usedFrame = Time::frames();
	}

	for (auto binding : command.bindings) {
		if (binding->usedFrame < (long long)Time::frames()) {
			binding->create();
			renderGraph->getRenderDataCollector()->add(*binding);
			binding->usedFrame = Time::frames();
		}
	}

	MeshData* meshData = command.mesh == NULL ? NULL : command.mesh->meshData;
	if (meshData)
		meshData->init();

	RenderTask task;
	task.age = 0;
	task.sceneData = command.sceneData;
	task.batchDrawData = command.batchDrawData;
	task.shaderProgram = program;
	task.renderMode = command.getRenderMode();
	task.materialData = materialRenderData;
	task.meshData = meshData;
	task.extraData = command.bindings;

	for (CameraRenderData* cameraRenderData : command.sceneData->cameraRenderDatas) {
		if (cameraRenderData->usedFrame < (long long)Time::frames()) {
			cameraRenderData->create();
			cameraRenderData->usedFrame = Time::frames();
		}

		if (cameraRenderData->hitData == NULL)
			continue;

		hitDatas.insert(cameraRenderData->hitData);

		task.cameraData = cameraRenderData;
		task.surface = cameraRenderData->surface;
		task.surface.clearFlags = Clear_Depth;
		task.surface.renderTarget = cameraRenderData->hitData->renderTarget;

		commandList.addRenderTask(command, task);
	}

	return true; 
}

void ScreenHitPass::execute(IRenderContext& context)
{
	MeshPass::execute(context);
	for (ScreenHitData* hitData : hitDatas)
		hitData->readBack(context);
}

void ScreenHitPass::reset()
{
	MeshPass::reset();
	hitDatas.clear();
}
