#include "MeshActor.h"
#include "Utility/Utility.h"
#include "Asset.h"
#include "Console.h"

SerializeInstance(MeshActor);

MeshActor::MeshActor(const string& name) : Actor::Actor(name)
{
}

MeshActor::MeshActor(Mesh & mesh, Material & material, const string& name, const Vector3f & localScale)
	: meshRender(mesh, material), Actor::Actor(name)
{
	setScale(localScale);
	rigidBody = new RigidBody(*this, PhysicalMaterial());
	rigidBody->addCollider(&mesh, SIMPLE);
}

MeshActor::MeshActor(Mesh & mesh, Material & material, const PhysicalMaterial & physicalMaterial, Shape & collisionShape, const string& name, ShapeComplexType complexType, const Vector3f & localScale)
	: meshRender(mesh, material), Actor::Actor(name)
{
	setScale(localScale);
	rigidBody = new RigidBody(*this, physicalMaterial);
	rigidBody->addCollider(&collisionShape, complexType);
}

void MeshActor::setMesh(Mesh* mesh)
{
	meshRender.setMesh(mesh);
	if (mesh != NULL && rigidBody == NULL) {
		rigidBody = new RigidBody(*this, PhysicalMaterial());
		rigidBody->addCollider(mesh, SIMPLE);
	}
}

void MeshActor::setHidden(bool value)
{
	meshRender.hidden = value;
}

bool MeshActor::isHidden()
{
	return meshRender.hidden;
}

void MeshActor::prerender(RenderCommandList& cmdLst)
{
	meshRender.transformMat = transformMat;
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
	Mesh* mesh = NULL;
	const SerializationInfo* meshinfo = from.get("mesh");
	if (meshinfo != NULL) {
		string pathType;
		if (meshinfo->get("pathType", pathType)) {
			string path;
			if (meshinfo->get("path", path))
			{
				if (pathType == "name") {
					mesh = getAsset<Mesh>("Mesh", path);
				}
				else if (pathType == "path") {
					mesh = getAssetByPath<Mesh>(path);
				}
			}
		}
	}
	if (mesh == NULL)
		return NULL;
	MeshActor* ma = new MeshActor(from.name);
	ma->setMesh(mesh);
	ChildrenInstantiate(Object, from, ma);
	return ma;
}

bool MeshActor::deserialize(const SerializationInfo & from)
{
	if (!::Actor::deserialize(from))
		return false;

	{
		const SerializationInfo* outlineinfos = from.get("outlines");
		if (outlineinfos != NULL) {
			for (int i = 0; i < meshRender.outlineMaterials.size(); i++) {
				const SerializationInfo* outline = outlineinfos->get(i);
				if (outline != NULL) {
					string imat;
					outline->get("material", imat);
					if (!imat.empty()) {
						istringstream stream = istringstream(imat);
						meshRender.outlineMaterials[i] = Material::MaterialLoader::loadMaterialInstance(stream, "Outline");
					}
					string boolString;
					outline->get("enable", boolString);
					meshRender.outlineEnable[i] = boolString == "true";
				}
			}
		}
	}
	
	{
		const SerializationInfo* matinfo = from.get("materials");
		if (matinfo != NULL) {
			for (int i = 0; i < meshRender.materials.size(); i++) {
				const SerializationInfo* mi = matinfo->get(i);
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
	}
	return true;
}

bool MeshActor::serialize(SerializationInfo & to)
{
	if (!Actor::serialize(to))
		return false;
	if (meshRender.mesh != NULL) {
		string path = MeshAssetInfo::getPath(meshRender.mesh);
		if (!path.empty()) {
			SerializationInfo* minfo = to.add("mesh");
			if (minfo != NULL) {
				minfo->type = "AssetSearch";
				minfo->set("path", path);
				minfo->set("pathType", "path");
			}
		}

		{
			SerializationInfo* outlineinfos = to.add("outlines");
			outlineinfos->type = "Array";
			outlineinfos->arrayType = "Outline";
			if (outlineinfos != NULL) {
				for (int i = 0; i < meshRender.outlineMaterials.size(); i++) {
					SerializationInfo* outline = outlineinfos->push();
					if (outline != NULL) {
						string imat;
						Material* outlineMaterial = meshRender.outlineMaterials[i];
						if (outlineMaterial != NULL)
							Material::MaterialLoader::saveMaterialInstanceToString(imat, *outlineMaterial);
						outline->set("material", imat);
						outline->set("enable", meshRender.outlineEnable[i] ? "true" : "false");
					}
				}
			}
		}

		{
			SerializationInfo& minfo = *to.add("materials");
			minfo.type = "Array";
			minfo.arrayType = "AssetSearch";
			for (int i = 0; i < meshRender.materials.size(); i++) {
				SerializationInfo& info = *minfo.push();
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
		}
	}
	return true;
}