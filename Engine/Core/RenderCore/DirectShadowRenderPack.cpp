#include "DirectShadowRenderPack.h"
#include "RenderCommandList.h"
#include "RenderCoreUtility.h"

bool DirectShadowRenderCommand::isValid() const
{
	return sceneData && materialRenderData && materialRenderData->isValid() && mesh != NULL && mesh->isValid();
}

Enum<ShaderFeature> DirectShadowRenderCommand::getShaderFeature() const
{
	Enum<ShaderFeature> shaderFeature;
	if (mesh->meshData->type == MT_Terrain) {
		shaderFeature |= Shader_Terrain;
	}
	else {
		if (mesh->meshData->type == MT_SkeletonMesh) {
			shaderFeature |= Shader_Skeleton;
			if (mesh->isMorph())
				shaderFeature |= Shader_Morph;
		}
	}
	return shaderFeature;
}

RenderMode DirectShadowRenderCommand::getRenderMode() const
{
	return RenderMode(materialRenderData->renderOrder, 0, 0);
}

bool DirectShadowRenderCommand::canCastShadow() const
{
	return false;
}

IRenderPack* DirectShadowRenderCommand::createRenderPack(SceneRenderData& sceneData, RenderCommandList& commandList) const
{
	return new DirectShadowRenderPack();
}

DirectShadowRenderPack::DirectShadowRenderPack()
{
}

bool DirectShadowRenderPack::setRenderCommand(const IRenderCommand& command)
{
	const DirectShadowRenderCommand* directShadowRenderCommand = dynamic_cast<const DirectShadowRenderCommand*>(&command);
	if (directShadowRenderCommand == NULL || directShadowRenderCommand->meshBatchDrawCall == NULL)
		return false;

	materialData = command.materialRenderData;
	if (materialData == NULL)
		return false;

	if (directShadowRenderCommand->meshBatchDrawCall
		&& directShadowRenderCommand->meshBatchDrawCall->getInstanceCount() > 0
		&& directShadowRenderCommand->meshBatchDrawCall->getDrawCommandCount() > 0) {
		meshBatchDrawCalls.insert(directShadowRenderCommand->meshBatchDrawCall);
	}

	return true;
}

void DirectShadowRenderPack::excute(IRenderContext& context, RenderTask& task, RenderTaskContext& taskContext)
{
	if (meshBatchDrawCalls.empty())
		return;

	if (taskContext.materialVariant != task.materialVariant) {
		taskContext.materialVariant = task.materialVariant;

		bindMaterialCullMode(context, task.materialVariant, false);
		bindMaterial(context, task.materialVariant);
	}

	for (auto& item : meshBatchDrawCalls)
	{
		if (item->reverseCullMode) {
			bindMaterialCullMode(context, task.materialVariant, true);
		}
		for (int passIndex = 0; passIndex < materialData->desc.passNum; passIndex++) {
			materialData->desc.currentPass = passIndex;
			context.setDrawInfo(passIndex, materialData->desc.passNum, materialData->desc.materialID);
			context.bindDrawInfo();
			unsigned int commandOffset = item->getDrawCommandOffset();
			unsigned int commandEnd = commandOffset + item->getDrawCommandCount();
			for (; commandOffset < commandEnd; commandOffset++) {
				context.drawMeshIndirect(taskContext.batchDrawData.batchDrawCommandArray->getCommandBuffer(), sizeof(DrawElementsIndirectCommand) * commandOffset);
			}
		}
		if (item->reverseCullMode) {
			bindMaterialCullMode(context, task.materialVariant, false);
		}
	}
}

void DirectShadowRenderPack::reset()
{
	materialData = NULL;
	meshBatchDrawCalls.clear();
}
