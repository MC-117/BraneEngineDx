#include "GUIEngineLoop.h"
#include "../Engine.h"

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

    surface->bindSurface();
    surface->clearColor(Color(0, 0, 0));

    IVendor& Vendor = VendorManager::getInstance().getVendor();
    Vendor.imGuiDrawFrame(Engine::engineConfig, Engine::windowContext);

    surface->swapBuffer(Engine::engineConfig.vsnyc, Engine::engineConfig.maxFPS);
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
