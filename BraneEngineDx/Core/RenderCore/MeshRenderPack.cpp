#include "MeshRenderPack.h"
#include "RenderCommandList.h"

bool MeshRenderCommand::isValid() const
{
	return sceneData && transformData && material && !material->isNull() && mesh != NULL && mesh->isValid();
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

	if (meshRenderCommand->transformIndex) {
		setRenderData(meshRenderCommand->mesh, meshRenderCommand->transformIndex);
	}

	return true;
}

void MeshDataRenderPack::setRenderData(MeshPart* part, MeshTransformIndex* data)
{
	meshParts.insert(pair<MeshPart*, MeshTransformIndex*>(part, data));
}

void MeshDataRenderPack::excute(IRenderContext& context, RenderTaskContext& taskContext)
{
	if (meshParts.empty())
		return;

	newVendorRenderExecution();

	if (taskContext.materialData != materialData) {
		taskContext.materialData = materialData;
		materialData->bind(context);
		if (lightDataPack.shadowTarget == NULL)
			context.bindTexture((ITexture*)Texture2D::whiteRGBADefaultTex.getVendorTexture(), "depthMap");
		else
			context.bindTexture((ITexture*)lightDataPack.shadowTarget->getDepthTexture()->getVendorTexture(), "depthMap");
	}

	cmds.resize(meshParts.size());
	int index = 0;
	for (auto b = meshParts.begin(), e = meshParts.end(); b != e; b++, index++) {
		DrawElementsIndirectCommand& c = cmds[index];
		c.baseVertex = b->first->vertexFirst;
		c.count = b->first->elementCount;
		c.firstIndex = b->first->elementFirst;
		if (b->second) {
			c.instanceCount = b->second->batchCount;
			c.baseInstance = b->second->indexBase;
		}
		else {
			c.instanceCount = 1;
			c.baseInstance = 0;
		}
	}

	for (int passIndex = 0; passIndex < materialData->desc.passNum; passIndex++) {
		materialData->desc.currentPass = passIndex;
		context.setDrawInfo(passIndex, materialData->desc.passNum, materialData->desc.materialID);
		context.execteMeshDraw(vendorRenderExecution, cmds);
	}
}
