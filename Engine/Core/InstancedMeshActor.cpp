#include "InstancedMeshActor.h"
#include "Utility/Utility.h"
#include "Asset.h"
#include "Engine.h"
#include "RenderCore/RenderCore.h"
#undef min
#undef max

SerializeInstance(InstancedMeshActor);

InstancedMeshActor::InstancedMeshActor(string name) : Actor(name)
{
	instancedMeshRender.isStatic = false;
	instancedMeshRender.canCastShadow = false;
	instancedMeshRender.frustumCulling = false;
	instancedMeshRender.hasPrePass = false;
}

MeshMaterialCollection* InstancedMeshActor::addMesh(Mesh& mesh)
{
	return instancedMeshRender.addMesh(mesh);
}

void InstancedMeshActor::set(float density, Vector2f& bound)
{
	this->density = density;
	this->bound = bound;
}

void InstancedMeshActor::updateData()
{
	instancedMeshRender.setNeedUpdate(true);
}

void InstancedMeshActor::begin()
{
	Actor::begin();
	instancedMeshRender.setNeedUpdate(true);
}

void InstancedMeshActor::end()
{
	Actor::end();
	instancedMeshRender.setNeedUpdate(true);
}

void InstancedMeshActor::prerender(SceneRenderData& sceneData)
{
	Actor::prerender(sceneData);
	const bool needUpdate = instancedMeshRender.getNeedUpdate();
	if (!needUpdate)
		return;
	
	auto cleanFunc = ([this] (RenderThreadContext& context)
	{
		MeshBatchDrawData meshBatchDrawData = instancedMeshRender.getMeshBatchDrawData();
		meshBatchDrawData.transformData->release();
		meshBatchDrawData.batchDrawCommandArray->clean();
	});
	RENDER_THREAD_ENQUEUE_TASK(InstancedMeshActorCleanTransform, cleanFunc);
	
	float w = bound.x() / density;
	int yd = bound.y() / w;
	float yo = (bound.y() - yd * w) * 0.5;
	float hx = bound.x() * 0.5, hy = bound.y() * 0.5;
	unsigned int instanceCount = (yd + 1) * (density + 1);
	
	MeshTransformData data;
	getMeshTransformData(&data);
	int meshCount = instancedMeshRender.getMeshCount();
	for (int meshIndex = 0; meshIndex < meshCount; meshIndex++) {
		MeshMaterialCollection* collection = instancedMeshRender.getMesh(meshIndex);
		MeshTransformData instancedData = data;
		collection->getMeshTransformData(&instancedData);
		auto func = ([this, density = density, w, yd, yo, hx, hy,
			instancedData, instanceCount, collection, transformMat = transformMat, meshIndex, meshCount] (RenderThreadContext& context)
		{
			static int runCount = 0;
			Console::log("InstancedMeshActorUpdateTransform Run %d", runCount);
			runCount++;
			MeshBatchDrawData meshBatchDrawData = instancedMeshRender.getMeshBatchDrawData();
			auto reservedData = meshBatchDrawData.transformData->addMeshTransform(instanceCount);
			if (!reservedData.isValid())
				throw overflow_error("MeshTransform Overflow");
			MeshTransformData* uploadData = reservedData.beginModify();
			unsigned int flatIndex = 0;
			for (int i = 0; i <= density; i++) {
				for (int j = 0; j <= yd; j++) {
					if (flatIndex >= instanceCount) {
						throw overflow_error("MeshTransform Overflow");
					}
					MeshTransformData& targetData = uploadData[flatIndex];
					targetData = instancedData;
					Matrix4f m = Matrix4f::Identity();
					m(0, 3) = i * w - hx + ((rand() / (double)RAND_MAX) - 0.5) * 2 * w;
					m(1, 3) = j * w - hy + yo + ((rand() / (double)RAND_MAX) - 0.5) * 2 * w;
					m.block(0, 0, 3, 3) = Quaternionf::FromAngleAxis((rand() / (double)RAND_MAX) * 2 * PI, Vector3f(0, 0, 1)).toRotationMatrix();
					targetData.localToWorld = MATRIX_UPLOAD_OP(transformMat * m);
					++flatIndex;
				}
			}
			reservedData.endModify();
			collection->setInstanceInfo(reservedData.getBaseIndex(), reservedData.getCount());
		});
		RENDER_THREAD_ENQUEUE_TASK(InstancedMeshActorUpdateTransform, func);
	}
}

Render* InstancedMeshActor::getRender()
{
	return &instancedMeshRender;
}

unsigned int InstancedMeshActor::getRenders(vector<Render*>& renders)
{
	renders.push_back(&instancedMeshRender);
	return 1;
}

void InstancedMeshActor::setHidden(bool value)
{
	instancedMeshRender.hidden = value;
}

bool InstancedMeshActor::isHidden()
{
	return instancedMeshRender.hidden;
}

Serializable * InstancedMeshActor::instantiate(const SerializationInfo & from)
{
	InstancedMeshActor* gma = new InstancedMeshActor(from.name);
	ChildrenInstantiate(Object, from, gma);
	return gma;
}

bool InstancedMeshActor::deserialize(const SerializationInfo & from)
{
	if (!Actor::deserialize(from))
		return false;
	from.get("density", density);
	SVector2f boundf;
	if (from.get("bound", boundf)) {
		bound.x() = boundf.x;
		bound.y() = boundf.y;
	}

	const SerializationInfo* meshInfos = from.get("meshes");
	if (meshInfos == NULL)
		return true;
	for (int meshIndex = 0; meshIndex < meshInfos->sublists.size(); meshIndex++) {
		Mesh* mesh = NULL;
		const SerializationInfo* meshinfo = meshInfos->get(meshIndex);
		const SerializationInfo* minfo = meshinfo->get("mesh");
		if (minfo != NULL) {
			string pathType;
			if (minfo->get("pathType", pathType)) {
				string path;
				if (minfo->get("path", path))
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
			continue;
		MeshMaterialCollection* collection = addMesh(*mesh);
		collection->deserialize(*meshinfo);
	}
	return true;
}

bool InstancedMeshActor::serialize(SerializationInfo & to)
{
	if (!Actor::serialize(to))
		return false;
	to.set("density", density);
	SVector2f boundf;
	boundf.x = bound.x();
	boundf.y = bound.y();
	to.set("bound", boundf);

	SerializationInfo* meshInfos = to.addArray("meshes", "MeshMaterialCollection");
	
	for (int meshIndex = 0; meshIndex < instancedMeshRender.getMeshCount(); meshIndex++) {
		SerializationInfo* meshInfo = meshInfos->push();
		MeshMaterialCollection* collection = instancedMeshRender.getMesh(meshIndex);
		string path = MeshAssetInfo::getPath(collection->getMesh());
		if (!path.empty()) {
			SerializationInfo* minfo = meshInfo->add("mesh");
			if (minfo != NULL) {
				minfo->type = "AssetSearch";
				minfo->set("path", path);
				minfo->set("pathType", "path");
			}
		}
		collection->serialize(*meshInfo);
	}
	return true;
}
