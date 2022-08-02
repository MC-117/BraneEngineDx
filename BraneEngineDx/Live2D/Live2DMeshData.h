#pragma once

#include "Live2DConfig.h"

class Live2DMeshData : public MeshData
{
public:
	bool inited = false;
	bool updated = true;

	Csm::CubismModel* model = NULL;

	GPUBuffer vertexBuffer = GPUBuffer(GPUBufferType::GB_Vertex, sizeof(Vector3f));
	GPUBuffer uvBuffer = GPUBuffer(GPUBufferType::GB_Vertex, sizeof(Vector2f));
	GPUBuffer elementBuffer = GPUBuffer(GPUBufferType::GB_Index, sizeof(unsigned int));

	vector<MeshPart> meshParts;

	Live2DMeshData();
	virtual ~Live2DMeshData();

	void release();

	void setModel(Csm::CubismModel* model);
	void updateElement(int index);
	void updateVertex(int index);

	const MeshPart* getMeshPart(int index) const;

	virtual bool isValid() const;
	virtual bool isGenerated() const;
	virtual void bindShape();
};