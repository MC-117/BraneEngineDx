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
	return new DirectShadowRenderPack(sceneData.meshTransformDataPack);
}

DirectShadowRenderPack::DirectShadowRenderPack(MeshTransformRenderData& meshTransformDataPack)
	: meshTransformDataPack(meshTransformDataPack)
{
}

bool DirectShadowRenderPack::setRenderCommand(const IRenderCommand& command)
{
	const DirectShadowRenderCommand* directShadowRenderCommand = dynamic_cast<const DirectShadowRenderCommand*>(&command);
	if (directShadowRenderCommand == NULL || directShadowRenderCommand->transformIndex == NULL)
		return false;

	materialData = dynamic_cast<MaterialRenderData*>(command.material->getRenderData());
	if (materialData == NULL)
		return false;

	setRenderData(directShadowRenderCommand->mesh, directShadowRenderCommand->transformIndex);

	return true;
}

void DirectShadowRenderPack::setRenderData(MeshPart* part, MeshTransformIndex* data)
{
	meshParts.insert(pair<MeshPart*, MeshTransformIndex*>(part, data));
}

void DirectShadowRenderPack::excute(IRenderContext& context, RenderTaskContext& taskContext)
{
	if (meshParts.empty())
		return;

	newVendorRenderExecution();

	if (taskContext.materialData != materialData) {
		taskContext.materialData = materialData;
		materialData->bind(context);
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
		context.setDrawInfo(passIndex, materialData->desc.passNum, 0);
		context.execteMeshDraw(vendorRenderExecution, cmds);
	}
}
