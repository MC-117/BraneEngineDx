#include "TerrainGeometry.h"
#ifdef PHYSICS_USE_PHYSX
#include "../Physics/PhysicalWorld.h"
#endif

TerrainPatchMeshData::TerrainPatchMeshData()
{
	type = MT_Terrain;
	vertexPerFace = 4;
}

TerrainPatchMeshData::~TerrainPatchMeshData()
{
	release();
}

bool TerrainPatchMeshData::isValid() const
{
	return inited && data.unit > 0 && data.height > 0 &&
		data.grid != Vector2u::Zero() && heightMap.isValid();
}

Vector2u TerrainPatchMeshData::getPreferredResolution() const
{
	return data.grid * MAX_TERRAIN_TESS_FACTOR;
}

void TerrainPatchMeshData::setTerrainData(const TerrainData& inData)
{
	if (inData.grid == Vector2u::Zero())
		return;

	if (!inited) {
		needUpdateData = true;
		needUpdateMesh = true;
		updateMesh();
		inited = true;
	}

	if (data.unit != inData.unit) {
		data.unit = inData.unit;
		needUpdateData = true;
	}
	if (data.height != inData.height) {
		data.height = inData.height;
		needUpdateData = true;
	}
	if (data.triSize != inData.triSize) {
		data.triSize = inData.triSize;
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

void TerrainPatchMeshData::updateHeightMap(const Texture2D& src)
{
	sourceMap = &src;
	
	Vector2u preferredSize = getPreferredResolution();
	heightMap.copyFrom(src, preferredSize.x(), preferredSize.y());
	heightMap.setTextureInfo(Texture2DInfo{ TW_Clamp, TW_Clamp });
}

void TerrainPatchMeshData::updateMesh()
{
	needUpdateMesh = true;
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

void TerrainPatchMeshData::update()
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

void TerrainPatchMeshData::release()
{
	vertexBuffer.resize(0);
	uvBuffer.resize(0);
	elementBuffer.resize(0);
	terrainBuffer.resize(0);
	needUpdateMesh = false;
	needUpdateData = false;
}

bool TerrainPatchMeshData::isGenerated() const
{
	return true;
}

void TerrainPatchMeshData::init()
{
	if (!isValid())
		return;
	update();
	heightMap.bind();
}

void TerrainPatchMeshData::bindShape()
{
	if (!isValid())
		return;
	update();
	if (currentMeshData == this)
		return;
	VendorManager::getInstance().getVendor().setTerrainDrawContext();
	vertexBuffer.bindBase(1);
	uvBuffer.bindBase(2);
	elementBuffer.bindBase(0);
	terrainBuffer.bindBase(MAT_BASE_BIND_INDEX);
	heightMap.bindBase(TERRAIN_MAP_BIND_INDEX);
	currentMeshData = this;
}

void TerrainPatchMeshData::bindShapeWithContext(IRenderContext& context)
{
	if (!isValid())
		return;
	if (currentMeshData == this)
		return;
	context.setTerrainDrawContext();
	context.bindBufferBase(vertexBuffer.getVendorGPUBuffer(), 1);
	context.bindBufferBase(uvBuffer.getVendorGPUBuffer(), 2);
	context.bindBufferBase(elementBuffer.getVendorGPUBuffer(), 0);
	context.bindBufferBase(terrainBuffer.getVendorGPUBuffer(), "TerrainData"); // MAT_BASE_BIND_INDEX
	context.bindTexture((ITexture*)heightMap.getVendorTexture(), "terrainMap");// TERRAIN_MAP_BIND_INDEX, TERRAIN_MAP_BIND_INDEX);
	context.currentMeshData = this;
}

SerializeInstance(TerrainGeometry);

TerrainGeometry::TerrainGeometry()
{
	setTerrainData({ meshData.data.unit, meshData.data.height, meshData.data.grid });
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
	bound.minVal = Vector3f::Zero();
	bound.maxVal = Vector3f(data.unit * data.grid.x(), data.unit * data.grid.y(), data.height);

	meshData.setTerrainData(data);

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
	float unit = meshData.data.unit;
	int xvcount = meshData.heightMap.getWidth();
	int yvcount = meshData.heightMap.getHeight();

	updateCollisionShape();

	if (!pxHeightField)
		return NULL;
	return new PxHeightFieldGeometry(pxHeightField, PxMeshGeometryFlags(), height * scale.z() * 0.0001f,
		girds.y() * unit / (yvcount * sampleRate) * scale.y(), girds.x() * unit / (xvcount * sampleRate) * scale.x());
}
#endif

Serializable* TerrainGeometry::instantiate(const SerializationInfo& from)
{
	return new TerrainGeometry();
}
