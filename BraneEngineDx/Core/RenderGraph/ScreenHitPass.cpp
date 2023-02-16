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
	MeshTransformIndex* transformIndex = cmd.sceneData->setMeshPartTransform(
		screenHitRenderCommand->mesh, material, screenHitRenderCommand->instanceID, screenHitRenderCommand->instanceIDCount);

	ScreenHitRenderCommand command;
	command.instanceID = screenHitRenderCommand->instanceID;
	command.instanceIDCount = screenHitRenderCommand->instanceIDCount;
	command.sceneData = cmd.sceneData;
	command.transformData = cmd.transformData;
	command.material = material;
	command.mesh = screenHitRenderCommand->mesh;
	command.transformIndex = transformIndex;
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

	RenderTask task;
	task.age = 0;
	task.sceneData = command.sceneData;
	task.transformData = command.transformData;
	task.shaderProgram = program;
	task.renderMode = command.getRenderMode();
	task.materialData = materialRenderData;
	task.meshData = meshData;
	task.extraData = command.bindings;

	for (CameraRenderData* cameraRenderData : command.sceneData->cameraRenderDatas) {
		if (cameraRenderData->usedFrame < (long long)Time::frames()) {
			cameraRenderData->create();
			cameraRenderData->upload();
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
