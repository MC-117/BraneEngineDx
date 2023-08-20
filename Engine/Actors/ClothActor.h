#pragma once

#include "../Core/Physics/ClothBody.h"
#include "../Core/MeshActor.h"

class ClothActor : public MeshActor
{
public:
	Serialize(ClothActor, MeshActor);

	ClothBody clothBody;
	ClothActor(const string& name = "ClothActor");

	virtual void afterTick();

	virtual void setMesh(Mesh* mesh);

	void addSphereCollider(const Vector3f& offset, float radius);
	void addBoxCollider(const Vector3f& center, const Vector3f& extend);

	virtual void setupPhysics(PhysicalWorld& physicalWorld);
	virtual void releasePhysics(PhysicalWorld& physicalWorld);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	Mesh* baseMesh = NULL;
};