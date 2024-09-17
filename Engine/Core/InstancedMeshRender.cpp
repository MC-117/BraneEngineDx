#include "InstancedMeshRender.h"
#include "RenderCore/MeshRenderPack.h"

InstancedMeshRender::InstancedMeshRender()
	: meshBatchDrawData({ &meshTransformRenderData, &meshBatchDrawCommandArray })
{
}

bool InstancedMeshRender::getNeedUpdate() const
{
	for (const auto collection : collections) {
		if (collection->getNeedUpdate())
			return true;
	}
	return false;
}

void InstancedMeshRender::setNeedUpdate(bool update)
{
	if (update) {
		for (const auto collection : collections) {
			collection->markCachedMeshCommandDirty();
		}
	}
}

int InstancedMeshRender::getMeshCount() const
{
	return collections.size();
}

MeshMaterialCollection* InstancedMeshRender::getMesh(int index) const
{
	if (index < collections.size())
		return collections[index];
	return NULL;
}

MeshMaterialCollection* InstancedMeshRender::addMesh(Mesh& mesh)
{
	MeshMaterialCollection* collection = new MeshMaterialCollection;
	collection->setNeedCacheMeshCommand(true);
	collection->setMesh(&mesh);
	collections.emplace_back(collection);
	return collection;
}

MeshBatchDrawData& InstancedMeshRender::getMeshBatchDrawData()
{
	return meshBatchDrawData;
}

void InstancedMeshRender::render(RenderInfo& info)
{
	MeshMaterialCollection::DispatchData dispatchData;
	dispatchData.init<MeshRenderCommand>();
	dispatchData.hidden = hidden;
	dispatchData.isStatic = isStatic;
	dispatchData.canCastShadow = canCastShadow;
	dispatchData.hasPrePass = hasPrePass;
	dispatchData.meshBatchDrawData = meshBatchDrawData;

	bool meshCommandDirty = false;
	for (const auto collection : collections) {
		if (collection->getNeedUpdate()) {
			meshCommandDirty = true;
			break;
		}
	}

	for (const auto collection : collections) {
		if (meshCommandDirty) {
			collection->markCachedMeshCommandDirty();
		}
		collection->dispatchMeshDraw(dispatchData);
	}
}

Matrix4f InstancedMeshRender::getTransformMatrix() const
{
	return transformMat;
}

IRendering::RenderType InstancedMeshRender::getRenderType() const
{
	return IRendering::RenderType::Normal_Render;
}

Shape* InstancedMeshRender::getShape() const
{
	return Render::getShape();
}

Material* InstancedMeshRender::getMaterial(unsigned index)
{
	return Render::getMaterial(index);
}

bool InstancedMeshRender::getMaterialEnable(unsigned index)
{
	return Render::getMaterialEnable(index);
}

Shader* InstancedMeshRender::getShader() const
{
	return Render::getShader();
}

InstancedTransformRenderDataHandle InstancedMeshRender::getInstancedTransformRenderDataHandle(unsigned int index) const
{
	InstancedTransformRenderDataHandle handle;
	handle.batchDrawData = meshBatchDrawData;
	MeshMaterialCollection* collection = getMesh(index);
	if (collection == NULL)
		return handle;
	collection->gatherInstanceInfo(handle);
	return handle;
}