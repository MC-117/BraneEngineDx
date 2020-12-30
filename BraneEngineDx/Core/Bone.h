#pragma once
#ifndef _BONE_H_
#define _BONE_H_

#include "Actor.h"
#include "MeshRender.h"

class Bone : public Actor
{
public:
	Serialize(Bone);

	MeshRender sphereMeshRender;
	MeshRender coneMeshRender;

	Bone(const string& name = "Bone");

	virtual void prerender();
	virtual Render* getRender();
	virtual unsigned int getRenders(vector<Render*>& renders);

	virtual void setHidden(bool value);
	virtual bool isHidden();

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	static Mesh sphereMesh;
	static Mesh coneMesh;
	static Shader outLineShader;
	static Material outLineMaterial;
	static bool isLoadDefaultResource;

	static void loadDefaultResource();
};

#endif // !_BONE_H_
