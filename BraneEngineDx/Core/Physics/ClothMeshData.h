#pragma once
#include "../Mesh.h"
#include "../GPUBuffer.h"

class ClothMeshData : public MeshData
{
public:
	bool inited = false;
	bool updated = true;

	vector<float> invMass;

	GPUBuffer vertexBuffer = GPUBuffer(GPUBufferType::GB_Vertex, sizeof(Vector3f));
	GPUBuffer normalBuffer = GPUBuffer(GPUBufferType::GB_Vertex, sizeof(Vector3f));
	GPUBuffer uvBuffer = GPUBuffer(GPUBufferType::GB_Vertex, sizeof(Vector2f));
	GPUBuffer elementBuffer = GPUBuffer(GPUBufferType::GB_Index, sizeof(unsigned int));

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