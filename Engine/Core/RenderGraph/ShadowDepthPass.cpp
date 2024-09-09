#include "ShadowDepthPass.h"
#include "../RenderCore/DirectShadowRenderPack.h"
#include "../RenderCore/MeshRenderPack.h"
#include "../Asset.h"

bool ShadowDepthPass::setRenderCommand(const IRenderCommand& cmd)
{
	const MeshRenderCommand* meshRenderCommand = dynamic_cast<const MeshRenderCommand*>(&cmd);
	if (meshRenderCommand == NULL || !cmd.canCastShadow())
		return false;

	ShaderMatchRule matchRule;
	matchRule.fragmentFlag = ShaderMatchFlag::Best;
	MaterialRenderData* materialRenderData = cmd.materialRenderData;
	
	Enum<ShaderFeature> shaderFeature = cmd.getShaderFeature();
	IMaterial* materialVariant = materialRenderData->getVariant((unsigned int)shaderFeature | ShaderFeature::Shader_Depth, matchRule);
	if (materialVariant == NULL || !materialVariant->init()) {
		materialRenderData = defaultDepthMaterialRenderData;
		materialVariant = materialRenderData->getVariant(shaderFeature);
		if (materialVariant == NULL || !materialVariant->init())
			return false;
	}
	IMeshBatchDrawCommandArray* drawCommandArray = dynamic_cast<IMeshBatchDrawCommandArray*>(meshRenderCommand->batchDrawData.batchDrawCommandArray);
	const MeshBatchDrawKey renderKey(meshRenderCommand->mesh, materialRenderData, meshRenderCommand->reverseCullMode);
	MeshBatchDrawCall* batchDrawCall = drawCommandArray->setMeshBatchDrawCall(renderKey, meshRenderCommand->instanceIDCount);

	DirectShadowRenderCommand command;
	command.instanceID = meshRenderCommand->instanceID;
	command.instanceIDCount = meshRenderCommand->instanceIDCount;
	command.reverseCullMode = renderKey.negativeScale;
	command.sceneData = cmd.sceneData;
	command.batchDrawData = cmd.batchDrawData;
	command.materialRenderData = materialRenderData;
	command.mesh = meshRenderCommand->mesh;
	command.mainLightData = &cmd.sceneData->lightDataPack.mainLightData;
	command.meshBatchDrawCall = batchDrawCall;
	command.bindings = cmd.bindings;

	MeshData* meshData = command.mesh == NULL ? NULL : command.mesh->meshData;
	if (meshData)
		meshData->init();

	CameraRenderData& cameraRenderData = command.sceneData->lightDataPack.shadowCameraRenderData;

	RenderTask task;
	task.age = 0;
	task.sceneData = command.sceneData;
	task.batchDrawData = command.batchDrawData;
	task.shaderProgram = materialVariant->program;
	task.materialVariant = materialVariant;
	task.renderMode = command.getRenderMode();
	task.cameraData = &cameraRenderData;
	task.surface = cameraRenderData.surface;
	task.meshData = meshData;
	task.extraData = command.bindings;

	return commandList.addRenderTask(command, task);
}

bool ShadowDepthPass::loadDefaultResource()
{
	if (defaultDepthMaterialRenderData == NULL) {
		Material* defaultDepthMaterial = getAssetByPath<Material>("Engine/Shaders/Depth.mat");
		defaultDepthMaterialRenderData = defaultDepthMaterial->getMaterialRenderData();
	}
	if (defaultDepthMaterialRenderData) {
		renderGraph->getRenderDataCollectorMainThread()->add(*defaultDepthMaterialRenderData);
		renderGraph->getRenderDataCollectorRenderThread()->add(*defaultDepthMaterialRenderData);
	}
	return defaultDepthMaterialRenderData;
}
