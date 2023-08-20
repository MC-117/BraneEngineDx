#include "ClothActor.h"
#include "../Core/Asset.h"

SerializeInstance(ClothActor);

ClothActor::ClothActor(const string& name)
    : MeshActor(name), clothBody(*this, PhysicalMaterial(0, DYNAMIC))
{
}

void ClothActor::afterTick()
{
#if ENABLE_PHYSICS
	if (clothBody.isValid())
		clothBody.updateObjectTransform();
#endif
	MeshActor::afterTick();
}

void ClothActor::setMesh(Mesh* mesh)
{
    if (mesh == NULL)
        return;
    baseMesh = mesh;
    for (int i = 0; i < mesh->meshParts.size(); i++) {
        clothBody.addMeshPart(mesh->partNames[i], mesh->meshParts[i]);
    }
    meshRender.setMesh(&clothBody.getMesh());
}

void ClothActor::addSphereCollider(const Vector3f& offset, float radius)
{
	updataRigidBody();
	PhysicalCollider* collider = rigidBody->addCollider(new Sphere(radius));
	collider->setPositionOffset(offset);
	collider = clothBody.addCollider(new Sphere(radius));
	collider->setPositionOffset(offset);
}

void ClothActor::addBoxCollider(const Vector3f& center, const Vector3f& extend)
{
	updataRigidBody();
	Vector3f A = extend * -1;
	Vector3f B = extend;
	PhysicalCollider* collider = rigidBody->addCollider(new Box(A, B));
	collider->setPositionOffset(center);
	collider = clothBody.addCollider(new Box(A, B));
	collider->setPositionOffset(center);
}

void ClothActor::setupPhysics(PhysicalWorld& physicalWorld)
{
    MeshActor::setupPhysics(physicalWorld);
    if (clothBody.physicalWorld == NULL) {
        physicalWorld.addPhysicalBody(clothBody);
    }
}

void ClothActor::releasePhysics(PhysicalWorld& physicalWorld)
{
    MeshActor::releasePhysics(physicalWorld);
    physicalWorld.removePhysicalBody(clothBody);
}

Serializable* ClothActor::instantiate(const SerializationInfo& from)
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
	ClothActor* ca = new ClothActor(from.name);
	ca->setMesh(mesh);
	ChildrenInstantiate(Object, from, ca);
	return ca;
}

bool ClothActor::deserialize(const SerializationInfo& from)
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

bool ClothActor::serialize(SerializationInfo& to)
{
	Actor::serialize(to);
	if (meshRender.mesh != NULL) {
		string path = MeshAssetInfo::getPath(baseMesh);
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
