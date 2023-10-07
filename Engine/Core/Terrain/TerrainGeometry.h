#pragma once

#include "../Shape.h"
#include "../Mesh.h"
#include "../Texture2D.h"
#include "../GPUBuffer.h"

#define MAX_TERRAIN_TESS_FACTOR 16

struct TerrainData
{
	float unit = 500;
	float height = 500;
	Vector2u grid = { 8, 8 };
	float triSize = 16;
	Vector3f pading;
};

class ENGINE_API TerrainPatchMeshData : public MeshData
{
public:
	bool inited = false;
	bool needUpdateMesh = true;
	bool needUpdateData = true;

	const Texture2D* sourceMap;
	Texture2D heightMap;
	TerrainData data;

	GPUBuffer vertexBuffer = GPUBuffer(GB_Vertex, GBF_Float3);
	GPUBuffer uvBuffer = GPUBuffer(GB_Vertex, GBF_Float2);
	GPUBuffer elementBuffer = GPUBuffer(GB_Index, GBF_UInt);
	GPUBuffer terrainBuffer = GPUBuffer(GB_Constant, GBF_Struct, sizeof(TerrainData));

	TerrainPatchMeshData();
	virtual ~TerrainPatchMeshData();

	bool isValid() const;

	Vector2u getPreferredResolution() const;

	void setTerrainData(const TerrainData& inData);
	void updateHeightMap(const Texture2D& src);

	void updateMesh();

	void update();
	void release();
	
	virtual bool isGenerated() const;
	virtual void init();
	virtual void bindShape();
	virtual void bindShapeWithContext(IRenderContext& context);
};

class ENGINE_API TerrainGeometry : public Shape
{
public:
	Serialize(TerrainGeometry, Shape);

	TerrainPatchMeshData meshData;
	MeshPart meshPart;
	PxHeightField* pxHeightField = NULL;

	TerrainGeometry();

	bool isValid() const;

	TerrainData getTerrainData() const;

	void setTerrainData(const TerrainData& data);

#if ENABLE_PHYSICS
	void updateCollisionShape();
	virtual CollisionShape* generateComplexCollisionShape(const Vector3f& scale = Vector3f(1, 1, 1));
#endif

	static Serializable* instantiate(const SerializationInfo& from);
};