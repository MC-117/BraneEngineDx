#pragma once

#include "MeshActor.h"
#include "InstancedMeshRender.h"
#include <random>

struct MeshTransformRenderData;

class ENGINE_API InstancedMeshActor : public Actor
{
public:
	Serialize(InstancedMeshActor, Actor);

	float density = 20;
	Vector2f bound = { 100, 100 };

	InstancedMeshRender instancedMeshRender;
	
	InstancedMeshActor(string name = "InstancedMeshActor");

	virtual MeshMaterialCollection* addMesh(Mesh& mesh);

	void set(float density, Vector2f& bound);
	void updateData();
	virtual void begin();
	virtual void end();
	virtual void prerender(SceneRenderData& sceneData);
	
	virtual Render* getRender();
	virtual unsigned int getRenders(vector<Render*>& renders);

	virtual void setHidden(bool value);
	virtual bool isHidden();

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	bool update = true;
	unsigned int transCount = 0;
	MeshTransformRenderData* renderData = NULL;
};
