#pragma once

#include "Spine2DConfig.h"

class Spine2DMeshData : public MeshData
{
public:
	bool inited = false;
	bool updated = true;

	spine::Skeleton* model = NULL;

	GPUBuffer vertexBuffer = GPUBuffer(GB_Vertex, GBF_Float3);
	GPUBuffer uvBuffer = GPUBuffer(GB_Vertex, GBF_Float2);
	GPUBuffer elementBuffer = GPUBuffer(GB_Index, GBF_UInt);

	vector<MeshPart> meshParts;

	BoundBox bounds;

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