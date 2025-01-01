#include "GUIEngineLoop.h"
#include "../Engine.h"
#include "../RenderCore/RenderThread.h"

GUIOnlyEngineLoop::GUIOnlyEngineLoop(GUI& gui)
    : gui(gui)
{
}

bool GUIOnlyEngineLoop::willQuit()
{
    return false;
}

void GUIOnlyEngineLoop::init()
{
	
}

void GUIOnlyEngineLoop::loop(float deltaTime)
{
    renderThreadWaitHandle.wait();
    
    RenderInfo info;
    info.camera = NULL;
    gui.onGUI(info);
    gui.render(info);
	
    ImGui::Render();
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
    }

    IDeviceSurface* surface = Engine::getMainDeviceSurface();
    if (surface == NULL)
        throw runtime_error("MainDeviceSurface not initialized");

    RenderThreadContext context;
    CameraRenderData cameraRenderData;
    context.renderGraph = RenderPool::get().renderGraph;
    context.cameraRenderData = &cameraRenderData;
    context.sceneRenderData = RenderPool::get().sceneData;

    RENDER_CONTEXT_SCOPE(context);
        
    renderThreadWaitHandle = RENDER_THREAD_ENQUEUE_TASK(DrawImGUI, ([surface] (RenderThreadContext& context)
    {
        IRenderContext& renderContext = *VendorManager::getInstance().getVendor().getDefaultRenderContext();
        renderContext.bindSurface(surface);
        renderContext.execteImGuiDraw(ImGui::GetDrawData());
        surface->swapBuffer(Engine::engineConfig.vsnyc, Engine::engineConfig.maxFPS);
    }));
}

void GUIOnlyEngineLoop::release()
{
}

GUIEngineLoop::GUIEngineLoop(GUI& gui, RenderPool& renderPool)
    : RenderEngineLoop(renderPool)
    , gui(gui)
{
}

void GUIEngineLoop::onPrerender()
{
    gui.onGUI(renderInfo);
    RenderEngineLoop::onPrerender();
}

void GUIEngineLoop::onPostrender()
{
    RenderEngineLoop::onPostrender();
    gui.render(renderInfo);
}
