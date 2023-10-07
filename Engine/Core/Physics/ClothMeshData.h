#pragma once
#include "../Mesh.h"
#include "../GPUBuffer.h"

class ENGINE_API ClothMeshData : public MeshData
{
public:
	bool inited = false;
	bool updated = true;

	vector<float> invMass;

	GPUBuffer vertexBuffer = GPUBuffer(GB_Vertex, GBF_Float3);
	GPUBuffer normalBuffer = GPUBuffer(GB_Vertex, GBF_Float3);
	GPUBuffer uvBuffer = GPUBuffer(GB_Vertex, GBF_Float2);
	GPUBuffer elementBuffer = GPUBuffer(GB_Index, GBF_UInt);

	MeshPart totalPart;
	vector<string> partNames;
	vector<MeshPart> sourceMeshParts;
	vector<MeshPart> clothParts;

	Mesh mesh;

	ClothMeshData();
	virtual ~ClothMeshData();

	void release();

	void addMeshPart(const string& name, const MeshPart& meshPart);
	void apply(const Matrix4f& transformMat = Matrix4f::Identity());

	float& getMass(const MeshPart& part, int index);

	void updateElement();
	void updateVertex();
	void updateNormal();
	void updateUV();

	const MeshPart* getMeshPart(int index) const;

	virtual bool isValid() const;
	virtual bool isGenerated() const;
	virtual void init();
	virtual void bindShape();
	virtual void bindShapeWithContext(IRenderContext& context);
};