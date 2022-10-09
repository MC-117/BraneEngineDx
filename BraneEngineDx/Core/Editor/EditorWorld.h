#pragma once

#include "../Input.h"
#include "../IWorld.h"
#include "../AudioSource.h"
#include "../PostProcess/PostProcessingCamera.h"
#include "../RenderPool.h"

#if ENABLE_PHYSICS
#include "../Physics/PhysicalWorld.h"
#endif

class EditorWorld : public Transform, public IWorld
{
public:
	PostProcessingCamera camera;
	DirectLight directLight;

#if ENABLE_PHYSICS
	PhysicalWorld physicalWorld;
#endif

	EditorWorld();
	virtual ~EditorWorld();

	virtual void begin();
	virtual void tick(float deltaTime);
	virtual void afterTick();
	virtual void prerender(SceneRenderData& sceneData);
	virtual void render(RenderGraph& renderGraph);
	virtual void end();

	virtual Object* find(const string& name) const;
	virtual Object* getObject() const;

	virtual void update();
	virtual Texture* getSceneTexture();

	void setViewportSize(int width, int height);
	float getDeltaTime() const;
protected:
	Time lastTime, currentTime, startTime;
	float deltaTime = 0;
	ObjectIterator iter = ObjectIterator(this);
	SceneRenderData sceneRenderData;
	bool updatePhysics = false;
};