#include "MeshRender.h"
#include "Asset.h"

MeshRender::MeshRender(Mesh& mesh, Material& material) : mesh(mesh)
{
	remapMaterial();
	materials[0] = &material;
	canCastShadow = true;
}


MeshRender::~MeshRender()
{
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
	remapMaterial();
	auto iter = mesh.meshPartNames.find(name);
	if (iter == mesh.meshPartNames.end())
		return nullptr;
	return materials[iter->second];
}

pair<string, Material*> MeshRender::getMaterial(int index)
{
	remapMaterial();
	if (index < materials.size())
		for (auto b = mesh.meshPartNames.begin(), e = mesh.meshPartNames.end(); b != e; b++) {
			if (b->second == index) {
				return pair<string, Material*>(b->first, materials[index]);
			}
		}
	return pair<string, Material*>();
}

bool MeshRender::setMaterial(const string & name, Material & material, bool all)
{
	bool re = false;
	for (auto b = mesh.meshPartNames.begin(), e = mesh.meshPartNames.end(); b != e; b++) {
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

void MeshRender::fillMaterialsByDefault()
{
	if (outlineMaterial == NULL) {
		outlineMaterial = getAssetByPath<Material>("Engine/Shaders/Outline.mat");
		if (outlineMaterial != NULL)
			outlineMaterial = &outlineMaterial->instantiate();
	}
}

void MeshRender::preRender()
{
}

void MeshRender::render(RenderInfo& info)
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
	return &mesh;
}

Material * MeshRender::getMaterial(unsigned int index)
{
	if (index < materials.size())
		return materials[index];
	else
		return NULL;
}

Shader * MeshRender::getShader() const
{
	return materials[0]->getShader();
}

void MeshRender::setupRenderResource()
{
	mesh.bindShape();
}

void MeshRender::remapMaterial()
{
	int size = mesh.meshParts.size() == 0 ? 1 : mesh.meshParts.size();
	if (materials.size() != size)
		materials.resize(size, NULL);
}
