#include "ClothActor.h"
#include "../Core/Asset.h"
#include "../Core/Importer/MaterialImporter.h"

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

	meshRender.outlineCollection.deserialize(from);
	meshRender.collection.deserialize(from);
	return true;
}

bool ClothActor::serialize(SerializationInfo& to)
{
	Actor::serialize(to);
	if (meshRender.getMesh() != NULL) {
		string path = MeshAssetInfo::getPath(baseMesh);
		if (!path.empty()) {
			SerializationInfo* minfo = to.add("mesh");
			if (minfo != NULL) {
				minfo->type = "AssetSearch";
				minfo->set("path", path);
				minfo->set("pathType", "path");
			}
		}
		
		meshRender.outlineCollection.serialize(to);
		meshRender.collection.serialize(to);
	}
	return true;
}
