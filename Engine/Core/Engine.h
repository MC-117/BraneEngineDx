#pragma once
#ifndef _ENGINE_H_
#define _ENGINE_H_

#include "IVendor.h"
#include "Asset.h"
#include "World.h"
#include "Input.h"

class WUIMainWindow;
struct ImportContext;

struct ENGINE_API EngineContext
{
	virtual WUIMainWindow* getMainWindow() = 0;
	virtual IDeviceSurface* getMainDeviceSurface() = 0;
	virtual void toggleFullscreen() = 0;
};

typedef LPSTR NativeIconHandle;

class ENGINE_API Engine
{
public:
	static WindowContext windowContext;
	static EngineConfig engineConfig;

	Engine(EngineContext& context);
	virtual ~Engine() = default;

	static Engine& get();
	static const string& getVersion();
	static Input& getInput();
	static WUIMainWindow* getMainWindow();
	static IDeviceSurface* getMainDeviceSurface();
	static World* getCurrentWorld();
	static void setViewportSize(const Unit2Di& size);
	static void toggleFullscreen();
	
	virtual void config(const string& workingSpace, const string& configPath);
	virtual void setupBaseFramework();
	virtual void setupPhysicsFramework();
	virtual void setupGUIFramework(NativeIconHandle iconHandle);
	virtual void setupRenderFramework();
	virtual void setupGameFramework();
	virtual void setupBeforeAssetLoading();
	virtual bool loadAssets(ImportContext& context);
	virtual bool prepareGame();
	virtual void setEngineLoop(EngineLoop& engineLoop);
	EngineLoop* getEngineLoop();
	virtual void mainLoop(float deltaTime);
	virtual void releaseGameFramework();
	virtual void releasePhysicsFramework();
	virtual void releaseRenderFramework();
	virtual void releaseGUIFramework();
	virtual void releaseBaseFramework();

	virtual void changeWorld(World* world);
private:
	static Engine* globalEngine;
protected:
	EngineContext& context;
	string version;
	Input input;
	World* currentWorld;
	EngineLoop* engineLoop;
};

#endif // !_ENGINE_H_