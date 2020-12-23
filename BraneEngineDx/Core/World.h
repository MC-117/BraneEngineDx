#pragma once
#ifndef _WORLD_H_
#define _WORLD_H_
#if ENABLE_PHYSICS
#include "PhysicalWorld.h"
#endif
#include "Input.h"
#include "Transform.h"
#include "Camera.h"
#include "RenderPool.h"

class World : public Transform {
public:
	Serialize(World);
#if ENABLE_PHYSICS
	PhysicalWorld physicalWorld;
	thread* renderThread = NULL;
#endif

	Unit2Di screenSize = { 1280, 720 };

	Camera defaultCamera;
	RenderPool renderPool = RenderPool(defaultCamera);
	Input input;

	World();
	virtual ~World();

	virtual void begin();
	virtual void tick(float deltaTime);
	virtual void afterTick();
	virtual void end();
	void quit();
	bool willQuit();
	string addObject(Object& object);
	string addObject(Object* object);
	void destroyObject(string name);

	void setGUIOnly(bool value);
	bool getGUIOnly();
	void addUIControl(UIControl& uc);
	void addUIControl(UIControl* uc);

	void bindUI(void(*uiDraw)(GUI&));

	Camera& getCurrentCamera();
	Camera& getDefaultCamera();
	void switchCamera(Camera& camera);
	void switchToDefaultCamera();

	void updateListener();
	void setMainVolume(float v);
	
	void setViewportSize(int width, int height);
	int64_t getEngineTime();
	int64_t getSystemTime();

	bool loadTransform(const string& path);
	bool saveTransform(const string& path);

	World& operator+=(Object& object);
	World& operator+=(Object* object);

	World& operator+=(UIControl& uc);
	World& operator+=(UIControl* uc);

	void loadWorld(const SerializationInfo& from);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	int64_t currentTime = 0, startTime = 0, lastTime = 0;
	bool isQuit = false, guiOnly = false;
	ObjectIterator iter = ObjectIterator(this);
	Camera* camera = &defaultCamera;
	vector<Object*> destroyList;

	int64_t getCurrentTime();
};

#endif // !_WORLD_H_
