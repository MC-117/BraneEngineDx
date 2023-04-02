#include "Mesh.h"
#include "Utility/Utility.h"
#include <fstream>
#ifdef PHYSICS_USE_PHYSX
#include "Physics/PhysicalWorld.h"
#include <NvClothExt/ClothFabricCooker.h>
#endif
#include "Importer/AssimpImporter.h"

MeshPart::MeshPart()
{
}

MeshPart::MeshPart(const MeshPart & part)
{
	meshData = part.meshData;
	vertexFirst = part.vertexFirst;
	vertexCount = part.vertexCount;
	elementFirst = part.elementFirst;
	elementCount = part.elementCount;
	mesh = part.mesh;
	partIndex = part.partIndex;
	bound = part.bound;
}

MeshPart::MeshPart(const MeshPartDesc& desc)
{
	meshData = desc.meshData;
	vertexPerFace = desc.vertexPerFace;
	vertexFirst = desc.vertexFirst;
	vertexCount = desc.vertexCount;
	elementFirst = desc.elementFirst;
	elementCount = desc.elementCount;
}

MeshPart::MeshPart(MeshData * meshData, unsigned int vertexFirst, unsigned int vertexCount,
	unsigned int indexFirst, unsigned int indexCount)
	: meshData(meshData), vertexFirst(vertexFirst), vertexCount(vertexCount),
	elementFirst(indexFirst), elementCount(indexCount)
{
}

bool MeshPart::isValid()
{
	return meshData != NULL && meshData->isValid() && vertexCount != 0;
}

bool MeshPart::isSkeleton()
{
	return meshData->type == MeshType::MT_SkeletonMesh;
}

bool MeshPart::isMorph()
{
	SkeletonMesh* skm = dynamic_cast<SkeletonMesh*>(mesh);
	if (skm == NULL)
		return false;
	return skm->partHasMorph[partIndex];
}

MeshPart MeshPart::clone() const
{
	MeshPart part = VendorManager::getInstance().getVendor().newMeshPart(vertexCount, elementCount);
	memcpy(part.vertex(0).data(), vertex(0).data(), sizeof(Vector3f) * vertexCount);
	memcpy(part.normal(0).data(), normal(0).data(), sizeof(Vector3f) * vertexCount);
	memcpy(part.uv(0).data(), uv(0).data(), sizeof(Vector2f) * vertexCount);
	memcpy(&part.element(0), &element(0), sizeof(unsigned int) * elementCount);
	return part;
}

Mesh::Mesh() : Shape::Shape()
{
	
}

Mesh::Mesh(const Mesh & mesh)
{
	totalMeshPart = mesh.totalMeshPart;
	meshPartNameMap = mesh.meshPartNameMap;
	partNames = mesh.partNames;
	meshParts = mesh.meshParts;
	vertCount = mesh.vertCount;
	vertexPerFace = mesh.vertexPerFace;
	faceCount = mesh.faceCount;
	bound = mesh.bound;
	renderMode = mesh.renderMode;
}

Mesh::Mesh(int faceVertCount, int faceCount, float* vert, float* uv, float* norm)
{
	this->vertexPerFace = vertexPerFace;
	this->faceCount = faceCount;
	vertCount = faceCount * vertexPerFace;
	switch (vertexPerFace)
	{
	case 1:
		renderMode = STT_Point;
		break;
	case 2:
		renderMode = STT_Line;
		break;
	case 3:
		renderMode = STT_Triangle;
		break;
	case 4:
		renderMode = STT_Quad;
		break;
	default:
		break;
	}
}

Mesh::~Mesh()
{
#if ENABLE_PHYSICS
	if (collisionMesh)
		collisionMesh->release();
	for (int i = 0; i < fabricParts.size(); i++) {
		PFabric* fabric = fabricParts[i];
		if (fabric != NULL)
			fabricParts[i]->decRefCount();
	}
#endif
}

void Mesh::setTotalMeshPart(const MeshPart & part)
{
	totalMeshPart = part;
	totalMeshPart.mesh = this;
}

void Mesh::addMeshPart(const string& name, MeshPart & part)
{
	part.partIndex = meshParts.size();
	part.mesh = this;
	meshPartNameMap.insert(pair<string, int>(name, part.partIndex));
	partNames.push_back(name);
	meshParts.push_back(part);
}

void Mesh::resize(int vertexPerFace, int faceCount, int vertexCount)
{
	vertCount = vertexCount;
	this->vertexPerFace = vertexPerFace;
	this->faceCount = faceCount;
	switch (vertexPerFace)
	{
	case 1:
		renderMode = STT_Point;
		break;
	case 2:
		renderMode = STT_Line;
		break;
	case 3:
		renderMode = STT_Triangle;
		break;
	case 4:
		renderMode = STT_Quad;
		break;
	default:
		break;
	}
}

void Mesh::clone(const Mesh & mesh)
{
	meshPartNameMap = mesh.meshPartNameMap;
	partNames = mesh.partNames;
	meshParts = mesh.meshParts;
	vertCount = mesh.vertCount;
	vertexPerFace = mesh.vertexPerFace;
	faceCount = mesh.faceCount;
	bound = mesh.bound;
	renderMode = mesh.renderMode;
	totalMeshPart = mesh.totalMeshPart.clone();
	totalMeshPart.mesh = this;
	for (int i = 0; i < meshParts.size(); i++) {
		meshParts[i].mesh = this;
		meshParts[i].vertexFirst += totalMeshPart.vertexFirst - mesh.totalMeshPart.vertexFirst;
		meshParts[i].elementFirst += totalMeshPart.elementFirst - mesh.totalMeshPart.elementFirst;
	}
}

bool Mesh::writeObjStream(ostream& os, const vector<int>& partIndex) const
{
	vector<int> offsets;
	offsets.resize(partIndex.size());
	int count = 0;
	for (int index = 0; index < partIndex.size(); index++) {
		int i = partIndex[index];
		const SkeletonMeshPart& part = (const SkeletonMeshPart&)meshParts[i];
		for (int v = 0; v < part.vertexCount; v++) {
			string line = "v ";
			Vector3f vec = part.vertex(v);
			line += to_string(vec.x()) + " " + to_string(vec.y()) + " " + to_string(vec.z()) + "\n";
			os.write(line.c_str(), line.length());
		}
		offsets[i] = count;
		count += part.vertexCount;
	}
	for (int index = 0; index < partIndex.size(); index++) {
		int i = partIndex[index];
		const SkeletonMeshPart& part = (const SkeletonMeshPart&)meshParts[i];
		for (int v = 0; v < part.vertexCount; v++) {
			string line = "vt ";
			Vector2f vec = part.uv(v);
			line += to_string(vec.x()) + " " + to_string(vec.y()) + "\n";
			os.write(line.c_str(), line.length());
		}
	}
	for (int index = 0; index < partIndex.size(); index++) {
		int i = partIndex[index];
		const SkeletonMeshPart& part = (const SkeletonMeshPart&)meshParts[i];
		for (int v = 0; v < part.vertexCount; v++) {
			string line = "n ";
			Vector3f vec = part.normal(v);
			line += to_string(vec.x()) + " " + to_string(vec.y()) + " " + to_string(vec.z()) + "\n";
			os.write(line.c_str(), line.length());
		}
	}
	for (int index = 0; index < partIndex.size(); index++) {
		int i = partIndex[index];
		const SkeletonMeshPart& part = (const SkeletonMeshPart&)meshParts[i];
		string line = "g part" + to_string(i) + "\n";
		os.write(line.c_str(), line.length());
		for (int e = 0; e < part.elementCount; e += part.vertexPerFace) {
			string line = "f ";
			unsigned int vec0 = part.element(e);
			unsigned int vec1 = part.element(e + 1);
			unsigned int vec2 = part.element(e + 2);
			int offset = offsets[i];
			vec0 += offset;
			vec1 += offset;
			vec2 += offset;
			line += to_string(vec0) + "/" + to_string(vec0) + "/" + to_string(vec0) + " " +
				to_string(vec1) + "/" + to_string(vec1) + "/" + to_string(vec1) + " " +
				to_string(vec2) + "/" + to_string(vec2) + "/" + to_string(vec2) + "\n";
			os.write(line.c_str(), line.length());
		}
	}
	return true;
}

#if ENABLE_PHYSICS
CollisionShape * Mesh::generateComplexCollisionShape(const Vector3f& scale)
{
#ifdef PHYSICS_USE_BULLET
	if (vertexPerFace != 3)
		return Shape::generateCollisionShape();
	btTriangleMesh* m = new btTriangleMesh();
	for (int i = 0; i < vertCount; i++) {
		m->findOrAddVertex(toPVec3(totalMeshPart.vertex(i)), false);
	}
	for (int i = 0; i < faceCount; i++) {
		Matrix<unsigned int, 3, 1> f = totalMeshPart.element(i);
		m->addTriangleIndices(f.x(), f.y(), f.z());
	}
	btGImpactMeshShape * shape = new btGImpactMeshShape(m);
	shape->updateBound();
	return shape;
#endif
#ifdef PHYSICS_USE_PHYSX
	//if (vertexPerFace != 3)
	//	return Shape::generateCollisionShape();
	////PxCookingParams params(PhysicalWorld::gPhysicsSDK->getTolerancesScale());
	////// disable mesh cleaning - perform mesh validation on development configurations
	////params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
	////// disable edge precompute, edges are set for each triangle, slows contact generation
	////params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;

	////PhysicalWorld::gCooking->setParams(params);
	//if (collisionMesh == NULL) {
	//	PxTriangleMeshDesc desc;
	//	desc.points.count = totalMeshPart.vertexCount;
	//	desc.points.stride = sizeof(Vector3f);
	//	desc.points.data = totalMeshPart.vertex(0).data();

	//	desc.triangles.count = totalMeshPart.elementCount;
	//	desc.triangles.stride = sizeof(unsigned int);
	//	desc.triangles.data = totalMeshPart.element(0).data();/*
	//	bool res = PhysicalWorld::gCooking->validateTriangleMesh(desc);
	//	if (!res)
	//		return NULL;*/

	//	PxTriangleMesh* tri = PhysicalWorld::gCooking->createTriangleMesh(desc,
	//		PhysicalWorld::gPhysicsSDK->getPhysicsInsertionCallback());
	//	collisionMesh = tri;
	//	if (collisionMesh == NULL)
	//		return NULL;
	//}
	//PxTriangleMeshGeometry* shape = new PxTriangleMeshGeometry((PxTriangleMesh*)collisionMesh, toPVec3(scale));

	if (collisionMesh == NULL) {
		PxConvexMeshDesc convexDesc;
		convexDesc.points.count = totalMeshPart.vertexCount;
		convexDesc.points.stride = sizeof(Vector3f);
		convexDesc.points.data = totalMeshPart.vertex(0).data();
		convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

		PxConvexMesh* aConvexMesh = PhysicalWorld::gCooking->createConvexMesh(convexDesc,
			PhysicalWorld::gPhysicsSDK->getPhysicsInsertionCallback());
		collisionMesh = aConvexMesh;
		if (collisionMesh == NULL)
			return NULL;
	}

	PxConvexMeshGeometry* shape = new PxConvexMeshGeometry((PxConvexMesh*)collisionMesh, toPVec3(scale));

	return shape;
#endif
}

PFabric * Mesh::generateCloth(unsigned int partIndex)
{
	if (partIndex >= meshParts.size())
		return NULL;
	if (fabricParts.size() != meshParts.size())
		fabricParts.resize(meshParts.size(), NULL);
	PFabric*& fabricMeshPart = fabricParts[partIndex];
	if (fabricMeshPart == NULL) {
		MeshPart& meshPart = meshParts[partIndex];
		nv::cloth::ClothMeshDesc meshDesc;
		meshDesc.setToDefault();
		meshDesc.points.count = meshPart.vertexCount;
		meshDesc.points.stride = sizeof(Vector3f);
		meshDesc.points.data = meshPart.vertex(0).data();

		meshDesc.triangles.count = meshPart.elementCount;
		meshDesc.triangles.stride = sizeof(unsigned int);
		meshDesc.triangles.data = &meshPart.element(0);

		nv::cloth::Vector<int32_t>::Type phaseTypeInfo;
		fabricMeshPart = NvClothCookFabricFromMesh(PhysicalWorld::gNvClothFactory, meshDesc, { 0.0f, 0.0f, 1.0f }, &phaseTypeInfo);
	}
	return fabricMeshPart;
}
#endif

Mesh & Mesh::operator=(Mesh & mesh)
{
	totalMeshPart = mesh.totalMeshPart;
	meshPartNameMap = mesh.meshPartNameMap;
	meshParts = mesh.meshParts;
	vertCount = mesh.vertCount;
	vertexPerFace = mesh.vertexPerFace;
	faceCount = mesh.faceCount;
	bound = mesh.bound;
	renderMode = mesh.renderMode;
#if ENABLE_PHYSICS
	collisionMesh = mesh.collisionMesh;
	fabricParts = mesh.fabricParts;
#endif
	return *this;
}