#include "MeshActor.h"
#include "Utility/Utility.h"
#include "Asset.h"
#include "Console.h"
#include "RenderCore/RenderCore.h"
#include "Importer/MaterialImporter.h"

SerializeInstance(MeshActor);

MeshActor::MeshActor(const string& name) : Actor::Actor(name)
{
	meshRender.hasPrePass = true;
}

MeshActor::MeshActor(Mesh & mesh, Material & material, const string& name, const Vector3f & localScale)
	: meshRender(mesh, material), Actor::Actor(name)
{
	meshRender.hasPrePass = true;
	setScale(localScale);
	rigidBody = new RigidBody(*this, PhysicalMaterial());
	rigidBody->addCollider(&mesh, SIMPLE);
}

MeshActor::MeshActor(Mesh & mesh, Material & material, const PhysicalMaterial & physicalMaterial, Shape & collisionShape, const string& name, ShapeComplexType complexType, const Vector3f & localScale)
	: meshRender(mesh, material), Actor::Actor(name)
{
	meshRender.hasPrePass = true;
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

void MeshActor::prerender(SceneRenderData& sceneData)
{
	meshRender.transformMat = transformMat;
	MeshTransformData data;
	getMeshTransformData(&data);
	meshRender.getMeshTransformData(&data);
	RENDER_THREAD_ENQUEUE_TASK(MeshActorUpdateTransform, ([this, data] (RenderThreadContext& context)
	{
		unsigned int renderInstanceID = context.sceneRenderData->setMeshTransform(data);
		meshRender.setInstanceInfo(renderInstanceID, 1);
	}));
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

	meshRender.outlineCollection.deserialize(from);
	meshRender.collection.deserialize(from);
	return true;
}

bool MeshActor::serialize(SerializationInfo & to)
{
	if (!Actor::serialize(to))
		return false;
	Mesh* mesh = meshRender.getMesh();
	if (mesh != NULL) {
		string path = MeshAssetInfo::getPath(mesh);
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