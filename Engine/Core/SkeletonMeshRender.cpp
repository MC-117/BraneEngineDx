#include "SkeletonMeshRender.h"
#include "Asset.h"
#include "Console.h"
#include "Camera.h"
#include "RenderCore/RenderCore.h"
#include "Importer/MaterialImporter.h"
#include "RenderCore/RenderCoreUtility.h"

SkeletonMeshRender::SkeletonMeshRender() : MeshRender()
{
}

SkeletonMeshRender::SkeletonMeshRender(SkeletonMesh & mesh, Material& material)
	: MeshRender(mesh, material)
{
}

SkeletonMeshRender::~SkeletonMeshRender()
{
}

void SkeletonMeshRender::setMesh(Mesh* mesh)
{
	skeletonMesh = dynamic_cast<SkeletonMesh*>(mesh);
	mesh = skeletonMesh;
	MeshRender::setMesh(mesh);
	if (skeletonMesh != NULL) {
		getRenderData()->setBoneCount(skeletonMesh->skeletonData.getBoneCount());
		morphWeights.setMesh(*mesh);
	}
}

unsigned int SkeletonMeshRender::getBoneCount() const
{
	return skeletonMesh ? skeletonMesh->skeletonData.getBoneCount() : 0;
}

void SkeletonMeshRender::resetBoneTransform()
{
	SkeletonRenderData* renderData = getRenderData();
	renderData->clean();
	renderData->setBoneCount(skeletonMesh->skeletonData.getBoneCount());
}

void SkeletonMeshRender::setBoneTransform(unsigned int index, const Matrix4f& mat)
{
	getRenderData()->updateBoneTransform(mat, index);
}

void SkeletonMeshRender::fillMaterialsByDefault()
{
	/*for (int i = 0; i < materials.size(); i++) {
		if (materials[i] == NULL) {
			materials[i] = &Material::defaultMaterial;
		}
	}*/
	/*if (outlineMaterial == NULL) {
		outlineMaterial = getAssetByPath<Material>("Engine/Shaders/Outline.mat");
		if (outlineMaterial != NULL)
			outlineMaterial = &outlineMaterial->instantiate();
	}*/
}

void SkeletonMeshRender::render(RenderInfo & info)
{
	bool hasMorphWeights = morphWeights.getMorphCount();
	IRenderData* renderData = getRenderData();
	IRenderData* morphWeightsRenderData = morphWeights.getRenderData();

	MeshMaterialCollection::DispatchData dispatchData;
	dispatchData.init<MeshRenderCommand>();
	dispatchData.hidden = hidden;
	dispatchData.isStatic = isStatic;
	dispatchData.canCastShadow = canCastShadow;
	dispatchData.hasPrePass = hasPrePass;

	auto bindRenderData = [renderData, hasMorphWeights, morphWeightsRenderData](MeshRenderCommand& command)
	{
		command.bindings.push_back(renderData);
		if (hasMorphWeights)
			command.bindings.push_back(morphWeightsRenderData);
	};

	dispatchData.renderDelegate += bindRenderData;

	collection.dispatchMeshDraw(dispatchData);
	outlineCollection.dispatchMeshDraw(dispatchData);
}

vector<Matrix4f> & SkeletonMeshRender::getTransformMatrixs()
{
	return getRenderData()->skeletonTransformArray.transformDatas;
}

SkeletonRenderData* SkeletonMeshRender::getRenderData()
{
	if (renderData == NULL)
		renderData = new SkeletonRenderData();
	return renderData;
}

bool SkeletonMeshRender::deserialize(const SerializationInfo& from)
{
	outlineCollection.deserialize(from);
	collection.deserialize(from);
	return true;
}

bool SkeletonMeshRender::serialize(SerializationInfo& to)
{
	string meshPath = AssetInfo::getPath(skeletonMesh);
	to.add("skeletonMesh", meshPath);

	outlineCollection.serialize(to);
	collection.serialize(to);
	return true;
}
