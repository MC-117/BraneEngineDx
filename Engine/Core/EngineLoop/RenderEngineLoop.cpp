#include "RenderEngineLoop.h"

#include "../GUI/GUISurface.h"

RenderEngineLoop::RenderEngineLoop(RenderPool& renderPool)
    : renderPool(renderPool)
{
}

bool RenderEngineLoop::willQuit()
{
    return false;
}

void RenderEngineLoop::init()
{
    renderPool.start();
}

void RenderEngineLoop::loop(float deltaTime)
{
    onTick(deltaTime);
    renderPool.beginRender();
    Camera* currentCamera = GUISurface::getMainGUISurface().getCamera();
    renderInfo.sceneData = renderPool.sceneData;
    renderInfo.renderGraph = renderPool.renderGraph;
    renderInfo.camera = currentCamera;
    onPrerender();
    renderPool.render(false);
    onPostrender();
    renderPool.endRender();
}

void RenderEngineLoop::release()
{
}

void RenderEngineLoop::onTick(float deltaTime)
{
}

void RenderEngineLoop::onPrerender()
{
}

void RenderEngineLoop::onPostrender()
{
}
