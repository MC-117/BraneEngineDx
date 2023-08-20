#pragma once

#include "Live2DConfig.h"

class Live2DMeshData : public MeshData
{
public:
	bool inited = false;
	bool updated = true;

	Csm::CubismModel* model = NULL;

	GPUBuffer vertexBuffer = GPUBuffer(GB_Vertex, GBF_Float3);
	GPUBuffer uvBuffer = GPUBuffer(GB_Vertex, GBF_Float2);
	GPUBuffer elementBuffer = GPUBuffer(GB_Index, GBF_UInt);

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
	virtual void init();
	virtual void bindShape();
	virtual void bindShapeWithContext(IRenderContext& context);
};