#include "MeshActor.h"
#include "Utility.h"
#include "Asset.h"
#include "Console.h"

SerializeInstance(MeshActor);

MeshActor::MeshActor(Mesh & mesh, Material & material, string name, const Vector3f & localScale)
	: mesh(mesh), meshRender(mesh, material), Actor::Actor(name)
{
	setScale(localScale);
}

MeshActor::MeshActor(Mesh & mesh, Material & material, const PhysicalMaterial & physicalMaterial, Shape & collisionShape, string name, ShapeComplexType complexType, const Vector3f & localScale)
	: mesh(mesh), meshRender(mesh, material), Actor::Actor(name)
{
	setScale(localScale);
}

void MeshActor::setHidden(bool value)
{
	meshRender.hidden = value;
}

bool MeshActor::isHidden()
{
	return meshRender.hidden;
}

void MeshActor::prerender()
{
	meshRender.transformMat = transformMat;
	unsigned int transID = RenderCommandList::setMeshTransform(transformMat);
	meshRender.instanceID = transID;
	for (int i = 0; i < mesh.meshParts.size(); i++) {
		if (meshRender.materials[i] == NULL)
			continue;
		RenderCommandList::setMeshPartTransform(&mesh.meshParts[i], meshRender.materials[i], transID);
		if (meshRender.enableOutline && meshRender.outlineMaterial != NULL)
			RenderCommandList::setMeshPartTransform(&mesh.meshParts[i], meshRender.outlineMaterial, transID);
	}
}

Render* MeshActor::getRender()
{
	return &meshRender;
}

unsigned int MeshActor::getRenders(vector<Render*>& renders)
{
	renders.push_back(&meshRender);
	return 1;
}

Serializable * MeshActor::instantiate(const SerializationInfo & from)
{
	const SerializationInfo* minfo = from.get("mesh");
	if (minfo == NULL)
		return NULL;
	string pathType;
	if (!minfo->get("pathType", pathType))
		return NULL;
	string path;
	if (!minfo->get("path", path))
		return NULL;
	Mesh* mesh = NULL;
	if (pathType == "name") {
		mesh = getAsset<Mesh>("Mesh", path);
	}
	else if (pathType == "path") {
		mesh = getAssetByPath<Mesh>(path);
	}
	if (mesh == NULL)
		return NULL;
	MeshActor* ma = new MeshActor(*mesh, *getAssetByPath<Material>("Engine/Shaders/Default.mat"), from.name);
	ChildrenInstantiate(Object, from, ma);
	return ma;
}

bool MeshActor::deserialize(const SerializationInfo & from)
{
	if (!::Actor::deserialize(from))
		return false;
	const SerializationInfo* minfo = from.get("materials");
	if (minfo != NULL) {
		for (int i = 0; i < meshRender.materials.size(); i++) {
			const SerializationInfo* mi = minfo->get(i);
			if (mi != NULL) {
				string path;
				if (!mi->get("path", path))
					continue;
				string pathType;
				bool nullMat = false;
				Material* mat = NULL;
				if (path == "default")
					mat = getAssetByPath<Material>("Engine/Shaders/Default.mat");
				else if (path == "null")
					nullMat = true;
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
				if (!nullMat) {
					if (mat != NULL) {
						meshRender.setMaterial(i, *mat);
					}
					else {
						Console::warn("MeshActor: cannot find material '%s' when deserialization",
							path.c_str());
					}
				}
			}
		}
	}
	return true;
}

bool MeshActor::serialize(SerializationInfo & to)
{
	Actor::serialize(to);
	to.type = "MeshActor";
	string path = MeshAssetInfo::getPath(&mesh);
	if (!path.empty()) {
		SerializationInfo* minfo = to.add("mesh");
		if (minfo != NULL) {
			minfo->type = "AssetSearch";
			minfo->set("path", path);
			minfo->set("pathType", "path");
		}
	}
	SerializationInfo& minfo = *to.add("materials");
	minfo.type = "Array";
	minfo.arrayType = "AssetSearch";
	for (int i = 0; i < meshRender.materials.size(); i++) {
		SerializationInfo &info = *minfo.push();
		string path;
		string pathType;
		if (meshRender.materials[i] == NULL) {
			path = "null";
			pathType = "name";
			continue;
		}
		if (meshRender.materials[i] == &Material::defaultMaterial) {
			path = "default";
			pathType = "name";
		}
		else {
			path = AssetInfo::getPath(meshRender.materials[i]);
			pathType = "path";
		}
		if (path.empty()) {
			path = meshRender.materials[i]->getShaderName();
			pathType = "name";
		}
		info.add("path", path);
		info.add("pathType", pathType);
	}
	return true;
}