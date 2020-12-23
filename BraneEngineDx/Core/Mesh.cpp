#include "Mesh.h"
#include "Utility.h"
#include <fstream>
#ifdef PHYSICS_USE_BULLET
#include <BulletCollision\Gimpact\btGImpactShape.h>
#include <BulletCollision\Gimpact\btTriangleShapeEx.h>
#endif
#ifdef PHYSICS_USE_PHYSX
#include "PhysicalWorld.h"
#include <NvClothExt/ClothFabricCooker.h>
#endif
#include "Importer.h"

set<MeshData*> MeshData::MeshDataCollection;
MeshData MeshData::StaticMeshData;

unsigned int MeshData::currentVao = NULL;

MeshData::MeshData()
{
	MeshDataCollection.insert(this);
}

unsigned int MeshData::getVAO() const
{
	return vao;
}

unsigned int MeshData::bindShape()
{
	if (vao != 0 && vao == currentVao)
		return vao;
	glBindVertexArray(0);
	if (vao == NULL || vbo == NULL || (ubo == NULL && uvs.size() != 0) || (nbo == NULL && normals.size() != 0) || ibo == NULL) {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size() * 3, vertices.begin()->data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		if (uvs.size() != 0) {
			glGenBuffers(1, &ubo);
			glBindBuffer(GL_ARRAY_BUFFER, ubo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * uvs.size() * 2, uvs.begin()->data(), GL_STATIC_DRAW);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
		}

		if (normals.size() != 0) {
			glGenBuffers(1, &nbo);
			glBindBuffer(GL_ARRAY_BUFFER, nbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normals.size() * 3, normals.begin()->data(), GL_STATIC_DRAW);
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		}

		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * elements.size() * 3, elements.begin()->data(), GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	/*else {
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * vertices.size() * 3, vertices.begin()->data());

		if (uvs.size() != 0) {
			glBindBuffer(GL_ARRAY_BUFFER, ubo);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * uvs.size() * 2, uvs.begin()->data());
		}

		if (normals.size() != 0) {
			glBindBuffer(GL_ARRAY_BUFFER, nbo);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * normals.size() * 3, normals.begin()->data());
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}*/
	glBindVertexArray(vao);
	return vao;
}

bool MeshData::isGenerated()
{
	return vao != NULL;
}

bool MeshData::isType(const type_info & type)
{
	return type.hash_code() == id;
}

MeshPart MeshData::newMeshPart(unsigned int vertCount, unsigned int elementCount)
{
	MeshData* data = NULL;
	if (StaticMeshData.isGenerated()) {
		TypeID id = typeid(MeshData).hash_code();
		for (auto b = MeshDataCollection.begin(), e = MeshDataCollection.end(); b != e; b++) {
			if ((*b)->id == id && !(*b)->isGenerated()) {
				data = *b;
				break;
			}
		}
	}
	else
		data = &StaticMeshData;
	if (data == NULL) {
		data = new MeshData();
	}
	MeshPart part = { data, (unsigned int)data->vertices.size(), vertCount, (unsigned int)data->elements.size(), elementCount };
	data->vertices.resize(data->vertices.size() + vertCount);
	data->uvs.resize(data->uvs.size() + vertCount);
	data->normals.resize(data->normals.size() + vertCount);
	data->elements.resize(data->elements.size() + elementCount);
	return part;
}

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

unsigned int MeshPart::bindShape()
{
	if (meshData == NULL)
		return 0;
	return meshData->bindShape();
}

MeshPart MeshPart::clone() const
{
	MeshPart part = MeshData::newMeshPart(vertexCount, elementCount);
	memcpy(part.vertex(0).data(), vertex(0).data(), sizeof(Vector3f) * vertexCount);
	memcpy(part.normal(0).data(), normal(0).data(), sizeof(Vector3f) * vertexCount);
	memcpy(part.uv(0).data(), uv(0).data(), sizeof(Vector2f) * vertexCount);
	memcpy(part.element(0).data(), element(0).data(), sizeof(Matrix<unsigned int, 3, 1>) * elementCount);
	return part;
}

DrawElementsIndirectCommand MeshPart::drawCommand(int instanceCount, int baseInstance)
{
	return { (int)elementCount * 3, instanceCount, (int)elementFirst * 3, (int)vertexFirst, baseInstance };
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
		renderMode = GL_POINTS;
		break;
	case 2:
		renderMode = GL_LINES;
		break;
	case 3:
		renderMode = GL_TRIANGLES;
		break;
	case 4:
		renderMode = GL_QUADS;
		break;
	default:
		break;
	}
}

Mesh::~Mesh()
{
	if (collisionMesh)
		collisionMesh->release();
	if (fabricMesh)
		fabricMesh->decRefCount();
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
		renderMode = GL_POINTS;
		break;
	case 2:
		renderMode = GL_LINES;
		break;
	case 3:
		renderMode = GL_TRIANGLES;
		break;
	case 4:
		renderMode = GL_QUADS;
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

unsigned int Mesh::bindShape()
{
	int i = totalMeshPart.bindShape();
	return i;
}

void Mesh::drawCall()
{
	DrawElementsIndirectCommand cmd = totalMeshPart.drawCommand(1, 0);
	glDrawArraysIndirect(renderMode, &cmd);
}

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

bool Mesh::ObjReader::load(string path, Mesh & mesh)
{
	Importer imp = Importer(path, aiProcessPreset_TargetRealtime_MaxQuality);
	return imp.getMesh(mesh);
	/*string line;
	vector<vector<float>> vert;
	vector<vector<float>> uv;
	vector<vector<float>> norm;
	vector<vector<unsigned int>> index;
	map<string, MeshPart> meshParts;
	MeshPart* currentMeshPart = NULL;
	int vertPerFace = 0;
	fstream f;
	f.open(path, ios::in);
	if (!f.is_open()) {
		cout << "Something Went Wrong When Opening Objfiles" << endl;
		return false;
	}

	while (getline(f, line)) {
		if (line.empty())
			continue;
		vector<string> parameters = split(line, ' ');
		if (parameters.size() == 0)
			continue;
		else if (parameters[0][0] == '#') {

		}
		else if (parameters[0] == "v") {
			if (parameters.size() != 4) {
				cout << "This obj file is broken" << endl;
				return false;
			}
			vector<GLfloat> Point;
			for (int i = 1; i < 4; i++) {
				GLfloat xyz = atof(parameters[i].c_str());
				Point.push_back(xyz);
			}
			vert.push_back(Point);
		}
		else if (parameters[0] == "vt") {
			if (parameters.size() != 3) {
				cout << "This obj file is broken" << endl;
				return false;
			}
			vector<GLfloat>Point;
			for (int i = 1; i < 3; i++) {
				GLfloat xy = atof(parameters[i].c_str());
				Point.push_back(xy);
			}
			uv.push_back(Point);
		}
		else if (parameters[0] == "vn") {
			if (parameters.size() != 4) {
				cout << "This obj file is broken" << endl;
				return false;
			}
			vector<GLfloat>Point;
			for (int i = 1; i < 4; i++) {
				GLfloat xyz = atof(parameters[i].c_str());
				Point.push_back(xyz);
			}
			norm.push_back(Point);
		}
		else if (parameters[0] == "f") {
			if (parameters.size() < 3) {
				cout << "This obj file is broken" << endl;
				return false;
			}
			if (vertPerFace == 0)
				vertPerFace = parameters.size() - 1;
			if (currentMeshPart == NULL) {
				currentMeshPart = &meshParts.insert(pair<string, MeshPart>("Default", { 0, (unsigned int)(index.size() * vertPerFace), 0 })).first->second;
			}
			for (int i = 1; i < parameters.size(); i++) {
				vector<string> info = split(parameters[i], '/');
				vector<unsigned int> indexSet;
				if (info.size() != 3) {
					cout << "This obj file is broken" << endl;
					return false;
				}
				for (int j = 0; j < info.size(); j++)
					indexSet.push_back(atoi(info[j].c_str()) - 1);
				index.push_back(indexSet);
			}
		}
		else if (parameters[0] == "usemtl") {
			if (parameters.size() != 2) {
				cout << "This obj file is broken" << endl;
				return false;
			}
			if (currentMeshPart != NULL) {
				currentMeshPart->vertexCount = index.size() * vertPerFace - currentMeshPart->vertexFirst;
			}
			currentMeshPart = &meshParts.insert(pair<string, MeshPart>(parameters[1], { 0, (unsigned int)(index.size() * vertPerFace), 0 })).first->second;
		}
	}
	f.close();
	if (currentMeshPart != NULL) {
		currentMeshPart->vertexCount = index.size() * vertPerFace - currentMeshPart->vertexFirst;
	}
	MeshPart totalPart = MeshData::newMeshPart(index.size() * vertPerFace);
	mesh.totalMeshPart = totalPart;
	for (auto b = meshParts.begin(), e = meshParts.end(); b != e; b++) {
		b->second.vertexFirst += totalPart.vertexFirst;
		b->second.meshData = totalPart.meshData;
		mesh.addMeshPart(b->first, b->second);
	}
	mesh.resize(vertPerFace, index.size());
	Matrix<float, 3, 2> bound;
	bound << FLT_MAX, FLT_MIN,
		FLT_MAX, FLT_MIN,
		FLT_MAX, FLT_MIN;
	for (int i = 0; i < index.size(); i++) {
		for (int j = 0; j < 3; j++) {
			float t = vert[index[i][0]][j];
			if (t < bound(j, 0)) {
				if (bound(j, 0) != FLT_MAX && bound(j, 0) > bound(j, 1))
					bound(j, 1) = bound(j, 0);
				bound(j, 0) = t;
			}
			else if (t > bound(j, 1))
				bound(j, 1) = t;
			totalPart.vertex(i)(j) = t;
		}
		if (uv.size() != 0) {
			totalPart.uv(i)(0) = uv[index[i][1]][0];
			totalPart.uv(i)(1) = uv[index[i][1]][1];
		}
		if (norm.size() != 0) {
			totalPart.normal(i)(0) = norm[index[i][2]][0];
			totalPart.normal(i)(1) = norm[index[i][2]][1];
			totalPart.normal(i)(2) = norm[index[i][2]][2];
		}
	}
	mesh.bound = bound;
	return true;*/
}