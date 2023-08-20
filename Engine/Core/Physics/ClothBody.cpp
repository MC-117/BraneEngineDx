#include "ClothBody.h"
#include "PhysicalWorld.h"
#include <NvClothExt/ClothMeshDesc.h>
#include <NvClothExt/ClothFabricCooker.h>

ClothBodyCollider::ClothBodyCollider(Shape* shape, ShapeComplexType shapeComplexType, Vector3f scale)
	: PhysicalCollider(shape, shapeComplexType, scale)
{
	for (int i = 0; i < 6; i++) {
		index.box.planeIndex[i] = -1;
	}
	index.box.index = -1;
}

ClothBodyCollider::~ClothBodyCollider()
{
}

void ClothBodyCollider::setPositionOffset(const Vector3f& offset)
{
	if (positionOffset == offset)
		return;
	positionOffset = offset;
	apply();
}

void ClothBodyCollider::setRotationOffset(const Quaternionf& offset)
{
	if (rotationOffset == offset)
		return;
	rotationOffset = offset;
	apply();
}

void ClothBodyCollider::setSphere(PCloth& cloth)
{
	if (index.sphere.index == -1) {
		index.sphere.index = cloth.getNumSpheres();
	}
	Vector3f pos = (shape->getCenter() + positionOffset).cwiseProduct(localScale);
	float radius = shape->getRadius() * localScale.x();
	PxVec4 sphere = { pos.x(), pos.y(), pos.z(), radius };
	cloth.setSpheres({ &sphere, &sphere + 1 }, index.sphere.index, index.sphere.index);
}

void ClothBodyCollider::setCapsule(PCloth& cloth)
{
	if (index.capsule.sphereIndex[0] == -1) {
		index.capsule.sphereIndex[0] = cloth.getNumSpheres();
	}
	if (index.capsule.sphereIndex[1] == -1) {
		index.capsule.sphereIndex[1] = cloth.getNumSpheres() + 1;
	}
	if (index.capsule.index == -1) {
		index.capsule.index = cloth.getNumCapsules();
	}
	Vector3f center = (shape->getCenter() + positionOffset).cwiseProduct(localScale);
	float radius = shape->getRadius();
	float halfLength = shape->getHeight() * 0.5 - radius;
	Vector3f zVec = rotationOffset * Vector3f::UnitZ() * halfLength;
	Vector4f sphere[2] = {
		{ center.x() + zVec.x(), center.y() + zVec.x(), center.z() + zVec.x(), 1 },
		{ center.x() - zVec.x(), center.y() - zVec.x(), center.z() - zVec.x(), 1 }
	};

	cloth.setSpheres({ (PxVec4*)sphere, (PxVec4*)sphere + 1 }, index.capsule.sphereIndex[0], index.capsule.sphereIndex[0]);
	cloth.setSpheres({ (PxVec4*)sphere + 1, (PxVec4*)sphere + 2 }, index.capsule.sphereIndex[1], index.capsule.sphereIndex[1]);

	cloth.setCapsules({ index.capsule.sphereIndex, index.capsule.sphereIndex + 2 },
		index.capsule.index, index.capsule.index);
}

void ClothBodyCollider::setBox(PCloth& cloth)
{
	for (int i = 0; i < 1; i++) {
		if (index.box.planeIndex[i] == -1) {
			index.box.planeIndex[i] = cloth.getNumPlanes() + i;
		}
	}
	if (index.box.index == -1) {
		index.box.index = cloth.getNumConvexes();
	}

	Vector3f center = (shape->getCenter() + positionOffset).cwiseProduct(localScale);

	Vector3f extent = shape->getExtent().cwiseProduct(localScale);

	Vector3f xVec = rotationOffset * Vector3f::UnitX();
	Vector3f yVec = rotationOffset * Vector3f::UnitY();
	Vector3f zVec = rotationOffset * Vector3f::UnitZ();

	Vector4f planes[6];
	planes[0].block(0, 0, 3, 1) = zVec;
	planes[0].w() = (center + zVec * extent.z()).dot(zVec);
	planes[1].block(0, 0, 3, 1) = -zVec;
	planes[1].w() = (center - zVec * extent.z()).dot(-zVec);
	planes[2].block(0, 0, 3, 1) = yVec;
	planes[2].w() = (center + yVec * extent.y()).dot(yVec);
	planes[3].block(0, 0, 3, 1) = -yVec;
	planes[3].w() = (center - yVec * extent.y()).dot(-yVec);
	planes[4].block(0, 0, 3, 1) = xVec;
	planes[4].w() = (center + xVec * extent.x()).dot(xVec);
	planes[5].block(0, 0, 3, 1) = -xVec;
	planes[5].w() = (center - xVec * extent.x()).dot(-xVec);

	unsigned int mask = 0;
	for (int i = 0; i < 1; i++) {
		int pindex = index.box.planeIndex[i];
		cloth.setPlanes({ (PxVec4*)planes + i, (PxVec4*)planes + i + 1 }, pindex, pindex);
		mask |= 1 << pindex;
	}

	cloth.setConvexes({ &mask, &mask + 1 },
		index.box.index, index.box.index);
}

void ClothBodyCollider::setRawShape(PCloth& cloth)
{
	Sphere* sphere = dynamic_cast<Sphere*>(shape);
	if (sphere) {
		setSphere(cloth);
		return;
	}
	Capsule* capsule = dynamic_cast<Capsule*>(shape);
	if (capsule) {
		setCapsule(cloth);
		return;
	}
	Box* box = dynamic_cast<Box*>(shape);
	if (box) {
		setBox(cloth);
		return;
	}
}

void ClothBodyCollider::apply()
{
	ClothBody* clothBody = dynamic_cast<ClothBody*>(body);
	if (clothBody == NULL)
		return;
	for (int i = 0; i < clothBody->rawClothBodies.size(); i++) {
		setRawShape(*clothBody->rawClothBodies[i]);
	}
}

ClothVertexCluster::ClothVertexCluster(const Vector3f& positionOffset, const Quaternionf& rotationOffset)
{
	setReferencePose(positionOffset, rotationOffset);
}

void ClothVertexCluster::addSphere(const Vector4f& sphere)
{
	spheres.push_back(sphere);
}

void ClothVertexCluster::pickVertex()
{
	if (mesh == NULL)
		return;
	set<int> indexSet;
	for (int i = 0; i < spheres.size(); i++) {
		Vector4f sphere = spheres[i];
		float radius = sphere.w();
		sphere.w() = 1;
		Vector3f pos = (transformMat * sphere).block(0, 0, 3, 1);
		for (int j = 0; j < mesh->vertices.size(); j++) {
			Vector3f vertex = mesh->vertices[j];
			if ((pos - vertex).norm() <= radius)
				indexSet.insert(j);
		}
	}

	indices.resize(indexSet.size());
	vertices.resize(indexSet.size());
	int i = 0;
	for (auto b = indexSet.begin(), e = indexSet.end(); b != e; b++, i++) {
		indices[i] = *b;
		vertices[i] = mesh->vertices[*b];
		mesh->invMass[*b] = 0;
	}
	needUpdate = true;
}

void ClothVertexCluster::updateVertex()
{
	if (mesh == NULL || !needUpdate)
		return;
	for (int i = 0; i < indices.size(); i++) {
		int index = indices[i];
		Vector3f& target = mesh->vertices[index];
		Vector3f source = vertices[i];
		target = ((transformMat * bindedPose) *
			Vector4f(source.x(), source.y(), source.z(), 1)).block(0, 0, 3, 1);
	}
	needUpdate = false;
}

void ClothVertexCluster::setReferencePose(const Vector3f& positionOffset, const Quaternionf& rotationOffset)
{
	Matrix4f T = Matrix4f::Identity();
	T(0, 3) = positionOffset.x();
	T(1, 3) = positionOffset.y();
	T(2, 3) = positionOffset.z();
	Matrix4f R = Matrix4f::Identity();
	R.block(0, 0, 3, 3) = rotationOffset.toRotationMatrix();
	transformMat = T * R;
	bindedPose = transformMat.inverse();
}

Vector3f ClothVertexCluster::getVertexReferencePosition(int indexOfIndex) const
{
	return vertices[indexOfIndex];
}

int ClothVertexCluster::getVertexCount() const
{
	return indices.size();
}

Vector3f ClothVertexCluster::getVertexPosition(int indexOfIndex) const
{
	return mesh->vertices[indices[indexOfIndex]];
}

ClothBody::ClothBody(::Transform & targetTransform, const PhysicalMaterial & material)
	: PhysicalBody(targetTransform, material)
{
	bodyType = SOFT;
}

ClothBody::~ClothBody()
{
	for (int i = 0; i < clusters.size(); i++) {
		delete clusters[i];
	}
	for (int i = 0; i < rawClothFabrics.size(); i++) {
		rawClothFabrics[i]->decRefCount();
	}
	for (int i = 0; i < rawClothBodies.size(); i++) {
		NV_CLOTH_DELETE(rawClothBodies[i]);
	}
	if (clothSolver != NULL)
		NV_CLOTH_DELETE(clothSolver);
}

void ClothBody::addMeshPart(const string& name, const MeshPart& meshPart)
{
	meshData.addMeshPart(name, meshPart);
}

ClothVertexCluster* ClothBody::addVertexCluster(const Vector3f& position, const Quaternionf& rotation)
{
	ClothVertexCluster* cluster = new ClothVertexCluster(position, rotation);
	cluster->mesh = &meshData;
	clusters.push_back(cluster);
	return cluster;
}

bool ClothBody::isValid() const
{
	return !meshData.sourceMeshParts.empty();
}

ClothVertexCluster* ClothBody::getVertexCluster(int index)
{
	if (index >= clusters.size())
		return NULL;
	return clusters[index];
}

Mesh& ClothBody::getMesh()
{
	return meshData.mesh;
}

const ClothMeshData& ClothBody::getMeshData() const
{
	return meshData;
}

int ClothBody::getVertexClusterCount() const
{
	return clusters.size();
}

void ClothBody::initBody()
{
	meshData.apply();
	for (int i = 0; i < clusters.size(); i++) {
		ClothVertexCluster* cluster = clusters[i];
		cluster->pickVertex();
	}
	int count = meshData.clothParts.size();
	rawClothFabrics.resize(count);
	rawClothBodies.resize(count);

	Vector3f position = targetTransform.getPosition(WORLD);
	Quaternionf rotation = targetTransform.getRotation(WORLD);
	for (int i = 0; i < count; i++) {
		MeshPart& meshPart = meshData.clothParts[i];
		PCloth*& rawClothBody = rawClothBodies[i];
		PFabric*& fabric = rawClothFabrics[i];

		nv::cloth::ClothMeshDesc meshDesc;
		meshDesc.setToDefault();
		meshDesc.points.count = meshPart.vertexCount;
		meshDesc.points.stride = sizeof(Vector3f);
		meshDesc.points.data = meshPart.vertex(0).data();

		meshDesc.triangles.count = meshPart.elementCount / 3;
		meshDesc.triangles.stride = sizeof(unsigned int) * 3;
		meshDesc.triangles.data = &meshPart.element(0);

		nv::cloth::Vector<int32_t>::Type phaseTypeInfo;
		fabric = NvClothCookFabricFromMesh(PhysicalWorld::gNvClothFactory, meshDesc, { 0.0f, 0.0f, -10.0f }, &phaseTypeInfo);

		vector<Vector4f> clothParticles;
		clothParticles.resize(meshPart.vertexCount);
		for (int v = 0; v < meshPart.vertexCount; v++) {
			Vector4f& position = clothParticles[v];
			position.block(0, 0, 3, 1) = meshPart.vertex(v);
			position.w() = meshData.getMass(meshPart, v);
		}
		rawClothBody = PhysicalWorld::gNvClothFactory->createCloth(
			{ (PxVec4*)clothParticles.data(), (PxVec4*)clothParticles.data() + meshPart.vertexCount }, *fabric);
		nv::cloth::PhaseConfig* phases = new nv::cloth::PhaseConfig[fabric->getNumPhases()];
		for (int p = 0; p < fabric->getNumPhases(); p++)
		{
			phases[p].mPhaseIndex = p; // Set index to the corresponding set (constraint group)

									   //For this example we give very phase the same config
			phases[p].mStiffness = 1.0f;
			phases[p].mStiffnessMultiplier = 1.0f;
			phases[p].mCompressionLimit = 1.0f;
			phases[p].mStretchLimit = 1.0f;
		}
		rawClothBody->setPhaseConfig({ phases, phases + fabric->getNumPhases() });
		delete[] phases;

		rawClothBody->setGravity(physicalWorld->physicsScene->getGravity());

		rawClothBody->setTranslation(toPVec3(position));
		rawClothBody->setRotation(toPQuat(rotation));
		rawClothBody->clearInertia();
	}
}

PhysicalCollider* ClothBody::addCollider(Shape* shape, ShapeComplexType shapeComplexType)
{
	ClothBodyCollider* collider = new ClothBodyCollider(shape, shapeComplexType, targetTransform.getScale());
	collider->body = this;
	colliders.push_back(collider);
	return collider;
}

void ClothBody::updateObjectTransform()
{
	for (int i = 0; i < clusters.size(); i++) {
		ClothVertexCluster* cluster = clusters[i];
		cluster->updateVertex();
	}
	for (int i = 0; i < rawClothBodies.size(); i++) {
		MeshPart& clothPart = meshData.clothParts[i];
		PCloth* rawClothBody = rawClothBodies[i];
		auto clothParticles = rawClothBody->getCurrentParticles();
		int vertexBase = clothPart.vertexFirst;
		for (int v = 0; v < clothPart.vertexCount; v++) {
			int vertexIndex = vertexBase + v;
			PxVec4& simP = clothParticles[v];
			Vector3f& dataP = meshData.vertices[vertexIndex];
			float invMass = meshData.invMass[vertexIndex];
			if (invMass == 0) {
				memcpy(&simP, &dataP, sizeof(Vector3f));
				simP.w = 0;
			}
			else {
				dataP = toVector3f(simP.getXYZ());
			}
			meshData.normals[vertexIndex] = Vector3f(0, 0, 0);
		}
		for (int e = 0; e < clothPart.elementCount; e += 3) {
			unsigned int* element = &clothPart.element(e);
			const auto p0 = clothPart.vertex(element[0]);
			const auto p1 = clothPart.vertex(element[1]);
			const auto p2 = clothPart.vertex(element[2]);

			const auto normal = ((p2 - p0).cross(p1 - p0)).normalized();

			clothPart.normal(element[0]) += normal;
			clothPart.normal(element[1]) += normal;
			clothPart.normal(element[2]) += normal;
		}
		for (int v = 0; v < clothPart.vertexCount; v++) {
			clothPart.normal(v).normalize();
		}
		rawClothBody->setGravity(physicalWorld->physicsScene->getGravity());
		rawClothBody->setTranslation(toPVec3(targetTransform.getPosition(WORLD)));
		rawClothBody->setRotation(toPQuat(targetTransform.getRotation(WORLD)));
	}
	meshData.updateVertex();
	meshData.updateNormal();
	for (int i = 0; i < colliders.size(); i++) {
		ClothBodyCollider* collider = (ClothBodyCollider*)colliders[i];
		for (int j = 0; j < rawClothBodies.size(); j++) {
			PCloth* rawClothBody = rawClothBodies[j];
			collider->setRawShape(*rawClothBody);
		}
	}
}

void ClothBody::addToWorld(PhysicalWorld & physicalWorld)
{
	if (!isValid() || this->physicalWorld != NULL)
		return;
	PhysicalBody::addToWorld(physicalWorld);
	this->physicalWorld = &physicalWorld;
	initBody();
	clothSolver = PhysicalWorld::gNvClothFactory->createSolver();
	for (int i = 0; i < rawClothBodies.size(); i++) {
		clothSolver->addCloth(rawClothBodies[i]);
	}
	physicalWorld.clothSolvers.insert(clothSolver);
}

void ClothBody::removeFromWorld()
{
	if (physicalWorld != NULL) {
		physicalWorld->clothSolvers.erase(clothSolver);
		for (int i = 0; i < rawClothBodies.size(); i++) {
			clothSolver->removeCloth(rawClothBodies[i]);
		}
	}
}

void * ClothBody::getSoftObject() const
{
	return rawClothBodies.empty() ? NULL : rawClothBodies[0];
}
