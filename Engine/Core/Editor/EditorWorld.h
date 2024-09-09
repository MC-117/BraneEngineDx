#pragma once

#include "../Input.h"
#include "../IWorld.h"
#include "../Audio/AudioSource.h"
#include "../Camera.h"
#include "../DirectLight.h"
#include "../RenderPool.h"
#include "../GUI/GUISurface.h"
#include "../CameraManager.h"

#if ENABLE_PHYSICS
#include "../Physics/PhysicalWorld.h"
#endif

class EditorWorld : public Transform, public IWorld
{
public:
	Camera camera;
	DirectLight directLight;

#if ENABLE_PHYSICS
	PhysicalWorld physicalWorld;
#endif

	EditorWorld(const string& name);
	virtual ~EditorWorld();

	virtual void begin();
	virtual void tick(float deltaTime);
	virtual void afterTick();
	virtual void prerender(SceneRenderData& sceneData);
	virtual void render(RenderInfo& info);
	virtual void onGUI(ImDrawList* drawList);
	virtual void end();

	virtual Object* find(const string& name) const;
	virtual Object* getObject() const;

	virtual void update();
	virtual GUISurface& getGUISurface();
	virtual Gizmo& getGizmo();
	virtual SceneRenderData* getSceneRenderData();

	void setViewportSize(int width, int height);
	float getDeltaTime() const;
protected:
	Time lastTime, currentTime, startTime;
	float deltaTime = 0;
	CameraManager cameraManager;
	GUISurface guiSurface;
	ObjectIterator iter = ObjectIterator(this);
	SceneRenderData* sceneRenderData = NULL;
	bool updatePhysics = false;
};