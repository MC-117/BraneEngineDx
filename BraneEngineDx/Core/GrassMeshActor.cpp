#include "GrassMeshActor.h"
#include "Utility/Utility.h"
#include "Asset.h"
#include "Engine.h"
#include "RenderCore/RenderCore.h"
#undef min
#undef max

SerializeInstance(GrassMeshActor);

GrassMeshActor::GrassMeshActor(Mesh & mesh, Material & material, string name)
	: MeshActor(mesh, material, name)
{
	meshRender.isStatic = true;
	meshRender.canCastShadow = false;
	meshRender.frustumCulling = false;
	meshRender.hasPrePass = false;
	events.registerFunc("updateData", [](void* obj) {
		((GrassMeshActor*)obj)->updateData();
	});
	events.registerFunc("setBoundX", [](void* obj, float x) {
		((GrassMeshActor*)obj)->bound.x() = x;
	});
	events.registerFunc("setBoundY", [](void* obj, float y) {
		((GrassMeshActor*)obj)->bound.y() = y;
	});
	events.registerFunc("setDensity", [](void* obj, float d) {
		((GrassMeshActor*)obj)->density = d;
	});
}

void GrassMeshActor::set(float density, Vector2f& bound)
{
	this->density = density;
	this->bound = bound;
}

void GrassMeshActor::updateData()
{
	update = true;
}

void GrassMeshActor::begin()
{
	MeshActor::begin();
	update = true;
}

void GrassMeshActor::tick(float delteTime)
{
	MeshActor::tick(delteTime);
	float time = Engine::getCurrentWorld()->getEngineTime() * 0.000000001f;
	for (auto b = meshRender.materials.begin(), e = meshRender.materials.end(); b != e; b++)
		if (*b != NULL)
			(*b)->setScalar("time", time);
}

void GrassMeshActor::end()
{
	MeshActor::end();
	update = true;
}

void GrassMeshActor::prerender(SceneRenderData& sceneData)
{
	if (sceneData.willUpdateStatic()) {
		float w = bound.x() / density;
		int yd = bound.y() / w;
		float yo = (bound.y() - yd * w) * 0.5;
		float hx = bound.x() * 0.5, hy = bound.y() * 0.5;
		transCount = (yd + 1) * (density + 1);
		srand(1234);
		for (int i = 0; i <= density; i++) {
			for (int j = 0; j <= yd; j++) {
				Matrix4f m = Matrix4f::Identity();
				m(0, 3) = i * w - hx + ((rand() / (double)RAND_MAX) - 0.5) * 2 * w;
				m(1, 3) = j * w - hy + yo + ((rand() / (double)RAND_MAX) - 0.5) * 2 * w;
				m.block(0, 0, 3, 3) = Quaternionf::FromAngleAxis((rand() / (double)RAND_MAX) * 2 * PI, Vector3f(0, 0, 1)).toRotationMatrix();
				unsigned int transID = sceneData.setStaticMeshTransform(transformMat * m);
				if (i == 0 && j == 0) {
					meshRender.instanceID = transID;
					meshRender.instanceCount = transCount;
				}
			}
		}
	}
	else if (update) {
		sceneData.setUpdateStatic();
	}
	update = false;
}

Serializable * GrassMeshActor::instantiate(const SerializationInfo & from)
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
	GrassMeshActor* gma = new GrassMeshActor(*mesh, *getAssetByPath<Material>("Engine/Shaders/Default.mat"), from.name);
	ChildrenInstantiate(Object, from, gma);
	return gma;
}

bool GrassMeshActor::deserialize(const SerializationInfo & from)
{
	if (!MeshActor::deserialize(from))
		return false;
	from.get("density", density);
	SVector2f boundf;
	if (from.get("bound", boundf)) {
		bound.x() = boundf.x;
		bound.y() = boundf.y;
	}
	return true;
}

bool GrassMeshActor::serialize(SerializationInfo & to)
{
	if (!MeshActor::serialize(to))
		return false;
	to.set("density", density);
	SVector2f boundf;
	boundf.x = bound.x();
	boundf.y = bound.y();
	to.set("bound", boundf);
	return true;
}
