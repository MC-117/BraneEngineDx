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
	RenderCommandList cmdList;

	RenderPool(Camera& defaultCamera);

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
};

#endif // !_RENDER_POOL_H_
