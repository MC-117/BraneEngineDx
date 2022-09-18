#pragma once

#include "Spine2DConfig.h"

class Spine2DMeshData : public MeshData
{
public:
	bool inited = false;
	bool updated = true;

	spine::Skeleton* model = NULL;

	GPUBuffer vertexBuffer = GPUBuffer(GPUBufferType::GB_Vertex, sizeof(Vector3f));
	GPUBuffer uvBuffer = GPUBuffer(GPUBufferType::GB_Vertex, sizeof(Vector2f));
	GPUBuffer elementBuffer = GPUBuffer(GPUBufferType::GB_Index, sizeof(unsigned int));

	vector<MeshPart> meshParts;

	Range<Vector3f> bounds;

	Spine2DMeshData();
	virtual ~Spine2DMeshData();

	void release();

	void setModel(spine::Skeleton* model);
	void updateElement(int index);
	void updateVertex(int index);

	void updateBounds();

	const MeshPart* getMeshPart(int index) const;
	MeshPart* getMeshPart(int index);

	virtual bool isValid() const;
	virtual bool isGenerated() const;
	virtual void init();
	virtual void bindShape();
	virtual void bindShapeWithContext(IRenderContext& context);
};