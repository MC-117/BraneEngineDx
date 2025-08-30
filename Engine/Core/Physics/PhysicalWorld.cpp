#include "PhysicalWorld.h"
#include "../Utility/Utility.h"
#include "../Engine.h"

bool PhysicalWorld::isInitSDK = false;

#define PX_USE_GPU 0

#ifdef PHYSICS_USE_PHYSX
PxPhysics* PhysicalWorld::gPhysicsSDK = NULL;
PxBEErrorCallback PhysicalWorld::gDefaultErrorCallback;
PxDefaultAllocator PhysicalWorld::gDefaultAllocatorCallback;
PxSimulationFilterShader PhysicalWorld::gDefaultFilterShader = PhysicalLayer::SimulationFilterShader;
PxFoundation* PhysicalWorld::gFoundation = NULL;
PxPvd* PhysicalWorld::gPvd = NULL;
PxCooking* PhysicalWorld::gCooking = NULL;
PxDefaultCpuDispatcher* PhysicalWorld::gDispatcher = NULL;
PxCudaContextManager* PhysicalWorld::gCudaContextManager = NULL;
nv::cloth::Factory* PhysicalWorld::gNvClothFactory = NULL;
#endif // !PHYSICS_USE_PHYSX

PhysicalWorld::PhysicalWorld()
	: accumulator(0)
	, stepSize(1 / 60.0f)
#ifdef PHYSICS_USE_BULLET
	, collisionConfiguration(), dispatcher(&collisionConfiguration), overlappingPairCache(), solver(),
	physicsScene(new btDiscreteDynamicsWorld(&dispatcher, &overlappingPairCache, &solver, &collisionConfiguration))
{
	init();
	physicsScene->setDebugDrawer(&db);
	db.setDebugMode(btIDebugDraw::DBG_DrawAabb);
	physicsScene->getSolverInfo().m_minimumSolverBatchSize = 128;
	physicsScene->getSolverInfo().m_globalCfm = 0.00001;
}
#endif // !PHYSICS_USE_BULLET

#ifdef PHYSICS_USE_PHYSX
{
	init();
	PxSceneDesc sceneDesc(gPhysicsSDK->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = gDefaultFilterShader;
	sceneDesc.flags |= PxSceneFlag::eENABLE_PCM;
	sceneDesc.flags |= PxSceneFlag::eENABLE_ACTIVE_ACTORS;
	sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;
	if (gCudaContextManager != NULL) {
		sceneDesc.cudaContextManager = gCudaContextManager;
		sceneDesc.flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS;
		sceneDesc.broadPhaseType = PxBroadPhaseType::eGPU;
	}

	physicsScene = gPhysicsSDK->createScene(sceneDesc);
	physicsScene->setContactModifyCallback(this);
	physicsScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
	physicsScene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 1.0f);
	physicsScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
	physicsScene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS, 1.0f);
	physicsScene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.0f);

	PxPvdSceneClient* pvdClient = physicsScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	controllerManager = PxCreateControllerManager(*physicsScene);
	//controllerManager->setDebugRenderingFlags(PxControllerDebugRenderFlag::eALL);
}
#endif // !PHYSICS_USE_PHYSX

PhysicalWorld::~PhysicalWorld()
{
#ifdef PHYSICS_USE_BULLET
	if (physicsScene != NULL)
		delete physicsScene;
#endif // !PHYSICS_USE_BULLET
#ifdef PHYSICS_USE_PHYSX
	//NV_CLOTH_DELETE(clothSolver);
	if (physicsScene != NULL) {
		physicsScene->fetchResults(true);
		physicsScene->release();
	}
#endif // !PHYSICS_USE_PHYSX
}

bool PhysicalWorld::getPause() const
{
	return pause;
}

void PhysicalWorld::setPause(bool value)
{
	pause = value;
}

void PhysicalWorld::addPhysicalBody(PhysicalBody & body)
{
	body.addToWorld(*this);
}

void PhysicalWorld::removePhysicalBody(PhysicalBody & body)
{
	if (body.physicalWorld == this)
		body.removeFromWorld();
}

void PhysicalWorld::addPhysicalConstraint(PhysicalConstraint& constraint)
{
	constraint.addToWorld(*this);
}

void PhysicalWorld::removePhysicalConstraint(PhysicalConstraint& constraint)
{
	if (constraint.physicalWorld == this)
		constraint.removeFromWorld();
}

Vector3f PhysicalWorld::getGravity()
{
	return toVector3f(physicsScene->getGravity());
}

void PhysicalWorld::setGravity(const Vector3f& g)
{
	physicsScene->setGravity(toPVec3(g));
}

#ifdef PHYSICS_USE_BULLET
void PhysicalWorld::collisionDispatch()
{
	int numManifolds = physicsScene->getDispatcher()->getNumManifolds();

	for (int i = 0; i<numManifolds; i++)
	{
		btPersistentManifold* contactManifold = physicsScene->getDispatcher()->getManifoldByIndexInternal(i);
		PhysicalBody* objA = dynamic_cast<PhysicalBody*>((CollisionObject*)(contactManifold->getBody0()));
		PhysicalBody* objB = dynamic_cast<PhysicalBody*>((CollisionObject*)(contactManifold->getBody1()));

		int numContacts = contactManifold->getNumContacts();
		for (int j = 0; j<numContacts; j++)
		{
			btManifoldPoint& pt = contactManifold->getContactPoint(j);
			if (pt.getDistance() <= 0.f)
			{
				if (objB != NULL) {
					ContactInfo info;
					info.otherObject = &objA->targetTransform;
					info.location = toVector3f(pt.getPositionWorldOnB());
					info.normal = toVector3f(pt.m_normalWorldOnB);
					objB->handleCollision(info);
				}
			}
		}
	}
}
#endif

#ifdef PHYSICS_USE_PHYSX
struct NvClothChunkTask
{
	nv::cloth::Solver* solver = NULL;
	int taskID = -1;
	bool done = false;
	thread* taskThread = NULL;

	NvClothChunkTask(nv::cloth::Solver* solver, int taskID)
		: solver(solver), taskID(taskID)
	{
	}

	~NvClothChunkTask()
	{
		if (taskThread != NULL)
			delete taskThread;
	}

	void doTask()
	{
		if (solver != NULL && taskID >= 0)
			solver->simulateChunk(taskID);
		done = true;
	}

	void start()
	{
		taskThread = new thread([](NvClothChunkTask* t) {
			t->doTask();
		}, this);
		taskThread->detach();
	}
};
#endif

void PhysicalWorld::updatePhysicalWorld(float deltaTime)
{
	if (pause) {
		return;
	}
#ifndef PHYSICS_USE_PHYSX5
	if (isInit)
		physicsScene->fetchResults(true);
#endif
	isInit = true;
	dirty = false;
#ifdef PHYSICS_USE_BULLET
	for (int i = 0; i < physicsScene->getCollisionObjectArray().size(); i++) {
		PhysicalBody* obj = dynamic_cast<PhysicalBody*>(physicsScene->getCollisionObjectArray()[i]);
		obj->onStepSimulation(obj, this);
		if (obj != NULL && obj->material.physicalType == PhysicalType::STATIC) {
			obj->initBody();
		}
		/*else {
			RigidBody* ro = dynamic_cast<RigidBody*>(obj);
			if (ro != NULL && obj->material.physicalType == PhysicalType::DYNAMIC)
			ro->activate();
		}*/
	}
	physicsScene->stepSimulation(deltaTime, 2);
#endif
#ifdef PHYSICS_USE_PHYSX
	float stepSize = this->stepSize;
	accumulator += deltaTime;
	if (accumulator > 0 && accumulator < stepSize) {
		onStepSimulation(this, accumulator);
		physicsScene->simulate(accumulator);
#ifdef PHYSICS_USE_PHYSX5
		physicsScene->fetchResults(true);
#endif
		accumulator = 0;
	}
	else {
		bool simulate = true;
		int maxStep = 5, step = 0;
		while (accumulator >= stepSize) {
			if (step >= maxStep) {
				accumulator = 0;
				break;
			}
#ifndef PHYSICS_USE_PHYSX5
			if (step > 0)
				physicsScene->fetchResults(true);
#endif
			step++;
			accumulator -= stepSize;
			onStepSimulation(this, stepSize);
			physicsScene->simulate(stepSize);
#ifdef PHYSICS_USE_PHYSX5
			physicsScene->fetchResults(true);
#endif
		}
	}
	for (auto b = clothSolvers.begin(), e = clothSolvers.end(); b != e; b++) {
		if ((*b)->beginSimulation(deltaTime)) {
			int count = (*b)->getSimulationChunkCount();
			for (int i = 0; i < count; i++) {
				(*b)->simulateChunk(i);
			}
			(*b)->endSimulation();
		}
	}
	/*unsigned int size;
	PxActor** cobj = physicsScene->getActiveActors(size);
	for (int i = 0; i < size; i++) {
		PhysicalBody* obj = (PhysicalBody*)cobj[i]->userData;
		if (obj != NULL) {
			obj->onStepSimulation(obj, this);
			obj->updateObjectTransform();
		}
	}*/
#endif
}

bool PhysicalWorld::rayTest(const Vector3f & startPoint, const Vector3f & endPoint, ContactInfo& result)
{
#ifdef PHYSICS_USE_BULLET
	PVec3 sp = toPVec3(startPoint), ep = toPVec3(endPoint);
	btCollisionWorld::ClosestRayResultCallback res(sp, ep);
	physicsScene->rayTest(sp, ep, res);
	if (!res.hasHit())
		return false;
	PhysicalBody* obj = dynamic_cast<PhysicalBody*>((CollisionObject*)res.m_collisionObject);
	if (obj == NULL)
		return false;
	result.physicalObject = obj;
	result.otherObject = &obj->targetTransform;
	result.location = toVector3f(res.m_hitPointWorld);
	result.normal = toVector3f(res.m_hitNormalWorld);
	result.objectName = obj->targetTransform.path;
	result.impact = (endPoint - startPoint).normalized();
	return true;
#endif
#ifdef PHYSICS_USE_PHYSX
	PVec3 sp = toPVec3(startPoint);
	PVec3 dir = toPVec3(endPoint - startPoint);
	PxRaycastBuffer hit;
	if (!physicsScene->raycast(sp, dir.getNormalized(), dir.normalize(), hit))
		return false;
	if (hit.block.actor == NULL || hit.block.actor->userData == NULL)
		return false;
	PhysicalBody* obj = (PhysicalBody*)hit.block.actor->userData;
	result.physicalObject = obj;
	result.objectName = obj->targetTransform.name;
	result.otherObject = &obj->targetTransform;
	result.location = toVector3f(hit.block.position);
	result.normal = toVector3f(hit.block.normal).normalized();
	return true;
#endif
}

bool rayResultComp(const ContactInfo& a, const ContactInfo& b) {
	return (b.location - a.location).dot(a.impact) >= 0;
}

#ifdef PHYSICS_USE_PHYSX
bool pxRayTestIter(const PxRaycastHit& hit, vector<ContactInfo>& contects) {
	PhysicalBody* obj = (PhysicalBody*)hit.actor->userData;
	ContactInfo info;
	info.physicalObject = obj;
	info.objectName = obj->targetTransform.name;
	info.otherObject = &obj->targetTransform;
	info.location = toVector3f(hit.position);
	info.normal = toVector3f(hit.normal);
	contects.emplace_back(info);
	return true;
}
#endif

bool PhysicalWorld::rayTestAll(const Vector3f & startPoint, const Vector3f & endPoint, vector<ContactInfo> & result)
{
#ifdef PHYSICS_USE_BULLET
	PVec3 sp = toPVec3(startPoint), ep = toPVec3(endPoint);
	btCollisionWorld::AllHitsRayResultCallback res(sp, ep);
	physicsScene->rayTest(sp, ep, res);
	if (!res.hasHit())
		return false;
	int s = res.m_collisionObjects.size();
	for (int i = 0; i < s; i++) {
		PhysicalBody* obj = dynamic_cast<PhysicalBody*>((CollisionObject*)res.m_collisionObjects[i]);
		if (obj == NULL)
			continue;
		result.push_back(ContactInfo());
		result.back().physicalObject = obj;
		result.back().otherObject = &obj->targetTransform;
		result.back().location = toVector3f(res.m_hitPointWorld[i]);
		result.back().normal = toVector3f(res.m_hitNormalWorld[i]);
		result.back().objectName = obj->targetTransform.path;
		result.back().impact = (endPoint - startPoint).normalized();
	}
	sort(result.begin(), result.end(), rayResultComp);
	return true;
#endif
#ifdef PHYSICS_USE_PHYSX
	PVec3 sp = toPVec3(startPoint);
	PVec3 dir = toPVec3(endPoint - startPoint);
	PxHitIterator<PxRaycastHit, ContactInfo> hits(result, pxRayTestIter);
	if (!physicsScene->raycast(sp, dir.getNormalized(), dir.normalize(), hits))
		return false;
	return hits.nbTouches != 0;
#endif
}

bool PhysicalWorld::sweep(const Geometry& geometry, const Vector3f& startPoint, const Vector3f& endPoint, ContactInfo& result)
{
	PVec3 sp = toPVec3(startPoint);
	PVec3 dir = toPVec3(endPoint - startPoint);
	PxSweepBuffer hit;
	CollisionShape* geo = geometry.generateCollisionShape();
	if (!physicsScene->sweep(*geo, PxTransform(sp), dir.getNormalized(), dir.normalize(), hit)) {
		delete geo;
		return false;
	}
	delete geo;
	if (hit.block.actor == NULL || hit.block.actor->userData == NULL)
		return false;
	PhysicalBody* obj = (PhysicalBody*)hit.block.actor->userData;
	result.physicalObject = obj;
	result.objectName = obj->targetTransform.name;
	result.otherObject = &obj->targetTransform;
	result.location = toVector3f(hit.block.position);
	result.normal = toVector3f(hit.block.normal).normalized();
	return true;
}

#ifdef PHYSICS_USE_PHYSX
bool pxSweepIter(const PxSweepHit& hit, vector<ContactInfo>& contects) {
	PhysicalBody* obj = (PhysicalBody*)hit.actor->userData;
	ContactInfo info;
	info.physicalObject = obj;
	info.objectName = obj->targetTransform.name;
	info.otherObject = &obj->targetTransform;
	info.location = toVector3f(hit.position);
	info.normal = toVector3f(hit.normal);
	contects.emplace_back(info);
	return true;
}
#endif

bool PhysicalWorld::sweepAll(const Geometry& geometry, const Vector3f& startPoint, const Vector3f& endPoint, vector<ContactInfo>& result)
{
	PVec3 sp = toPVec3(startPoint);
	PVec3 dir = toPVec3(endPoint - startPoint);
	PxHitIterator<PxSweepHit, ContactInfo> hits(result, pxSweepIter);
	CollisionShape* geo = geometry.generateCollisionShape();
	if (!physicsScene->sweep(*geo, PxTransform(sp), dir.getNormalized(), dir.normalize(), hits)) {
		delete geo;
		return false;
	}
	delete geo;
	return hits.nbTouches != 0;
}

bool PhysicalWorld::init()
{
	if (isInitSDK)
		return true;
#ifdef PHYSICS_USE_PHYSX
	gFoundation = PxCreateFoundation(
		PX_PHYSICS_VERSION,
		gDefaultAllocatorCallback,
		gDefaultErrorCallback);

	gPvd = PxCreatePvd(*gFoundation);
	//PxPvdTransport* transport = PxDefaultPvdFileTransportCreate("_PVD_.pvd");
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("localhost", 5425, 10000);
	if (!gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL))
		Console::error("PVD connect failed");

	// Creating instance of PhysX SDK
	gPhysicsSDK = PxCreatePhysics(
		PX_PHYSICS_VERSION,
		*gFoundation,
		PxTolerancesScale(), true, gPvd);

	if (gPhysicsSDK == NULL) {
		cerr << "Error creating PhysX4 device." << endl;
		return false;
	}

	gDispatcher = PxDefaultCpuDispatcherCreate(8);
	if (!gDispatcher)
		cerr << "PxDefaultCpuDispatcherCreate failed!" << endl;

#if PX_USE_GPU
	PxCudaContextManagerDesc cudaContextManagerDesc;
	gCudaContextManager = PxCreateCudaContextManager(*gFoundation, cudaContextManagerDesc, PxGetProfilerCallback());
	if (!gCudaContextManager)
		cerr << "PxCreateCudaContextManager failed!" << endl;

	nv::cloth::InitializeNvCloth(&gDefaultAllocatorCallback, &gDefaultErrorCallback, (nv::cloth::PxAssertHandler*)&PxGetAssertHandler(),PxGetProfilerCallback());
	auto cudaContext = gCudaContextManager->getContext();
	gNvClothFactory = NvClothCreateFactoryCUDA(cudaContext);
	if (!gNvClothFactory) {
		throw runtime_error("NvClothCreateFactoryCUDA failed!");
		cerr << "NvClothCreateFactoryCUDA failed!" << endl;
	}
#else
	nv::cloth::InitializeNvCloth(&gDefaultAllocatorCallback, &gDefaultErrorCallback, nv::cloth::GetNvClothAssertHandler(), PxGetProfilerCallback());
	gNvClothFactory = NvClothCreateFactoryCPU();
	if (!gNvClothFactory) {
		throw runtime_error("NvClothCreateFactoryCUDA failed!");
		cerr << "NvClothCreateFactoryCUDA failed!" << endl;
	}
#endif

	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale());
	if (!gCooking)
		cerr << "PxCreateCooking failed!" << endl;

	if (!PxInitExtensions(*gPhysicsSDK, gPvd)) {
		cerr << "PxInitExtensions failed!" << endl;
		return false;
	}
	PxInitVehicleSDK(*gPhysicsSDK);
	physx::PxVehicleSetBasisVectors(PxVec3(0, 0, 1), PxVec3(1, 0, 0));
	physx::PxVehicleSetUpdateMode(PxVehicleUpdateMode::eVELOCITY_CHANGE);
#endif // !PHYSICS_USE_PHYSX
	isInitSDK = true;
	return true;
}

void PhysicalWorld::release()
{
#ifdef PHYSICS_USE_PHYSX
	if (!isInitSDK) {
		isInitSDK = false;
		return;
	}
	PxCloseVehicleSDK();
	NvClothDestroyFactory(gNvClothFactory);
	gCooking->release();
	gDispatcher->release();
	gPhysicsSDK->release();
	if (gPvd)
	{
		PxPvdTransport* transport = gPvd->getTransport();
		gPvd->release();
		gPvd = NULL;
		if (transport)
			transport->release();
	}
	gFoundation->release();
#endif // !PHYSICS_USE_PHYSX
}

#ifdef PHYSICS_USE_PHYSX
void PhysicalWorld::onContactModify(PxContactModifyPair * const pairs, PxU32 count)
{
	if (pairs->actor[0]->userData == NULL || pairs->actor[1]->userData == NULL)
		return;
	PhysicalBody* objA = (PhysicalBody*)pairs->actor[0]->userData;
	PhysicalBody* objB = (PhysicalBody*)pairs->actor[1]->userData;
	PhysicalBody* colliderA = (PhysicalBody*)pairs->shape[0]->userData;
	PhysicalBody* colliderB = (PhysicalBody*)pairs->shape[1]->userData;
	ContactInfo info;
	info.physicalObject = objB;
	info.physicalCollider = colliderB;
	info.objectName = objB->targetTransform.name;
	info.otherObject = &objB->targetTransform;
	info.location = toVector3f(pairs->contacts.getPoint(0));
	info.normal = toVector3f(pairs->contacts.getNormal(0));
	objA->handleCollision(info);

	info.physicalObject = objA;
	info.physicalCollider = colliderA;
	info.objectName = objA->targetTransform.name;
	info.otherObject = &objA->targetTransform;
	info.location = toVector3f(pairs->contacts.getPoint(1));
	info.normal = toVector3f(pairs->contacts.getNormal(1));
	objB->handleCollision(info);
}

void _clip(Vector3f& p0, const Vector3f p1, const Vector3f& camDir, const Vector3f& camPos, float zNear)
{
	Vector3f p = camPos + camDir * zNear;
	float w = camDir.dot(p);
	Vector3f a = p1 - p0;
	float t = (w - p0.dot(camDir)) / a.dot(camDir);
	p0 += a * t;
}

void PhysicalWorld::drawDedug(ImDrawList * list, const PxRenderBuffer & buffer, Camera & cam)
{
	if (list == NULL)
		return;
	Matrix4f pvm = cam.getProjectionMatrixReversedZ() * cam.getViewMatrix();
	Vector3f camDir = cam.getForward(WORLD);
	Vector3f camPos = cam.getPosition(WORLD);
	for (PxU32 i = 0; i < buffer.getNbLines(); i++)
	{
		const PxDebugLine& line = buffer.getLines()[i];
		Vector3f p0 = toVector3f(line.pos0);
		Vector3f p1 = toVector3f(line.pos1);
		Vector4f _p0 = pvm * Vector4f(p0.x(), p0.y(), p0.z(), 1);
		Vector4f _p1 = pvm * Vector4f(p1.x(), p1.y(), p1.z(), 1);

		if ((_p0.z() < cam.zNear || _p0.z() > cam.zFar) &&
			(_p1.z() < cam.zNear || _p1.z() > cam.zFar))
			continue;

		if (_p0.z() < cam.zNear) {
			_clip(p0, p1, camDir, camPos, cam.zNear);
			_p0 = pvm * Vector4f(p0.x(), p0.y(), p0.z(), 1);
		}

		if (_p1.z() < cam.zNear) {
			_clip(p1, p0, camDir, camPos, cam.zNear);
			_p1 = pvm * Vector4f(p1.x(), p1.y(), p1.z(), 1);
		}

		_p0 /= _p0.w();
		_p1 /= _p1.w();

		Vector2f __p0((_p0.x() + 1.0f) * 0.5f, (1.0f - _p0.y()) * 0.5f);
		Vector2f __p1((_p1.x() + 1.0f) * 0.5f, (1.0f - _p1.y()) * 0.5f);

		CohenSutherlandLineClipAndDraw(__p0, __p1);

		list->AddLine({ __p0.x() * cam.size.x, __p0.y() * cam.size.y },
			{ __p1.x() * cam.size.x, __p1.y() * cam.size.y },
			(line.color0 + line.color1) / 2);
	}
	for (PxU32 i = 0; i < buffer.getNbPoints(); i++)
	{
		const PxDebugPoint& point = buffer.getPoints()[i];
		Vector3f p0 = toVector3f(point.pos);
		Vector4f _p0 = pvm * Vector4f(p0.x(), p0.y(), p0.z(), 1);
		_p0 /= _p0.w();
		if (_p0.z() < 0)
			continue;
		list->AddCircleFilled({ (_p0.x() + 1.0f) * 0.5f * cam.size.x, (1.0f - _p0.y()) * 0.5f * cam.size.y },
			2, point.color);
	}
	/*for (PxU32 i = 0; i < buffer.getNbTriangles(); i++)
	{
		const PxDebugTriangle& tri = buffer.getTriangles()[i];
		Vector3f p0 = toVector3f(tri.pos0);
		Vector3f p1 = toVector3f(tri.pos1);
		Vector3f p2 = toVector3f(tri.pos2);
		Vector4f _p0 = pvm * Vector4f(p0.x(), p0.y(), p0.z(), 1);
		Vector4f _p1 = pvm * Vector4f(p1.x(), p1.y(), p1.z(), 1);
		Vector4f _p2 = pvm * Vector4f(p2.x(), p2.y(), p2.z(), 1);
		_p0 /= _p0.w();
		_p1 /= _p1.w();
		_p2 /= _p2.w();
		if (_p0.z() < 0 && _p1.z() < 0 && _p2.z() < 0)
			continue;
		list->AddTriangleFilled(
			{ (_p0.x() + 1.0f) * 0.5f * cam.size.x, (1.0f - _p0.y()) * 0.5f * cam.size.y },
			{ (_p1.x() + 1.0f) * 0.5f * cam.size.x, (1.0f - _p1.y()) * 0.5f * cam.size.y },
			{ (_p2.x() + 1.0f) * 0.5f * cam.size.x, (1.0f - _p2.y()) * 0.5f * cam.size.y },
			(tri.color0 + tri.color1 + tri.color2) / 3);
	}*/
	/*for (PxU32 i = 0; i < buffer.getNbTexts(); i++)
	{
		const PxDebugText& text = buffer.getTexts()[i];
		Vector3f p0 = toVector3f(text.position);
		Vector4f _p0 = pvm * Vector4f(p0.x(), p0.y(), p0.z(), 1);
		_p0 /= _p0.w();
		if (_p0.z() < 0)
			continue;
		list->AddText(ImVec2(_p0.x(), _p0.y()), text.color, text.string);
	}*/
}
#endif // !PHYSICS_USE_PHYSX
