#pragma once
#ifndef _CLOTHBODY_H_
#define _CLOTHBODY_H_

#include "PhysicalBody.h"
#include "ClothMeshData.h"

class ClothBodyCollider : public PhysicalCollider
{
public:
	union {
		struct {
			unsigned int index;
		} sphere;
		struct {
			unsigned int sphereIndex[2];
			unsigned int index;
		} capsule;
		struct {
			unsigned int planeIndex[6];
			unsigned int index;
		} box;
	} index;

	ClothBodyCollider(Shape* shape, ShapeComplexType shapeComplexType = SIMPLE, Vector3f scale = Vector3f::Ones());
	virtual ~ClothBodyCollider();

	virtual void setPositionOffset(const Vector3f& offset);
	virtual void setRotationOffset(const Quaternionf& offset);

	void setSphere(PCloth& cloth);
	void setCapsule(PCloth& cloth);
	void setBox(PCloth& cloth);

	void setRawShape(PCloth& cloth);
	virtual void apply();
};

class ClothVertexCluster
{
public:
	ClothMeshData* mesh = NULL;
	vector<Vector4f> spheres;
	vector<int> indices;
	vector<Vector3f> vertices;

	Matrix4f bindedPose = Matrix4f::Identity();
	Matrix4f transformMat = Matrix4f::Identity();

	bool needUpdate = true;

	ClothVertexCluster(const Vector3f& positionOffset, const Quaternionf& rotationOffset);

	void addSphere(const Vector4f & sphere);

	void pickVertex();
	void updateVertex();

	void setReferencePose(const Vector3f& positionOffset, const Quaternionf& rotationOffset);

	int getVertexCount() const;

	Vector3f getVertexReferencePosition(int indexOfIndex) const;
	Vector3f getVertexPosition(int indexOfIndex) const;
};

class ClothBody : public PhysicalBody
{
	friend class ClothBodyCollider;
	friend class ClothVertexCluster;
public:
	ClothBody(::Transform& targetTransform, const PhysicalMaterial& material);
	virtual ~ClothBody();

	bool isValid() const;

	void addMeshPart(const string& name, const MeshPart& meshPart);
	ClothVertexCluster* addVertexCluster(const Vector3f& position, const Quaternionf& rotation);
	ClothVertexCluster* getVertexCluster(int index);
	int getVertexClusterCount() const;

	Mesh& getMesh();
	const ClothMeshData& getMeshData() const;

	virtual void initBody();
	virtual PhysicalCollider* addCollider(Shape* shape, ShapeComplexType shapeComplexType = SIMPLE);
	virtual void updateObjectTransform();
	virtual void addToWorld(PhysicalWorld& physicalWorld);
	virtual void removeFromWorld();
	virtual void* getSoftObject() const;
protected:
	ClothMeshData meshData;
	set<int> clusterIndices;
	vector<ClothVertexCluster*> clusters;
#ifdef PHYSICS_USE_PHYSX
	vector<PFabric*> rawClothFabrics;
	vector<PCloth*> rawClothBodies;
	nv::cloth::Solver* clothSolver = NULL;
#endif // !PHYSICS_USE_PHYSX
};

#endif // !_CLOTHBODY_H_