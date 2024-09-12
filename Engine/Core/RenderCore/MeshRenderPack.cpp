#include "MeshRenderPack.h"
#include "RenderCommandList.h"
#include "RenderCoreUtility.h"
#include "../Profile/RenderProfile.h"
#include "Core/Asset.h"

MeshBatchDrawKey MeshRenderCommand::getMeshBatchDrawKey() const
{
	return MeshBatchDrawKey(mesh, materialRenderData, reverseCullMode);
}

bool MeshRenderCommand::isValid() const
{
	return sceneData && batchDrawData.isValid() && materialRenderData && materialRenderData->isValid() && mesh != NULL && mesh->isValid();
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
	return RenderMode(materialRenderData->renderOrder, 0, 0);
}

uint8_t MeshRenderCommand::getStencilValue() const
{
	return stencilValue;
}

bool MeshRenderCommand::canCastShadow() const
{
	return materialRenderData->canCastShadow && hasShadow;
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

	materialData = command.materialRenderData;
	if (materialData == NULL)
		return false;

	if (meshRenderCommand->meshBatchDrawCall
		&& meshRenderCommand->meshBatchDrawCall->getInstanceCount() > 0
		&& meshRenderCommand->meshBatchDrawCall->getDrawCommandCount() > 0) {
		meshBatchDrawCalls.insert(meshRenderCommand->meshBatchDrawCall);
	}

	return true;
}

void MeshDataRenderPack::excute(IRenderContext& context, RenderTask& task, RenderTaskContext& taskContext)
{
	if (meshBatchDrawCalls.empty())
		return;

	static const ShaderPropertyName depthMapName = "depthMap";

	if (taskContext.materialVariant != task.materialVariant) {
		taskContext.materialVariant = task.materialVariant;

		bindMaterialCullMode(context, task.materialVariant, false);
		bindMaterial(context, task.materialVariant);
		if (lightDataPack.shadowTarget == NULL)
			context.bindTexture((ITexture*)Texture2D::whiteRGBADefaultTex.getVendorTexture(), depthMapName);
		else
			context.bindTexture((ITexture*)lightDataPack.shadowTarget->getDepthTexture()->getVendorTexture(), depthMapName);
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
				RENDER_DESC_SCOPE(DrawMesh, "Material(%s)", AssetInfo::getPath(materialData->material).c_str());
				context.drawMeshIndirect(taskContext.batchDrawData.batchDrawCommandArray->getCommandBuffer(), sizeof(DrawElementsIndirectCommand) * commandOffset);
			}
		}
		if (item->reverseCullMode) {
			bindMaterialCullMode(context, task.materialVariant, false);
		}
	}
}

void MeshDataRenderPack::reset()
{
	materialData = NULL;
	meshBatchDrawCalls.clear();
}
