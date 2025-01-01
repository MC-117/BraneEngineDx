#include "TerrainGeometry.h"
#ifdef PHYSICS_USE_PHYSX
#include "../Physics/PhysicalWorld.h"
#endif
#include "../RenderCore/RenderCore.h"
#include "../Utility/MathUtility.h"

TerrainBasePatchMeshData::TerrainBasePatchMeshData()
	: sourceMap(NULL)
	, heightMap(false)
{
	type = MT_Terrain;
}

TerrainBasePatchMeshData::~TerrainBasePatchMeshData()
{
	release();
}

bool TerrainBasePatchMeshData::isValid() const
{
	return inited && data.widthPerGrid > 0 && data.height > 0 &&
		data.grid != Vector2u::Zero() && heightMap.isValid();
}

Vector2u TerrainBasePatchMeshData::getPreferredResolution() const
{
	return data.grid * MAX_TERRAIN_TESS_FACTOR;
}

void TerrainBasePatchMeshData::setTerrainData(const TerrainData& inData)
{
	if (inData.grid == Vector2u::Zero())
		return;

	if (!inited) {
		needUpdateData = true;
		needUpdateMesh = true;
		updateMesh();
		inited = true;
	}

	if (data.widthPerGrid != inData.widthPerGrid) {
		data.widthPerGrid = inData.widthPerGrid;
		needUpdateData = true;
	}
	if (data.height != inData.height) {
		data.height = inData.height;
		needUpdateData = true;
	}
	if (data.widthPerTriangle != inData.widthPerTriangle) {
		data.widthPerTriangle = inData.widthPerTriangle;
		needUpdateData = true;
	}
	if (data.grid != inData.grid) {
		data.grid = inData.grid;
		needUpdateData = true;
		updateMesh();
		if (sourceMap)
			updateHeightMap(*sourceMap);
	}
}

void TerrainBasePatchMeshData::updateHeightMap(const Texture2D& src)
{
	sourceMap = &src;
	
	Vector2u preferredSize = getPreferredResolution();
	heightMap.setTextureInfo(Texture2DInfo{ TW_Clamp, TW_Clamp });
	heightMap.copyFrom(src, preferredSize.x(), preferredSize.y());
}

void TerrainBasePatchMeshData::update()
{
	if (needUpdateMesh) {
		vertexBuffer.uploadData(vertices.size(), vertices.data());
		uvBuffer.uploadData(uvs.size(), uvs.data());
		elementBuffer.uploadData(elements.size(), elements.data());
		needUpdateMesh = false;
	}
	
	if (needUpdateData) {
		terrainBuffer.uploadData(1, &data);
		needUpdateData = false;
	}
}

void TerrainBasePatchMeshData::release()
{
	vertexBuffer.resize(0);
	uvBuffer.resize(0);
	elementBuffer.resize(0);
	terrainBuffer.resize(0);
	needUpdateMesh = false;
	needUpdateData = false;
}

bool TerrainBasePatchMeshData::isGenerated() const
{
	return true;
}

void TerrainBasePatchMeshData::init()
{
	if (!isValid())
		return;
	update();
	heightMap.bind();
}

void TerrainBasePatchMeshData::bindShape()
{
	if (!isValid())
		return;
	update();
	if (currentMeshData == this)
		return;
	bindDrawContext();
	vertexBuffer.bindBase(1);
	uvBuffer.bindBase(2);
	elementBuffer.bindBase(0);
	terrainBuffer.bindBase(MAT_BASE_BIND_INDEX);
	heightMap.bindBase(TERRAIN_MAP_BIND_INDEX);
	currentMeshData = this;
}

void TerrainBasePatchMeshData::bindShapeWithContext(IRenderContext& context)
{
	if (!isValid())
		return;
	if (currentMeshData == this)
		return;
	static const ShaderPropertyName TerrainDataName = "TerrainData";
	static const ShaderPropertyName terrainMapName = "terrainMap";

	bindDrawContext(context);
	context.bindBufferBase(vertexBuffer.getVendorGPUBuffer(), 1);
	context.bindBufferBase(uvBuffer.getVendorGPUBuffer(), 2);
	context.bindBufferBase(elementBuffer.getVendorGPUBuffer(), 0);
	context.bindBufferBase(terrainBuffer.getVendorGPUBuffer(), TerrainDataName); // MAT_BASE_BIND_INDEX
	context.bindTexture((ITexture*)heightMap.getVendorTexture(), terrainMapName);// TERRAIN_MAP_BIND_INDEX, TERRAIN_MAP_BIND_INDEX);
	context.currentMeshData = this;
}

TerrainQuadPatchMeshData::TerrainQuadPatchMeshData()
{
	vertexPerFace = 4;
}

void TerrainQuadPatchMeshData::updateMesh()
{
	int vxcount = data.grid.x() + 1;
	int vycount = data.grid.y() + 1;
	int vcount = vxcount * vycount;
	int icount = data.grid.x() * data.grid.y() * 4;

	vertices.resize(vcount);
	uvs.resize(vcount);
	elements.resize(icount);

	for (int x = 0; x < vxcount; x++) {
		for (int y = 0; y < vycount; y++) {
			int index = y * vxcount + x;
			vertices[index] = Vector3f(x, y, 0);
			uvs[index] = Vector2f(x / (float)vxcount, y / (float)vycount);
		}
	}

	for (int x = 0; x < data.grid.x(); x++) {
		for (int y = 0; y < data.grid.y(); y++) {
			int base = y * data.grid.x() + x;
			unsigned int* indices = elements.data() + base * 4;
			indices[0] = y * vxcount + x;
			indices[1] = indices[0] + 1;
			indices[2] = indices[1] + vxcount;
			indices[3] = indices[2] - 1;
		}
	}
	needUpdateMesh = true;
}

void TerrainQuadPatchMeshData::bindDrawContext()
{
	VendorManager::getInstance().getVendor().setTerrainDrawContext();
}

void TerrainQuadPatchMeshData::bindDrawContext(IRenderContext& context)
{
	context.setTerrainDrawContext();
}

int TerrainQuadPatchMeshData::getLodCount() const
{
	return 0;
}

MeshPart TerrainQuadPatchMeshData::getLodMeshPart(int lod)
{
	BoundBox bound;
	bound.minPoint = Vector3f::Zero();
	bound.maxPoint = Vector3f(data.widthPerGrid * data.grid.x(), data.widthPerGrid * data.grid.y(), data.height);
	
	MeshPart meshPart;
	meshPart.meshData = this;
	meshPart.vertexPerFace = vertexPerFace;
	meshPart.partIndex = 0;
	meshPart.bound = bound;
	meshPart.vertexFirst = 0;
	meshPart.elementFirst = 0;
	meshPart.vertexCount = vertices.size();
	meshPart.elementCount = elements.size();

	return meshPart;
}

TerrainTrianglePatchMeshData::TerrainTrianglePatchMeshData()
{
	vertexPerFace = 3;
}

void TerrainTrianglePatchMeshData::setTerrainData(const TerrainData& inData)
{
	int edgeTriCount = data.widthPerGrid / data.widthPerTriangle;
	edgeTriCount = 1 >> int(std::ceil(std::log2(edgeTriCount)));
	TerrainData newData = inData;
	newData.widthPerTriangle = data.widthPerGrid / edgeTriCount;
	TerrainBasePatchMeshData::setTerrainData(newData);
}

void TerrainTrianglePatchMeshData::updateMesh()
{
	int edgeTriCount = data.trianglesPerTile;
	int edgeVertCount = edgeTriCount + 1;

	int vcount = edgeVertCount * edgeVertCount;
	int qcount = edgeTriCount * edgeTriCount;
	int icount = edgeTriCount * edgeTriCount * 6;

	vertices.resize(vcount);
	uvs.resize(vcount);
	elements.resize(icount);

	for (int x = 0; x < edgeVertCount; x++) {
		for (int y = 0; y < edgeVertCount; y++) {
			int index = y * edgeVertCount + x;
			vertices[index] = Vector3f(x, y, 0);
			uvs[index] = Vector2f(x / (float)edgeVertCount, y / (float)edgeVertCount);
		}
	}

	for (uint32_t i = 0; i < qcount; i++) {
		uint32_t x = Math::invertMortonCode2(i);
		uint32_t y = Math::invertMortonCode2(i >> 1);

		unsigned int* indices = elements.data() + i * 6;

		indices[0] = y * edgeVertCount + x;
		indices[1] = indices[0] + 1;
		indices[2] = indices[1] + edgeVertCount;
		indices[3] = indices[2];
		indices[4] = indices[2] - 1;
		indices[5] = indices[0];
	}
	
	needUpdateMesh = true;
}

void TerrainTrianglePatchMeshData::bindDrawContext()
{
	VendorManager::getInstance().getVendor().setMeshDrawContext();
}

void TerrainTrianglePatchMeshData::bindDrawContext(IRenderContext& context)
{
	context.setMeshDrawContext();
}

int TerrainTrianglePatchMeshData::getLodCount() const
{
	return std::max(1, (int)std::log2(data.widthPerGrid / data.widthPerTriangle));
}

MeshPart TerrainTrianglePatchMeshData::getLodMeshPart(int lod)
{
	int lodCount = getLodCount();
	if (lod >= lodCount)
		return MeshPart();
	BoundBox bound;
	bound.minPoint = Vector3f::Zero();
	bound.maxPoint = Vector3f(data.widthPerGrid * data.grid.x(), data.widthPerGrid * data.grid.y(), data.height);

	int edgeTriCount = 1 << (lodCount - lod);
	int icount = edgeTriCount * edgeTriCount * 6;

	assert(icount <= elements.size());

	MeshPart meshPart;
	meshPart.meshData = this;
	meshPart.vertexPerFace = vertexPerFace;
	meshPart.partIndex = 0;
	meshPart.bound = bound;
	meshPart.vertexFirst = 0;
	meshPart.elementFirst = 0;
	meshPart.vertexCount = vertices.size();
	meshPart.elementCount = edgeTriCount * edgeTriCount * 6;

	return meshPart;
}

SerializeInstance(TerrainGeometry);

TerrainGeometry::TerrainGeometry()
{
	setTerrainData(meshData.data);
}

bool TerrainGeometry::isValid() const
{
	return meshData.isValid();
}

TerrainData TerrainGeometry::getTerrainData() const
{
	return meshData.data;
}

void TerrainGeometry::setTerrainData(const TerrainData& data)
{
	bound.minPoint = Vector3f::Zero();
	bound.maxPoint = Vector3f(data.widthPerGrid * data.grid.x(), data.widthPerGrid * data.grid.y(), data.height);

	meshData.setTerrainData(data);

	for (int lod = 0; lod < meshData.getLodCount(); lod++) {
		
	}
	meshPart = meshData.getLodMeshPart(0);

	meshPart.meshData = &meshData;
	meshPart.vertexPerFace = meshData.vertexPerFace;
	meshPart.partIndex = 0;
	meshPart.bound = bound;
	meshPart.vertexFirst = 0;
	meshPart.elementFirst = 0;
	meshPart.vertexCount = meshData.vertices.size();
	meshPart.elementCount = meshData.elements.size();
}

const float sampleRate = 1;

#ifdef PHYSICS_USE_PHYSX
void TerrainGeometry::updateCollisionShape()
{
	if (!meshData.isValid())
		return;

	int xvcount = meshData.heightMap.getWidth() * sampleRate;
	int yvcount = meshData.heightMap.getHeight() * sampleRate;
	int size = xvcount * yvcount;

	PxHeightFieldSample* samples = new PxHeightFieldSample[size];
	for (int x = 0; x < xvcount; x++) {
		for (int y = 0; y < yvcount; y++) {
			int index = y * xvcount + x;
			PxHeightFieldSample& sample = samples[index];
			sample.materialIndex0 = 0;
			sample.materialIndex1 = 0;
			sample.clearTessFlag();
			Color color;
			meshData.heightMap.getPixel(color, y / sampleRate, x / sampleRate);
			sample.height = 10000 * color.a;
		}
	}

	PxHeightFieldDesc desc;
	desc.setToDefault();
	desc.format = PxHeightFieldFormat::eS16_TM;
	desc.nbRows = yvcount;
	desc.nbColumns = xvcount;
	desc.samples.data = samples;
	desc.samples.stride = sizeof(PxHeightFieldSample);
	//desc.convexEdgeThreshold = 3;

	if (pxHeightField) {
		pxHeightField->modifySamples(0, 0, desc, true);
	}
	else {
		pxHeightField = PhysicalWorld::gCooking->createHeightField(desc, PhysicalWorld::gPhysicsSDK->getPhysicsInsertionCallback());
	}

	delete[] samples;
}

CollisionShape* TerrainGeometry::generateComplexCollisionShape(const Vector3f& scale)
{
	Vector2u girds = meshData.data.grid;
	float height = meshData.data.height;
	float widthPerGrid = meshData.data.widthPerGrid;
	int xvcount = meshData.heightMap.getWidth();
	int yvcount = meshData.heightMap.getHeight();

	updateCollisionShape();

	if (!pxHeightField)
		return NULL;
	return new PxHeightFieldGeometry(pxHeightField, PxMeshGeometryFlags(), height * scale.z() * 0.0001f,
		girds.y() * widthPerGrid / (yvcount * sampleRate) * scale.y(), girds.x() * widthPerGrid / (xvcount * sampleRate) * scale.x());
}
#endif

Serializable* TerrainGeometry::instantiate(const SerializationInfo& from)
{
	return new TerrainGeometry();
}
