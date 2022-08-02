#pragma once
#ifndef _MESHACTOR_H_
#define _MESHACTOR_H_

#include "Actor.h"
#include "MeshRender.h"
#include "Physics/PhysicalWorld.h"

class MeshActor : public Actor {
public:
	Serialize(MeshActor, Actor);

	MeshRender meshRender;

	MeshActor(const string& name = "MeshActor");
	MeshActor(Mesh& mesh, Material& material, const string& name = "MeshActor", const Vector3f& localScale = { 1, 1, 1 });
	MeshActor(Mesh& mesh, Material& material, const PhysicalMaterial& physicalMaterial, Shape& collisionShape, const string& name = "MeshActor", ShapeComplexType complexType = SIMPLE, const Vector3f& localScale = { 1, 1, 1 });

	virtual void setMesh(Mesh* mesh);

	virtual void prerender(RenderCommandList& cmdLst);
	virtual Render* getRender();
	virtual unsigned int getRenders(vector<Render*>& renders);

	virtual void setHidden(bool value);
	virtual bool isHidden();

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
};

#endif // !_MESHACTOR_H_
