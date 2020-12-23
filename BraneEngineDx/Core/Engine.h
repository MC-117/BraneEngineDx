#pragma once
#ifndef _ENGINE_H_
#define _ENGINE_H_

#include "IVendor.h"
#include "Asset.h"

extern World world;

static class Engine
{
public:
	static string version;
	static World* currentWorld;
	static WindowContext windowContext;
	static EngineConfig engineConfig;

	static World* getCurrentWorld();
	static void setViewportSize(int width, int height);
	static void createWindow(unsigned int width, unsigned int height);
	static void toggleFullscreen();
	static void config();
	static void setup();
	static void start();
	static void clean();
};

void resizeWindow(int width, int height);

void engineMainLoop();

LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void InitialWorld();

void InitialTool();

#endif // !_ENGINE_H_