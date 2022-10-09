#include "SkeletonMeshRender.h"
#include "Asset.h"
#include "Console.h"

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
		transformMats.resize(skeletonMesh->skeletonData.boneList.size());
		morphWeights.setMesh(*mesh);
	}
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
	if (mesh == NULL || hidden || customRenaderSubmit)
		return;

	remapMaterial();
	fillMaterialsByDefault();

	if (!customTransformSubmit)
		instanceID = info.sceneData->setMeshTransform(transformMats);

	for (int i = 0; i < materials.size(); i++) {
		Material* material = materials[i];

		if (material == NULL || !meshPartsEnable[i])
			continue;

		MeshPart* part = &mesh->meshParts[i];

		if (!part->isValid())
			continue;

		if (!customTransformSubmit)
			info.sceneData->setMeshPartTransform(part, material, instanceID);

		MeshRenderCommand command;
		command.sceneData = info.sceneData;
		command.camera = info.camera;
		command.material = material;
		command.mesh = part;
		command.isStatic = isStatic;
		if (morphWeights.getMorphCount() > 0)
			command.bindings.push_back(morphWeights.getRenderData());

		info.renderGraph->setRenderCommand(command);

		Material* outlineMaterial = outlineMaterials[i];
		if (outlineEnable[i] && outlineMaterial != NULL) {
			if (!customTransformSubmit)
				info.sceneData->setMeshPartTransform(part, outlineMaterial, instanceID);
			command.material = outlineMaterial;
			info.renderGraph->setRenderCommand(command);
		}
	}
}

vector<Matrix4f> & SkeletonMeshRender::getTransformMatrixs()
{
	return transformMats;
}

bool SkeletonMeshRender::deserialize(const SerializationInfo& from)
{
	{
		const SerializationInfo* outlineinfos = from.get("outlines");
		if (outlineinfos != NULL) {
			if (outlineinfos->type == "Array") {
				for (int i = 0; i < outlineMaterials.size(); i++) {
					const SerializationInfo* outline = outlineinfos->get(i);
					if (outline != NULL) {
						string imat;
						outline->get("material", imat);
						if (!imat.empty()) {
							istringstream stream = istringstream(imat);
							outlineMaterials[i] = Material::MaterialLoader::loadMaterialInstance(stream, "Outline");
						}
						string boolString;
						outline->get("enable", boolString);
						outlineEnable[i] = boolString == "true";
					}
				}
			}
			else if (skeletonMesh != NULL) {
				for (int i = 0; i < outlineinfos->sublists.size(); i++) {
					const SerializationInfo& outline = outlineinfos->sublists[i];
					string imat;
					outline.get("material", imat);
					Material* outlineMaterial = NULL;
					if (!imat.empty()) {
						istringstream stream = istringstream(imat);
						outlineMaterial = Material::MaterialLoader::loadMaterialInstance(stream, "Outline");
					}
					string boolString;
					outline.get("enable", boolString);
					auto iter = skeletonMesh->meshPartNameMap.find(outline.name);
					if (iter != skeletonMesh->meshPartNameMap.end()) {
						outlineEnable[iter->second] = boolString == "true";
						outlineMaterials[iter->second] = outlineMaterial;
					}
					else {
						if (outlineMaterial != NULL) {
							delete outlineMaterial;
						}
					}
				}
			}
		}
	}

	{
		const SerializationInfo* minfo = from.get("materials");
		if (minfo != NULL) {
			if (minfo->type == "Array")
				for (int i = 0; i < materials.size(); i++) {
					const SerializationInfo* mi = minfo->get(i);
					if (mi != NULL) {
						string path;
						if (!mi->get("path", path))
							continue;
						string pathType;
						Material* mat = NULL;
						if (path == "default")
							mat = &Material::defaultMaterial;
						else {
							if (!mi->get("pathType", pathType))
								continue;
							if (pathType == "name") {
								mat = getAsset<Material>("Material", path);
							}
							else if (pathType == "path") {
								mat = getAssetByPath<Material>(path);
							}
						}
						if (mat != NULL) {
							setMaterial(i, *mat);
						}
						else {
							Console::warn("SkeletonMeshActor: cannot find material '%s' when deserialization",
								path.c_str());
						}
					}
				}
			else
				for (int i = 0; i < minfo->sublists.size(); i++) {
					const SerializationInfo& mi = minfo->sublists[i];
					string path;
					if (!mi.get("path", path))
						continue;
					string pathType;
					Material* mat = NULL;
					if (path == "default")
						mat = &Material::defaultMaterial;
					else {
						if (!mi.get("pathType", pathType))
							continue;
						if (pathType == "name") {
							mat = getAsset<Material>("Material", path);
						}
						else if (pathType == "path") {
							mat = getAssetByPath<Material>(path);
						}
					}
					if (mat != NULL) {
						if (!setMaterial(mi.name, *mat, true))
							Console::warn("SkeletonMeshActor: cannot find material slot '%s' when deserialization",
								mi.name.c_str());
					}
					else {
						Console::warn("SkeletonMeshActor: cannot find material '%s' when deserialization",
							path.c_str());
					}
				}
		}
	}
	return true;
}

bool SkeletonMeshRender::serialize(SerializationInfo& to)
{
	string meshPath = AssetInfo::getPath(skeletonMesh);
	to.add("skeletonMesh", meshPath);

	{
		SerializationInfo* outlineinfos = to.add("outlines");
		outlineinfos->type = "Array";
		outlineinfos->arrayType = "Outline";
		if (outlineinfos != NULL) {
			for (int i = 0; i < outlineMaterials.size(); i++) {
				SerializationInfo* outline = outlineinfos->push();
				if (outline != NULL) {
					string imat;
					Material* outlineMaterial = outlineMaterials[i];
					if (outlineMaterial != NULL)
						Material::MaterialLoader::saveMaterialInstanceToString(imat, *outlineMaterial);
					outline->set("material", imat);
					outline->set("enable", outlineEnable[i] ? "true" : "false");
				}
			}
		}
	}

	{
		SerializationInfo& minfo = *to.add("materials");
		minfo.type = "Array";
		minfo.arrayType = "AssetSearch";
		for (int i = 0; i < materials.size(); i++) {
			SerializationInfo& info = *minfo.push();
			string path;
			string pathType;
			Material* mat = materials[i];
			if (mat == NULL)
				continue;
			if (mat == &Material::defaultMaterial) {
				path = "default";
				pathType = "name";
			}
			else {
				path = AssetInfo::getPath(materials[i]);
				pathType = "path";
			}
			if (path.empty()) {
				path = mat->getShaderName();
				pathType = "name";
			}
			info.add("path", path);
			info.add("pathType", pathType);
		}
	}
	return true;
}
