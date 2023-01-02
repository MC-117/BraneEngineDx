#include "MeshRender.h"
#include "Asset.h"
#include "Engine.h"
#include "GUI/Gizmo.h"
#include "Editor/Editor.h"
#include "RenderCore/RenderCore.h"

MeshRender::MeshRender()
{
	setMesh(NULL);
	materials[0] = NULL;
	canCastShadow = true;
}

MeshRender::MeshRender(Mesh& mesh, Material& material)
{
	setMesh(&mesh);
	materials[0] = &material;
	canCastShadow = true;
}


MeshRender::~MeshRender()
{
}

void MeshRender::setMesh(Mesh* mesh)
{
	this->mesh = mesh;
	remapMaterial();
}

void MeshRender::setBaseColor(Color color)
{
	materials[0]->setBaseColor(color);
}

Color MeshRender::getBaseColor()
{
	return materials[0]->getBaseColor();
}

Material * MeshRender::getMaterial(const string & name)
{
	if (mesh == NULL)
		return NULL;
	remapMaterial();
	auto iter = mesh->meshPartNameMap.find(name);
	if (iter == mesh->meshPartNameMap.end())
		return nullptr;
	return materials[iter->second];
}

pair<string, Material*> MeshRender::getMaterial(int index)
{
	if (mesh != NULL) {
		remapMaterial();
		if (index < materials.size())
			for (auto b = mesh->meshPartNameMap.begin(), e = mesh->meshPartNameMap.end(); b != e; b++) {
				if (b->second == index) {
					return pair<string, Material*>(b->first, materials[index]);
				}
			}
	}
	return pair<string, Material*>();
}

bool MeshRender::getPartEnable(const string& name)
{
	if (mesh == NULL)
		return false;
	remapMaterial();
	auto iter = mesh->meshPartNameMap.find(name);
	if (iter == mesh->meshPartNameMap.end())
		return false;
	return meshPartsEnable[iter->second];
}

bool MeshRender::getPartEnable(int index)
{
	remapMaterial();
	if (index < materials.size())
		return meshPartsEnable[index];
	return false;
}

bool MeshRender::setMaterial(const string & name, Material & material, bool all)
{
	if (mesh == NULL)
		return false;
	bool re = false;
	for (auto b = mesh->meshPartNameMap.begin(), e = mesh->meshPartNameMap.end(); b != e; b++) {
		if (b->first == name) {
			materials[b->second] = &material;
			re = true;
			if (!all)
				break;
		}
	}
	return re;
}

bool MeshRender::setMaterial(int index, Material & material)
{
	if (index >= materials.size())
		return false;
	materials[index] = &material;
	return true;
}

bool MeshRender::setPartEnable(const string& name, bool enable, bool all)
{
	if (mesh == NULL)
		return false;
	bool re = false;
	for (auto b = mesh->meshPartNameMap.begin(), e = mesh->meshPartNameMap.end(); b != e; b++) {
		if (b->first == name) {
			meshPartsEnable[b->second] = enable;
			re = true;
			if (!all)
				break;
		}
	}
	return re;
}

bool MeshRender::setPartEnable(int index, bool enable)
{
	if (index >= meshPartsEnable.size())
		return false;
	meshPartsEnable[index] = enable;
	return true;
}

void MeshRender::fillMaterialsByDefault()
{
	/*if (outlineMaterial == NULL) {
		outlineMaterial = getAssetByPath<Material>("Engine/Shaders/Outline.mat");
		if (outlineMaterial != NULL)
			outlineMaterial = &outlineMaterial->instantiate();
	}*/
}

void MeshRender::preRender()
{
}

void MeshRender::render(RenderInfo& info)
{
	if (mesh == NULL || hidden || instanceID < 0)
		return;

	remapMaterial();
	fillMaterialsByDefault();

	for (int i = 0; i < materials.size(); i++) {
		Material* material = materials[i];
		if (material == NULL || !meshPartsEnable[i])
			continue;
		MeshPart* part = &mesh->meshParts[i];

		if (!part->isValid())
			continue;

		/*if (frustumCulling && !info.sceneData->frustumCulling(part->bound, transformMat)) {
			continue;
		}*/

		MeshRenderCommand command;
		command.sceneData = info.sceneData;
		command.material = material;
		command.mesh = part;
		command.hasPreDepth = hasPrePass;
		command.instanceID = instanceID;
		command.instanceIDCount = instanceCount;
		MeshTransformRenderData* transformData = isStatic ? &info.sceneData->staticMeshTransformDataPack : &info.sceneData->meshTransformDataPack;
		command.transformData = transformData;
		command.transformIndex = transformData->setMeshPartTransform(part, material, instanceID, instanceCount);
		info.renderGraph->setRenderCommand(command);

		Material* outlineMaterial = outlineMaterials[i];
		if (outlineEnable[i] && outlineMaterial != NULL) {
			command.transformIndex = transformData->setMeshPartTransform(part, outlineMaterial, instanceID, instanceCount);
			command.material = outlineMaterial;
			info.renderGraph->setRenderCommand(command);
		}
	}
}

Matrix4f MeshRender::getTransformMatrix() const
{
	return transformMat;
}

IRendering::RenderType MeshRender::getRenderType() const
{
	return IRendering::RenderType::Normal;
}

Shape * MeshRender::getShape() const
{
	return mesh;
}

Material * MeshRender::getMaterial(unsigned int index)
{
	if (index < materials.size())
		return materials[index];
	else
		return NULL;
}

bool MeshRender::getMaterialEnable(unsigned int index)
{
	if (index < meshPartsEnable.size())
		return meshPartsEnable[index];
	else
		return false;
}

Shader * MeshRender::getShader() const
{
	return materials[0]->getShader();
}

void MeshRender::remapMaterial()
{
	int size = (mesh == NULL || mesh->meshParts.size() == 0) ? 1 : mesh->meshParts.size();
	if (materials.size() != size) {
		materials.resize(size, NULL);
		meshPartsEnable.resize(size, true);
		outlineMaterials.resize(size, NULL);
		outlineEnable.resize(size, false);
	}
}
