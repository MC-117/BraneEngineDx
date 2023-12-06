#pragma once
#ifndef _WORLD_H_
#define _WORLD_H_
#include "Input.h"
#include "IWorld.h"
#include "Transform.h"
#include "Audio/AudioSource.h"
#include "Camera.h"
#include "RenderPool.h"

#if ENABLE_PHYSICS
#include "Physics/PhysicalWorld.h"
#endif

class ENGINE_API World : public Transform, public IWorld {
public:
	Serialize(World, Transform);
#if ENABLE_PHYSICS
	PhysicalWorld physicalWorld;
	thread* renderThread = NULL;
#endif

	Unit2Di screenSize = { 1280, 720 };

#ifdef AUDIO_USE_OPENAL
	AudioListener audioListener;
#endif // AUDIO_USE_OPENAL

	Camera defaultCamera;

	World();
	virtual ~World();

	virtual void begin();
	virtual void tick(float deltaTime);
	virtual void afterTick();
	virtual void prerender(SceneRenderData& sceneData);
	virtual void end();
	void quit(int code = 0);
	void setPause(bool pause);
	bool getPause() const;
	bool willQuit();
	bool willRestart();
	string addObject(Object& object);
	string addObject(Object* object);
	void destroyObject(string name);

	virtual Object* find(const string& name) const;
	virtual Object* getObject() const;

	void setGUIOnly(bool value);
	bool getGUIOnly();

	Camera& getCurrentCamera();
	Camera& getDefaultCamera();
	void switchCamera(Camera& camera);
	void switchToDefaultCamera();

#ifdef AUDIO_USE_OPENAL
	void updateListener();
	void setMainVolume(float v);
#endif // AUDIO_USE_OPENAL
	
	int64_t getEngineTime();
	int64_t getSystemTime();

	bool loadTransform(const string& path);
	bool saveTransform(const string& path);

	World& operator+=(Object& object);
	World& operator+=(Object* object);

	void loadWorld(const SerializationInfo& from);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	int64_t currentTime = 0, startTime = 0, lastTime = 0;
	bool isQuit = false, guiOnly = false, pause = false;
	bool doseWarmUp = false;
	int quitCode = 0;
	ObjectIterator iter = ObjectIterator(this);
	Camera* camera = &defaultCamera;
	vector<Object*> destroyList;

	int64_t getCurrentTime();
};

#endif // !_WORLD_H_
