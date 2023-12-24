#pragma once
#ifndef _RENDER_POOL_H_
#define _RENDER_POOL_H_

#include "Camera.h"
#include "GUI/GUI.h"

using namespace std;

class ENGINE_API RenderPool
{
public:
	SceneRenderData* sceneData = NULL;
	RenderGraph* renderGraph = NULL;

	RenderPool();
	virtual ~RenderPool();

	static RenderPool& get();

	void start();

	void setViewportSize(const Vector2i& size);
	void add(Render& render);
	void remove(Render& render);

	void beginRender();

	void render(bool guiOnly = false);

	void endRender();

	RenderPool& operator+=(Render& render);
	RenderPool& operator-=(Render& render);

protected:
	set<Render*> prePool;
	set<Render*> pool;
	thread renderThread;

	atomic_ullong renderFrame = 0;
	bool destory = false;

	void gameFence();
	void renderFence();

	void renderThreadMain();

	static void renderThreadLoop(RenderPool* pool);
};

#endif // !_RENDER_POOL_H_
