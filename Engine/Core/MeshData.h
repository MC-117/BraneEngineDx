#pragma once
#ifndef _MESHDATA_H_
#define _MESHDATA_H_

#include "Unit.h"
#include "GraphicType.h"

class IRenderContext;

class ENGINE_API MeshData
{
public:
	static MeshData* currentMeshData;
	MeshType type = MT_Mesh;
	unsigned int vertexPerFace = 3;
	vector<Vector3f> vertices;
	vector<Vector2f> uvs;
	vector<Vector3f> normals;
	vector<unsigned int> elements;
	MeshData();
	virtual ~MeshData();

	virtual bool isValid() const = 0;
	virtual bool isGenerated() const = 0;
	virtual void init() = 0;
	virtual void bindShape() = 0;
	virtual void bindShapeWithContext(IRenderContext& constext);
};

struct MeshPartDesc
{
	MeshData* meshData = NULL;
	unsigned int vertexFirst = 0;
	unsigned int vertexCount = 0;
	unsigned int elementFirst = 0;
	unsigned int elementCount = 0;
	unsigned int vertexPerFace = 3;
};

#endif // !_IMESHDATA_H_
