#include "Mesh.h"
#include "Utility.h"
#include <fstream>
#ifdef PHYSICS_USE_PHYSX
#include "PhysicalWorld.h"
#include <NvClothExt/ClothFabricCooker.h>
#endif
#include "Importer.h"

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
}

MeshPart::MeshPart(const MeshPartDesc& desc)
{
	meshData = desc.meshData;
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

bool MeshPart::isSkeleton()
{
	return dynamic_cast<SkeletonMesh*>(mesh) != NULL;
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
	memcpy(part.element(0).data(), element(0).data(), sizeof(Vector3u) * elementCount);
	return part;
}

Mesh::Mesh() : Shape::Shape()
{
	
}

Mesh::Mesh(const Mesh & mesh)
{
	totalMeshPart = mesh.totalMeshPart;
	meshPartNames = mesh.meshPartNames;
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
	if (fabricMesh)
		fabricMesh->decRefCount();
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
	meshPartNames.insert(pair<string, int>(name, part.partIndex));
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
	meshPartNames = mesh.meshPartNames;
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
	//	desc.triangles.stride = sizeof(Matrix<unsigned int, 3, 1>);
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

PFabric * Mesh::generateCloth()
{
	if (fabricMesh == NULL) {
		nv::cloth::ClothMeshDesc meshDesc;
		meshDesc.setToDefault();
		meshDesc.points.count = totalMeshPart.vertexCount;
		meshDesc.points.stride = sizeof(Vector3f);
		meshDesc.points.data = totalMeshPart.vertex(0).data();

		meshDesc.triangles.count = totalMeshPart.elementCount;
		meshDesc.triangles.stride = sizeof(Matrix<unsigned int, 3, 1>);
		meshDesc.triangles.data = totalMeshPart.element(0).data();

		nv::cloth::Vector<int32_t>::Type phaseTypeInfo;
		fabricMesh = NvClothCookFabricFromMesh(PhysicalWorld::gNvClothFactory, meshDesc, { 0.0f, 0.0f, 1.0f }, &phaseTypeInfo);
	}
	return fabricMesh;
}

PFabric * Mesh::generateCloth(unsigned int partIndex)
{
	if (partIndex >= meshParts.size())
		return NULL;
	PFabric** fabricMeshPart;
	auto iter = fabricMeshParts.find(partIndex);
	if (iter == fabricMeshParts.end())
		fabricMeshPart = &fabricMeshParts.insert(make_pair((unsigned long long)partIndex, (PFabric*)NULL)).first->second;
	else
		fabricMeshPart = &iter->second;
	if (*fabricMeshPart == NULL) {
		MeshPart& meshPart = meshParts[partIndex];
		nv::cloth::ClothMeshDesc meshDesc;
		meshDesc.setToDefault();
		meshDesc.points.count = meshPart.vertexCount;
		meshDesc.points.stride = sizeof(Vector3f);
		meshDesc.points.data = meshPart.vertex(0).data();

		meshDesc.triangles.count = meshPart.elementCount;
		meshDesc.triangles.stride = sizeof(Matrix<unsigned int, 3, 1>);
		meshDesc.triangles.data = meshPart.element(0).data();

		nv::cloth::Vector<int32_t>::Type phaseTypeInfo;
		*fabricMeshPart = NvClothCookFabricFromMesh(PhysicalWorld::gNvClothFactory, meshDesc, { 0.0f, 0.0f, 1.0f }, &phaseTypeInfo);
	}
	return *fabricMeshPart;
}

PFabric * Mesh::generateCloth(const vector<unsigned int>& partIndexs)
{
	PFabric** fabricMeshPart;
	unsigned long long partIndex = meshPartIdHash(partIndexs);
	auto iter = fabricMeshParts.find(partIndex);
	if (iter == fabricMeshParts.end())
		fabricMeshPart = &fabricMeshParts.insert(make_pair(partIndex, (PFabric*)NULL)).first->second;
	else
		fabricMeshPart = &iter->second;

	if (*fabricMeshPart == NULL) {
		vector<Vector3f> verts;
		vector<Matrix<unsigned int, 3, 1>> elements;

		unsigned int vertCount = 0, elementCount = 0;
		for (int i = 0; i < partIndexs.size(); i++) {
			if (partIndexs[i] < meshParts.size()) {
				vertCount += meshParts[partIndexs[i]].vertexCount;
				elementCount += meshParts[partIndexs[i]].elementCount;
			}
		}
		verts.resize(vertCount); elements.resize(elementCount);
		vertCount = 0; elementCount = 0;
		for (int i = 0; i < partIndexs.size(); i++) {
			if (partIndexs[i] < meshParts.size()) {
				MeshPart& meshPart = meshParts[partIndexs[i]];
				memcpy(verts[vertCount].data(), meshPart.vertex(0).data(), sizeof(Vector3f) * meshPart.vertexCount);
				memcpy(elements[elementCount].data(), meshPart.element(0).data(), sizeof(Matrix<unsigned int, 3, 1>) * meshPart.elementCount);
				vertCount += meshPart.vertexCount;
				elementCount += meshPart.elementCount;
			}
		}

		nv::cloth::ClothMeshDesc meshDesc;
		meshDesc.setToDefault();
		meshDesc.points.count = vertCount;
		meshDesc.points.stride = sizeof(Vector3f);
		meshDesc.points.data = verts.data()->data();

		meshDesc.triangles.count = elementCount;
		meshDesc.triangles.stride = sizeof(Matrix<unsigned int, 3, 1>);
		meshDesc.triangles.data = elements.data()->data();

		nv::cloth::Vector<int32_t>::Type phaseTypeInfo;
		*fabricMeshPart = NvClothCookFabricFromMesh(PhysicalWorld::gNvClothFactory, meshDesc, { 0.0f, 0.0f, 1.0f }, &phaseTypeInfo);
	}
	return *fabricMeshPart;
}
#endif

Mesh & Mesh::operator=(Mesh & mesh)
{
	totalMeshPart = mesh.totalMeshPart;
	meshPartNames = mesh.meshPartNames;
	meshParts = mesh.meshParts;
	vertCount = mesh.vertCount;
	vertexPerFace = mesh.vertexPerFace;
	faceCount = mesh.faceCount;
	bound = mesh.bound;
	renderMode = mesh.renderMode;
	return *this;
}

#if ENABLE_PHYSICS
unsigned long long Mesh::meshPartIdHash(const vector<unsigned int>& partIndexs)
{
	vector<unsigned int> indexs = partIndexs;
	sort(indexs.begin(), indexs.end());
	unsigned long long hash = 0, factor = 1;
	for (int i = 0; i < indexs.size(); i++) {
		hash += indexs[i] * factor;
		factor *= 10;
	}
	return hash;
}
#endif