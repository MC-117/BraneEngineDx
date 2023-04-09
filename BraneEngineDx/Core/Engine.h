#pragma once
#ifndef _ENGINE_H_
#define _ENGINE_H_

#include "IVendor.h"
#include "Asset.h"
#include "World.h"
#include "Input.h"

class WUIMainWindow;

static class Engine
{
public:
	static string version;
	static Input input;
	static World* currentWorld;
	static WindowContext windowContext;
	static EngineConfig engineConfig;
	static WUIMainWindow mainWindow;

	static IDeviceSurface* getMainDeviceSurface();
	static World* getCurrentWorld();
	static void setViewportSize(const Unit2Di& size);
	static void toggleFullscreen();
	static void config();
	static void setup();
	static void start();
	static void clean();
	static void mainLoop(float deltaTime);
};

void InitialWorld();

void InitialTool();

#endif // !_ENGINE_H_