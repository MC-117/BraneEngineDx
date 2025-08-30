#pragma once

#include "../Shape.h"
#include "../Mesh.h"
#include "../Texture2D.h"
#include "../GPUBuffer.h"
#include "../RenderCore/TerrainData.h"

#define MAX_TERRAIN_TESS_FACTOR 16

class ENGINE_API TerrainBasePatchMeshData : public MeshData
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

	TerrainBasePatchMeshData();
	virtual ~TerrainBasePatchMeshData();

	bool isValid() const;

	Vector2u getPreferredResolution() const;

	virtual void setTerrainData(const TerrainData& inData);
	void updateHeightMap(const Texture2D& src);

	virtual void updateMesh() = 0;

	virtual int getLodCount() const = 0;
	virtual MeshPart getLodMeshPart(int Lod) = 0;

	void update();
	void release();
	
	virtual bool isGenerated() const;
	virtual void init();
	virtual void bindShape();
	virtual void bindShapeWithContext(IRenderContext& context);
};

class ENGINE_API TerrainQuadPatchMeshData : public TerrainBasePatchMeshData
{
public:
	TerrainQuadPatchMeshData();
	
	virtual void updateMesh();

	virtual int getLodCount() const;
	virtual MeshPart getLodMeshPart(int lod);
};

class ENGINE_API TerrainTrianglePatchMeshData : public TerrainBasePatchMeshData
{
public:
	TerrainTrianglePatchMeshData();

	virtual void setTerrainData(const TerrainData& inData);
	
	virtual void updateMesh();

	virtual int getLodCount() const;
	virtual MeshPart getLodMeshPart(int lod);
};

struct MeshTransformData;

class ENGINE_API TerrainGeometry : public Shape
{
public:
	Serialize(TerrainGeometry, Shape);

	TerrainQuadPatchMeshData meshData;
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