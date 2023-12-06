#include "WorldEngineLoop.h"
#include "../GUI/GUISurface.h"

WorldEngineLoop::WorldEngineLoop(Ref<World> world)
{
    setWorld(world);
}

void WorldEngineLoop::setWorld(Ref<World> world)
{
    this->world = world;
}

bool WorldEngineLoop::willQuit()
{
    return world->willQuit();
}

void WorldEngineLoop::init()
{
    if (!world)
        return;
    world->begin();
}

void WorldEngineLoop::loop(float deltaTime)
{
    if (!world)
        return;
    world->tick(deltaTime);
    world->afterTick();
}

void WorldEngineLoop::release()
{
    if (!world)
        return;
    world->end();
}

WorldRenderEngineLoop::WorldRenderEngineLoop(Ref<World> world, GUI& gui, RenderPool& renderPool)
    : GUIEngineLoop(gui, renderPool)
{
    setWorld(world);
}

void WorldRenderEngineLoop::setWorld(Ref<World> world)
{
    this->world = world;
}

bool WorldRenderEngineLoop::willQuit()
{
    return world->willQuit();
}

void WorldRenderEngineLoop::init()
{
    GUIEngineLoop::init();
    if (!world)
        return;
    world->begin();
}

void WorldRenderEngineLoop::release()
{
    if (!world)
        return;
    world->end();
}

void WorldRenderEngineLoop::onTick(float deltaTime)
{
    World* pWorld = world;
    if (!pWorld)
        return;
    pWorld->tick(deltaTime);
    GUISurface& surface = GUISurface::getMainGUISurface();
    surface.gizmoUpdate();
    GUIEngineLoop::onTick(deltaTime);
    pWorld->afterTick();
}

void WorldRenderEngineLoop::onPrerender()
{
    World* pWorld = world;
    if (!pWorld)
        return;
    pWorld->prerender(*renderPool.sceneData);
    GUIEngineLoop::onPrerender();
}
