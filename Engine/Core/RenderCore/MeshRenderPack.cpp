#include "MeshRenderPack.h"
#include "RenderCommandList.h"
#include "RenderCoreUtility.h"
#include "../Profile/RenderProfile.h"
#include "../Asset.h"

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

CullType MeshRenderCommand::getCullType() const
{
	return getMaterialCullMode(materialRenderData, reverseCullMode);
}

RenderMode MeshRenderCommand::getRenderMode(const Name& passName, const CameraRenderData* cameraRenderData) const
{
	const int renderOrder = materialRenderData->renderOrder;
	RenderMode renderMode = RenderMode(renderOrder);
	renderMode.dsMode.stencilTest = materialRenderData->desc.enableStencilTest;

	bool isGeometry = passName == "Geometry"_N;
	bool isTranslucent = passName == "Translucent"_N;
	bool cameraForceStencilTest = cameraRenderData->forceStencilTest &&
		(passName == "PreDepth"_N || isGeometry || isTranslucent);

	if (isGeometry) {
		renderMode[3].setBlendMode(BM_Disable);
	}

	if (isTranslucent) {
		renderMode.dsMode.accessFlag = DSA_DepthReadOnly;
	}
	
	if (cameraForceStencilTest) {
		renderMode.dsMode.stencilTest = true;
		renderMode.dsMode.stencilComparion_front = cameraRenderData->stencilCompare;
		renderMode.dsMode.stencilComparion_back = cameraRenderData->stencilCompare;
	}
	else {
		renderMode.dsMode.stencilComparion_front = materialRenderData->desc.stencilCompare;
		renderMode.dsMode.stencilComparion_back = materialRenderData->desc.stencilCompare;
	}
	return renderMode;
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

bool MeshDataRenderPack::setRenderCommand(const IRenderCommand& command, const RenderTask& task)
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

#pragma optimize("", off)
void MeshDataRenderPack::excute(IRenderContext& context, RenderTask& task, RenderTaskContext& taskContext)
{
	if (meshBatchDrawCalls.empty())
		return;

	static const ShaderPropertyName depthMapName = "depthMap";

	if (taskContext.materialVariant != task.materialVariant) {
		taskContext.materialVariant = task.materialVariant;

		bindMaterial(context, task.materialVariant);
		if (lightDataPack.shadowTarget == NULL)
			context.bindTexture((ITexture*)Texture2D::whiteRGBADefaultTex.getVendorTexture(), depthMapName);
		else
			context.bindTexture((ITexture*)lightDataPack.shadowTarget->getDepthTexture()->getVendorTexture(), depthMapName);
	}

	for (auto& item : meshBatchDrawCalls)
	{
		for (int passIndex = 0; passIndex < materialData->desc.passNum; passIndex++) {
			materialData->desc.currentPass = passIndex;
			context.setDrawInfo(passIndex, materialData->desc.passNum, materialData->desc.materialID);
			context.bindDrawInfo();
			unsigned int commandOffset = item->getDrawCommandOffset();
			unsigned int commandEnd = commandOffset + item->getDrawCommandCount();
			for (; commandOffset < commandEnd; commandOffset++) {
				RENDER_DESC_SCOPE(context, DrawMesh, "Material(%s)", AssetInfo::getPath(materialData->material).c_str());
				context.drawMeshIndirect(taskContext.batchDrawData.batchDrawCommandArray->getCommandBuffer(), sizeof(DrawElementsIndirectCommand) * commandOffset);
			}
		}
	}
}
#pragma optimize("", on)

void MeshDataRenderPack::reset()
{
	materialData = NULL;
	meshBatchDrawCalls.clear();
}
