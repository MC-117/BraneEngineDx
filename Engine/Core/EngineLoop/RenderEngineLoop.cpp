#include "RenderEngineLoop.h"

#include "../GUI/GUISurface.h"
#include "Core/RenderCore/RenderThread.h"

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
}

void RenderEngineLoop::loop(float deltaTime)
{
    IImporter::tick();
    
    onTick(deltaTime);
    
    Camera* currentCamera = GUISurface::getMainGUISurface().getCamera();
    RenderThreadContext context;
    context.renderGraph = renderPool.renderGraph;
    context.sceneRenderData = renderPool.sceneData;
    context.cameraRenderData = currentCamera->cameraRender.getRenderData();

    RENDER_CONTEXT_SCOPE(context);
    renderPool.beginRender();
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
