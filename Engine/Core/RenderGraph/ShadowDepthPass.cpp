#include "ShadowDepthPass.h"
#include "../RenderCore/DirectShadowRenderPack.h"
#include "../RenderCore/MeshRenderPack.h"
#include "../Asset.h"

bool ShadowDepthPass::setRenderCommand(const IRenderCommand& cmd)
{
	if (defaultDepthMaterial == NULL) {
		defaultDepthMaterial = getAssetByPath<Material>("Engine/Shaders/Depth.mat");
	}
	if (defaultDepthMaterial == NULL)
		return false;
	const MeshRenderCommand* meshRenderCommand = dynamic_cast<const MeshRenderCommand*>(&cmd);
	if (meshRenderCommand == NULL || !cmd.canCastShadow())
		return false;

	ShaderMatchRule matchRule;
	matchRule.fragmentFlag = ShaderMatchFlag::Best;
	Material* material = cmd.material;
	Enum<ShaderFeature> shaderFeature = cmd.getShaderFeature();
	ShaderProgram* program = material->getShader()->getProgram((unsigned int)shaderFeature | ShaderFeature::Shader_Depth, matchRule);
	if (program == NULL || !program->init()) {
		material = defaultDepthMaterial;
		program = material->getShader()->getProgram(shaderFeature);
		if (program == NULL || !program->init())
			return false;
	}
	IMeshBatchDrawCommandArray* drawCommandArray = dynamic_cast<IMeshBatchDrawCommandArray*>(meshRenderCommand->batchDrawData.batchDrawCommandArray);
	const MeshBatchDrawKey renderKey(meshRenderCommand->mesh, material, meshRenderCommand->reverseCullMode);
	MeshBatchDrawCall* batchDrawCall = drawCommandArray->setMeshBatchDrawCall(renderKey, meshRenderCommand->instanceIDCount);

	DirectShadowRenderCommand command;
	command.instanceID = meshRenderCommand->instanceID;
	command.instanceIDCount = meshRenderCommand->instanceIDCount;
	command.reverseCullMode = renderKey.negativeScale;
	command.sceneData = cmd.sceneData;
	command.batchDrawData = cmd.batchDrawData;
	command.material = material;
	command.mesh = meshRenderCommand->mesh;
	command.mainLightData = &cmd.sceneData->lightDataPack.mainLightData;
	command.meshBatchDrawCall = batchDrawCall;
	command.bindings = cmd.bindings;

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

	CameraRenderData& cameraRenderData = command.sceneData->lightDataPack.shadowCameraRenderData;

	if (cameraRenderData.usedFrame < (long long)Time::frames()) {
		cameraRenderData.create();
		cameraRenderData.usedFrame = Time::frames();
	}

	RenderTask task;
	task.age = 0;
	task.sceneData = command.sceneData;
	task.batchDrawData = command.batchDrawData;
	task.shaderProgram = program;
	task.renderMode = command.getRenderMode();
	task.cameraData = &cameraRenderData;
	task.surface = cameraRenderData.surface;
	task.materialData = materialRenderData;
	task.meshData = meshData;
	task.extraData = command.bindings;

	return commandList.addRenderTask(command, task);
}
