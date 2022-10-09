#include "World.h"
//#include "MaterialLoader.h"
#include <fstream>
#include "Utility/Utility.h"
#include "Console.h"
#include "Engine.h"

SerializeInstance(World);

Timer timer;

void renderTick(World* w)
{
	while (!w->willQuit())
		if (w != NULL)
			w->renderPool.render();
}

World::World() : Transform("RootWorld")
{
	if (defaultCamera.parent == NULL)
		*this += defaultCamera;
	defaultCamera.setActive(true);
	renderPool.switchToDefaultCamera();
}

World::~World()
{
}

void World::begin()
{
	lastTime = currentTime = startTime = getCurrentTime();
	Transform::begin();
	iter.reset();
	while (iter.next())
		iter.current().begin();
	renderPool.start();
}

void World::tick(float deltaTime)
{
	timer.reset();
	currentTime = getCurrentTime();
	float dt = (currentTime - lastTime) * 0.000000001f;
	lastTime = currentTime;
	Transform::tick(deltaTime);
	iter.reset();
	vector<Object*> destroyList;
	while (iter.next()) {
		Object& obj = iter.current();
		vector<Render*> rds;
		obj.getRenders(rds);
		if (obj.isDestroy()) {
			obj.end();
			destroyList.push_back(&obj);
			for (int i = 0; i < rds.size(); i++)
				renderPool.remove(*rds[i]);
#if ENABLE_PHYSICS
			obj.releasePhysics(physicalWorld);
#endif
			DestroyFlag flag = obj.getDestroyFlag();
			if (flag == DestroyFlag::Self)
				iter.unparentCurrent();
		}
		else {
			for (int i = 0; i < rds.size(); i++)
				if (rds[i]->renderPool == NULL)
					renderPool.add(*rds[i]);
#if ENABLE_PHYSICS
			obj.setupPhysics(physicalWorld);
#endif
			if (!obj.isinitialized())
				obj.begin();
			if (!pause) {
				if (doseWarmUp)
					obj.tick(deltaTime);
				else
					doseWarmUp = true;
			}
		}
	}

	if (Engine::input.getCursorHidden()) {
		renderPool.gui.gizmo.setCameraControl(Gizmo::CameraControlMode::None);
	}
	else {
		renderPool.gui.gizmo.setCameraControl(Gizmo::CameraControlMode::Free);
	}
	renderPool.gui.gizmo.onUpdate(getCurrentCamera());
	for (auto b = destroyList.rbegin(), e = destroyList.rend(); b != e; b++)
		delete *b;
	timer.record("Tick");
#if ENABLE_PHYSICS
	physicalWorld.updatePhysicalWorld(dt);
#endif
	timer.record("Physics");
}

void World::afterTick()
{
	Transform::afterTick();
	iter.reset();
	while (iter.next())
		iter.current().afterTick();
	timer.record("Aftertick");
#ifdef AUDIO_USE_OPENAL
	updateListener();
#endif // AUDIO_USE_OPENAL
	iter.reset();
	if (!guiOnly) {
		while (iter.next())
			iter.current().prerender(renderPool.sceneData);
		timer.record("PreRender");
	}

	renderPool.render(guiOnly);
	timer.record("Render");
	Console::getTimer("Game") = timer;
}

void World::end()
{
	iter.reset();
	while (iter.next()) {
		Object* obj = &iter.current();
		delete obj;
	}
	Transform::end();
}

void World::quit(int code)
{
	isQuit = true;
	quitCode = code;
}

void World::setPause(bool pause)
{
	this->pause = pause;
}

bool World::getPause() const
{
	return pause;
}

bool World::willQuit()
{
	return isQuit;
}

bool World::willRestart()
{
	return isQuit && quitCode == 1;
}

string World::addObject(Object & object)
{
	addChild(object);
	vector<Render*> rds;
	object.getRenders(rds);
	for (int i = 0; i < rds.size(); i++)
		if (rds[i]->renderPool == NULL)
			renderPool.add(*rds[i]);
	return object.name;
}

string World::addObject(Object * object)
{
	addChild(*object);
	vector<Render*> rds;
	object->getRenders(rds);
	for (int i = 0; i < rds.size(); i++)
		if (rds[i]->renderPool == NULL)
			renderPool.add(*rds[i]);
	return object->name;
}

void World::destroyObject(string name)
{
	Object* obj = find(name);
	if (obj != NULL)
		obj->destroy();
}

Object* World::find(const string& name) const
{
	ObjectConstIterator iter(this);
	while (iter.next()) {
		Object* obj = &iter.current();
		if (obj->name == name)
			return obj;
	}
	return NULL;
}

Object* World::getObject() const
{
	return (Object*)this;
}

void World::setGUIOnly(bool value)
{
	guiOnly = value;
}

bool World::getGUIOnly()
{
	return guiOnly;
}

void World::addUIControl(UIControl & uc)
{
	renderPool.gui += uc;
}

void World::addUIControl(UIControl * uc)
{
	renderPool.gui += uc;
}

Camera & World::getCurrentCamera()
{
	if (camera == NULL)
		switchToDefaultCamera();
	return *camera;
}

Camera & World::getDefaultCamera()
{
	return defaultCamera;
}

void World::switchCamera(Camera & camera)
{
	if (this->camera != NULL)
		this->camera->setActive(false);
	this->camera = &camera;
	camera.setActive(true);
	camera.setSize(screenSize);
	renderPool.switchCamera(camera);
}

void World::switchToDefaultCamera()
{
	if (camera != NULL)
		camera->setActive(false);
	camera = &defaultCamera;
	camera->setActive(true);
	defaultCamera.setSize(screenSize);
	renderPool.switchToDefaultCamera();
}

#ifdef AUDIO_USE_OPENAL
void World::updateListener()
{
	if (camera != NULL) {
		audioListener.setPoseture(
			camera->getPosition(WORLD),
			camera->getForward(WORLD),
			camera->getUpward(WORLD));
		audioListener.update();
	}
}
#endif // AUDIO_USE_OPENAL

#ifdef AUDIO_USE_OPENAL
void World::setMainVolume(float v)
{
	audioListener.setVolume(v);
}
#endif // AUDIO_USE_OPENAL

void World::setViewportSize(int width, int height)
{
	screenSize = { width, height };
	renderPool.setViewportSize(screenSize);
}

int64_t World::getEngineTime()
{
	return lastTime - startTime;
}

int64_t World::getSystemTime()
{
	return currentTime;
}

bool World::loadTransform(const string & path)
{
	ifstream file(path);
	if (file.fail()) {
		cout << "Load world transform failed\n";
		return false;
	}
	string line;
	while (getline(file, line)) {
		vector<string> strs = split(line, ':');
		if (strs.size() != 2)
			continue;
		Transform *t = dynamic_cast<Transform*>(find(trim(strs[0], ' ')));
		if (t == NULL) {
			cout << "Cannot find (Object) " + strs[0] << endl;
			continue;
		}
		if (t == &getCurrentCamera()) {
			continue;
		}
		strs = split(strs[1], ',');
		if (strs.size() != 9)
			continue;
		float data[9];
		for (int i = 0; i < 9; i++) {
			data[i] = stof(strs[i]);
		}
		t->setPosition(data[0], data[1], data[2]);
		t->setRotation(data[3], data[4], data[5]);
		t->setScale(data[6], data[7], data[8]);
	}
	file.close();
	return true;
}

bool World::saveTransform(const string & path)
{
	string data;
	ObjectIterator iter = ObjectIterator(this);
	iter.reset();
	while (iter.next()) {
		Transform* t = dynamic_cast<Transform*>(&iter.current());
		if (t != NULL) {
			char str[1000];
			Vector3f rot = t->getEulerAngle();
			sprintf_s(str, ": %f, %f, %f, %f, %f, %f, %f, %f, %f\n", t->position.x(), t->position.y(), t->position.z(),
				rot.x(), rot.y(), rot.z(), t->scale.x(), t->scale.y(), t->scale.z());
			data += t->name + str;
		}
	}
	std::ofstream file(path);
	if (file.fail()) {
		cout << "Save world transform failed\n";
		return false;
	}
	file << data;
	file.close();
	return true;
}

World & World::operator+=(Object & object)
{
	addObject(object);
	return *this;
}

World & World::operator+=(Object * object)
{
	addObject(object);
	return *this;
}

World & World::operator+=(UIControl & uc)
{
	addUIControl(uc);
	return *this;
}

World & World::operator+=(UIControl * uc)
{
	addUIControl(uc);
	return *this;
}

void World::loadWorld(const SerializationInfo & from)
{
	ChildrenInstantiateObject(from, this, IR_ExistUniqueName);
}

Serializable * World::instantiate(const SerializationInfo & from)
{
	World* w = new World();
	ChildrenInstantiate(Object, from, w);
	return w;
}

bool World::deserialize(const SerializationInfo & from)
{
	if (!Transform::deserialize(from))
		return false;
	return true;
}

bool World::serialize(SerializationInfo & to)
{
	if (!Transform::serialize(to))
		return false;
	return true;
}

int64_t World::getCurrentTime()
{
	return Time::now().toNanosecond();
}
