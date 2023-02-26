#pragma once
#ifndef _MESH_H_
#define _MESH_H_

#include "Shape.h"
#include "MeshData.h"

struct MeshPart;
class Mesh;

struct MeshPart
{
	MeshData* meshData = NULL;
	unsigned int vertexFirst = 0;
	unsigned int vertexCount = 0;
	unsigned int elementFirst = 0;
	unsigned int elementCount = 0;
	unsigned int vertexPerFace = 3;
	Mesh* mesh = NULL;
	unsigned int partIndex = -1;
	BoundBox bound;

	MeshPart();
	MeshPart(const MeshPart& part);
	MeshPart(const MeshPartDesc& desc);
	MeshPart(MeshData* meshData, unsigned int vertexFirst, unsigned int vertexCount,
		unsigned int indexFirst, unsigned int indexCount);

	bool isValid();
	bool isSkeleton();
	bool isMorph();

	inline Vector3f& vertex(unsigned int index) {
		if (index >= vertexCount)
			throw overflow_error("MeshPart vertex access overflow");
		return meshData->vertices[vertexFirst + index];
	}
	inline Vector2f& uv(unsigned int index) {
		if (index >= vertexCount)
			throw overflow_error("MeshPart uv access overflow");
		return meshData->uvs[vertexFirst + index];
	}
	inline Vector3f& normal(unsigned int index) {
		if (index >= vertexCount)
			throw overflow_error("MeshPart normal access overflow");
		return meshData->normals[vertexFirst + index];
	}
	inline unsigned int& element(unsigned int index) {
		if (index >= elementCount)
			throw overflow_error("MeshPart index access overflow");
		return meshData->elements[elementFirst + index];
	}

	inline Vector3f& vertex(unsigned int index) const {
		if (index >= vertexCount)
			throw overflow_error("MeshPart vertex access overflow");
		return meshData->vertices[vertexFirst + index];
	}
	inline Vector2f& uv(unsigned int index) const {
		if (index >= vertexCount)
			throw overflow_error("MeshPart uv access overflow");
		return meshData->uvs[vertexFirst + index];
	}
	inline Vector3f& normal(unsigned int index) const {
		if (index >= vertexCount)
			throw overflow_error("MeshPart normal access overflow");
		return meshData->normals[vertexFirst + index];
	}
	inline unsigned int& element(unsigned int index) const  {
		if (index >= elementCount)
			throw overflow_error("MeshPart index access overflow");
		return meshData->elements[elementFirst + index];
	}

	MeshPart clone() const;
};

class Mesh : public Shape
{
public:
	multimap<string, unsigned int> meshPartNameMap;
	vector<string> partNames;
	vector<MeshPart> meshParts;

	MeshPart totalMeshPart;

	int vertCount = 0;
	int vertexPerFace = 3;
	int faceCount = 0;

	Mesh();
	Mesh(const Mesh& mesh);
	Mesh(int faceVertCount, int faceCount, float* vert, float* uv = NULL, float* norm = NULL);
	virtual ~Mesh();

	void setTotalMeshPart(const MeshPart& part);
	void addMeshPart(const string& name, MeshPart& part);
	void resize(int vertexPerFace, int faceCount, int vertexCount);
	void clone(const Mesh& mesh);

	virtual bool writeObjStream(ostream& os, const vector<int>& partIndex) const;

#if ENABLE_PHYSICS
	virtual CollisionShape* generateComplexCollisionShape(const Vector3f& scale = Vector3f(1, 1, 1));
	virtual PFabric* generateCloth(unsigned int partIndex);
#endif

	Mesh& operator=(Mesh& mesh);
#if ENABLE_PHYSICS
protected:
	PxBase* collisionMesh = NULL;
	vector<PFabric*> fabricParts;
#endif
};

#endif // !_MESH_H_