#include "DirectShadowRenderPack.h"
#include "RenderCommandList.h"

bool DirectShadowRenderCommand::isValid() const
{
	return sceneData && material && !material->isNull() && mesh != NULL && mesh->isValid();
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
	return RenderMode(material->getRenderOrder(), 0, 0);
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

	materialData = dynamic_cast<MaterialRenderData*>(command.material->getRenderData());
	if (materialData == NULL)
		return false;

	if (directShadowRenderCommand->meshBatchDrawCall
		&& directShadowRenderCommand->meshBatchDrawCall->getInstanceCount() > 0
		&& directShadowRenderCommand->meshBatchDrawCall->getDrawCommandCount() > 0) {
		meshBatchDrawCalls.insert(directShadowRenderCommand->meshBatchDrawCall);
	}

	return true;
}

void DirectShadowRenderPack::excute(IRenderContext& context, RenderTaskContext& taskContext)
{
	if (meshBatchDrawCalls.empty())
		return;

	if (taskContext.materialData != materialData) {
		taskContext.materialData = materialData;

		materialData->bindCullMode(context, false);
		materialData->bind(context);
	}

	for (auto& item : meshBatchDrawCalls)
	{
		if (item->reverseCullMode) {
			materialData->bindCullMode(context, true);
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
			materialData->bindCullMode(context, false);
		}
	}
}

void DirectShadowRenderPack::reset()
{
	materialData = NULL;
	meshBatchDrawCalls.clear();
}
