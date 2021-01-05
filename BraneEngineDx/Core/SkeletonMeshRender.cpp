#include "SkeletonMeshRender.h"
#include "Asset.h"

SkeletonMeshRender::SkeletonMeshRender(SkeletonMesh & mesh, Material& material)
	: MeshRender(mesh, material), skeletonMesh(mesh)
{
	transformMats.resize(mesh.skeletonData.boneList.size());
	morphWeights.resize(mesh.morphName.size(), 0);
}

SkeletonMeshRender::~SkeletonMeshRender()
{
}

void SkeletonMeshRender::fillMaterialsByDefault()
{
	/*for (int i = 0; i < materials.size(); i++) {
		if (materials[i] == NULL) {
			materials[i] = &Material::defaultMaterial;
		}
	}*/
	if (outlineMaterial == NULL) {
		outlineMaterial = getAssetByPath<Material>("Engine/Shaders/Outline.mat");
		if (outlineMaterial != NULL)
			outlineMaterial = &outlineMaterial->instantiate();
	}
}

bool SkeletonMeshRender::setMorphWeight(unsigned int index, float weight)
{
	if (index < morphWeights.size()) {
		morphWeights[index] = weight;
		morphUpdate = true;
		return true;
	}
	return false;
}

void SkeletonMeshRender::render(RenderInfo & info)
{
	if (hidden)
		return;
	remapMaterial();
	fillMaterialsByDefault();
	for (int i = 0; i < materials.size(); i++) {
		if (materials[i] == NULL)
			continue;
		info.cmdList->setRenderCommand({ materials[i], info.camera, &mesh.meshParts[i] }, isStatic);
		if (enableOutline && outlineMaterial != NULL)
			info.cmdList->setRenderCommand({ outlineMaterial, info.camera, &mesh.meshParts[i] }, isStatic);
	}
	if (morphUpdate) {
		skeletonMesh.updateMorphWeights(morphWeights);
		morphUpdate = false;
	}
}

vector<Matrix4f> & SkeletonMeshRender::getTransformMatrixs()
{
	return transformMats;
}
