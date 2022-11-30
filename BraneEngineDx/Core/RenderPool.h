#pragma once
#ifndef _RENDER_POOL_H_
#define _RENDER_POOL_H_

#include "Camera.h"
#include "GUI/GUI.h"

using namespace std;

class RenderPool
{
public:
	Camera& defaultCamera;
	Camera* camera = NULL;
	GUI gui;
	SceneRenderData* sceneData = NULL;
	RenderGraph* renderGraph = NULL;

	RenderPool(Camera& defaultCamera);
	virtual ~RenderPool();

	void start();

	void setViewportSize(Unit2Di size);
	void switchToDefaultCamera();
	void switchCamera(Camera& camera);
	void add(Render& render);
	void remove(Render& render);
	void render(bool guiOnly = false);

	RenderPool& operator+=(Render& render);
	RenderPool& operator-=(Render& render);

protected:
	set<Render*> prePool;
	set<Render*> pool;
	thread renderThread;

	unsigned int renderFrame = 0;
	bool destory = false;

	void gameFence();
	void renderFence();

	void renderThreadMain();

	static void renderThreadLoop(RenderPool* pool);
};

#endif // !_RENDER_POOL_H_
