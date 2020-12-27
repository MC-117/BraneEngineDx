#pragma once
#ifndef _MESHDATA_H_
#define _MESHDATA_H_

#include "Unit.h"

enum MeshType
{
	MT_Mesh, MT_SkeletonMesh
};

class MeshData
{
public:
	MeshType type = MT_Mesh;
	vector<Vector3f> vertices;
	vector<Vector2f> uvs;
	vector<Vector3f> normals;
	vector<Vector3u> elements;
	MeshData();
	virtual ~MeshData();

	virtual bool isGenerated() const = 0;
	virtual void bindShape() = 0;
protected:
	static MeshData* currentMeshData;
};

struct MeshPartDesc
{
	MeshData* meshData = NULL;
	unsigned int vertexFirst = 0;
	unsigned int vertexCount = 0;
	unsigned int elementFirst = 0;
	unsigned int elementCount = 0;
};

#endif // !_IMESHDATA_H_
