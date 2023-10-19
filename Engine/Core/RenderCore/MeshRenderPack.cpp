#include "MeshRenderPack.h"
#include "RenderCommandList.h"

MeshBatchDrawKey MeshRenderCommand::getMeshBatchDrawKey() const
{
	return MeshBatchDrawKey(mesh, material, reverseCullMode);
}

bool MeshRenderCommand::isValid() const
{
	return sceneData && batchDrawData.isValid() && material && !material->isNull() && mesh != NULL && mesh->isValid();
}

Enum<ShaderFeature> MeshRenderCommand::getShaderFeature() const
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

RenderMode MeshRenderCommand::getRenderMode() const
{
	return RenderMode(material->getRenderOrder(), 0, 0);
}

bool MeshRenderCommand::canCastShadow() const
{
	return material->canCastShadow && hasShadow;
}

IRenderPack* MeshRenderCommand::createRenderPack(SceneRenderData& sceneData, RenderCommandList& commandList) const
{
	return new MeshDataRenderPack(sceneData.lightDataPack);
}

MeshDataRenderPack::MeshDataRenderPack(LightRenderData& lightDataPack)
	: lightDataPack(lightDataPack)
{
}

bool MeshDataRenderPack::setRenderCommand(const IRenderCommand& command)
{
	const MeshRenderCommand* meshRenderCommand = dynamic_cast<const MeshRenderCommand*>(&command);
	if (meshRenderCommand == NULL)
		return false;

	materialData = dynamic_cast<MaterialRenderData*>(command.material->getRenderData());
	if (materialData == NULL)
		return false;

	if (meshRenderCommand->meshBatchDrawCall
		&& meshRenderCommand->meshBatchDrawCall->getInstanceCount() > 0
		&& meshRenderCommand->meshBatchDrawCall->getDrawCommandCount() > 0) {
		meshBatchDrawCalls.insert(meshRenderCommand->meshBatchDrawCall);
	}

	return true;
}

void MeshDataRenderPack::excute(IRenderContext& context, RenderTaskContext& taskContext)
{
	if (meshBatchDrawCalls.empty())
		return;

	static const ShaderPropertyName depthMapName = "depthMap";

	if (taskContext.materialData != materialData) {
		taskContext.materialData = materialData;

		materialData->bindCullMode(context, false);
		materialData->bind(context);
		if (lightDataPack.shadowTarget == NULL)
			context.bindTexture((ITexture*)Texture2D::whiteRGBADefaultTex.getVendorTexture(), depthMapName);
		else
			context.bindTexture((ITexture*)lightDataPack.shadowTarget->getDepthTexture()->getVendorTexture(), depthMapName);
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

void MeshDataRenderPack::reset()
{
	materialData = NULL;
	meshBatchDrawCalls.clear();
}
