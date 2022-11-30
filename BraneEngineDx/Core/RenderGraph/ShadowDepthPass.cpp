#include "ShadowDepthPass.h"
#include "../RenderCore/DirectShadowRenderPack.h"
#include "../RenderCore/MeshRenderPack.h"
#include "../Asset.h"

bool ShadowDepthPass::setRenderCommand(const IRenderCommand& cmd)
{
	if (material == NULL) {
		material = getAssetByPath<Material>("Engine/Shaders/Depth.mat");
	}
	if (material == NULL)
		return false;
	const MeshRenderCommand* meshRenderCommand = dynamic_cast<const MeshRenderCommand*>(&cmd);
	if (meshRenderCommand == NULL || !cmd.canCastShadow())
		return false;
	void* transformIndexHandle = NULL;
	for (int j = 0; j < meshRenderCommand->instanceIDCount; j++)
		transformIndexHandle = cmd.sceneData->setMeshPartTransform(
			meshRenderCommand->mesh, material, meshRenderCommand->instanceID + j);

	DirectShadowRenderCommand command;
	command.sceneData = cmd.sceneData;
	command.material = material;
	command.mesh = meshRenderCommand->mesh;
	command.directLightData = &cmd.sceneData->lightDataPack.directLightData;
	command.transformIndexHandle = transformIndexHandle;
	command.bindings = cmd.bindings;

	ShaderProgram* program = material->getShader()->getProgram(cmd.getShaderFeature());
	if (program == NULL)
		return false;

	MaterialRenderData* materialRenderData = dynamic_cast<MaterialRenderData*>(material->getRenderData());
	if (materialRenderData == NULL)
		return false;
	if (materialRenderData->usedFrame < (long long)Time::frames()) {
		materialRenderData->program = program;
		materialRenderData->create();
		materialRenderData->upload();
		materialRenderData->usedFrame = Time::frames();
	}

	for (auto binding : command.bindings) {
		if (binding->usedFrame < (long long)Time::frames()) {
			binding->create();
			binding->upload();
			binding->usedFrame = Time::frames();
		}
	}

	MeshData* meshData = command.mesh == NULL ? NULL : command.mesh->meshData;
	if (meshData)
		meshData->init();

	CameraRenderData& cameraRenderData = command.sceneData->lightDataPack.shadowCameraRenderData;

	if (cameraRenderData.usedFrame < (long long)Time::frames()) {
		cameraRenderData.create();
		cameraRenderData.upload();
		cameraRenderData.usedFrame = Time::frames();
	}

	RenderTask task;
	task.age = 0;
	task.sceneData = command.sceneData;
	task.shaderProgram = program;
	task.renderMode = command.getRenderMode();
	task.cameraData = &cameraRenderData;
	task.materialData = materialRenderData;
	task.meshData = meshData;
	task.extraData = command.bindings;

	commandList.addRenderTask(command, task);
}

void ShadowDepthPass::prepare()
{
	outputTextures.clear();
}

void ShadowDepthPass::execute(IRenderContext& context)
{
	RenderCommandExecutionInfo executionInfo(context);
	executionInfo.requireClearFrame = requireClearFrame;
	executionInfo.outputTextures = &outputTextures;
	executionInfo.timer = &timer;
	commandList.excuteCommand(executionInfo);
}

void ShadowDepthPass::reset()
{
	commandList.resetCommand();
}

void ShadowDepthPass::getOutputTextures(vector<pair<string, Texture*>>& textures)
{
	for (auto& tex : outputTextures)
		textures.push_back(tex);
}
