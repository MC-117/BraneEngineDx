#pragma once

#include "GUIEngineLoop.h"
#include "../World.h"

class WorldEngineLoop : public EngineLoop
{
public:
    WorldEngineLoop(Ref<World> world);

    void setWorld(Ref<World> world);

    virtual bool willQuit();
	
    virtual void init();
    virtual void loop(float deltaTime);
    virtual void release();
protected:
    Ref<World> world;
};

class WorldRenderEngineLoop : public GUIEngineLoop
{
public:
    WorldRenderEngineLoop(Ref<World> world, GUI& gui, RenderPool& renderPool);

    void setWorld(Ref<World> world);

    virtual bool willQuit();
	
    virtual void init();
    virtual void release();
protected:
    Ref<World> world;

    virtual void onTick(float deltaTime);
    virtual void onPrerender();
};
