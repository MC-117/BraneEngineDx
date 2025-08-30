#include "ScreenHitPass.h"
#include "../Asset.h"
#include "../RenderCore/RenderCoreUtility.h"

bool ScreenHitPass::setRenderCommand(const IRenderCommand& cmd)
{
	const ScreenHitRenderCommand* screenHitRenderCommand = dynamic_cast<const ScreenHitRenderCommand*>(&cmd);
	if (screenHitRenderCommand == NULL)
		return false;
	IMeshBatchDrawCommandArray* drawCommandArray = dynamic_cast<IMeshBatchDrawCommandArray*>(screenHitRenderCommand->batchDrawData.batchDrawCommandArray);
	const MeshBatchDrawKey renderKey(screenHitRenderCommand->mesh, materialRenderData, screenHitRenderCommand->reverseCullMode);
	MeshBatchDrawCall* batchDrawCall = drawCommandArray->setMeshBatchDrawCall(renderKey, screenHitRenderCommand->instanceID, screenHitRenderCommand->instanceIDCount);

	ScreenHitRenderCommand command;
	command.instanceID = screenHitRenderCommand->instanceID;
	command.instanceIDCount = screenHitRenderCommand->instanceIDCount;
	command.reverseCullMode = screenHitRenderCommand->reverseCullMode;
	command.sceneData = cmd.sceneData;
	command.batchDrawData = cmd.batchDrawData;
	command.materialRenderData = materialRenderData;
	command.mesh = screenHitRenderCommand->mesh;
	command.meshBatchDrawCall = batchDrawCall;
	command.bindings = cmd.bindings;

	IMaterial* materialVariant = materialRenderData->getVariant(cmd.getShaderFeature());
	if (materialVariant == NULL || !materialVariant->init())
		return false;

	MeshData* meshData = command.mesh == NULL ? NULL : command.mesh->meshData;
	if (meshData)
		meshData->init();

	RenderTask task;
	task.age = 0;
	task.sceneData = command.sceneData;
	task.batchDrawData = command.batchDrawData;
	task.shaderProgram = materialVariant->program;
	task.materialVariant = materialVariant;
	task.meshData = meshData;
	task.extraData = command.bindings;

	for (CameraRenderData* cameraRenderData : command.sceneData->cameraRenderDatas) {
		if (cameraRenderData->hitData == NULL)
			continue;

		hitDatas.insert(cameraRenderData->hitData);

		task.renderMode = command.getRenderMode("SreenHit"_N, cameraRenderData);
		task.cameraData = cameraRenderData;
		task.surface = cameraRenderData->surface;
		task.surface.clearFlags = Clear_Depth;
		task.surface.renderTarget = cameraRenderData->hitData->renderTarget;

		GraphicsPipelineStateDesc desc;
		setupPSODescFromRenderTask(desc, task, command.getCullType());
		task.graphicsPipelineState = fetchPSOIfDescChangedThenInit(NULL, desc);

		commandList.addRenderTask(command, task);
	}

	return true; 
}

bool ScreenHitPass::loadDefaultResource()
{
	if (materialRenderData == NULL) {
		Material* material = getAssetByPath<Material>("Engine/Shaders/Editor/ScreenHit.mat");
		materialRenderData = material->getMaterialRenderData();
	}
	if (materialRenderData) {
		renderGraph->getRenderDataCollectorMainThread()->add(*materialRenderData);
		renderGraph->getRenderDataCollectorRenderThread()->add(*materialRenderData);
	}
	return materialRenderData;
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
