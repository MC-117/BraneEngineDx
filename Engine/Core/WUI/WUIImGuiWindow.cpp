#include "WUIImGuiWindow.h"
#include "../Engine.h"
#include "../GUI/GUIUtility.h"
#include "../RenderCore/RenderThread.h"

static void ImGui_ImplWin32_GetWin32StyleFromViewportFlags(ImGuiViewportFlags flags, DWORD* out_style, DWORD* out_ex_style)
{
    if (flags & ImGuiViewportFlags_NoDecoration)
        *out_style = WS_POPUP;
    else
        *out_style = WS_OVERLAPPEDWINDOW;

    if (flags & ImGuiViewportFlags_NoTaskBarIcon)
        *out_ex_style = WS_EX_TOOLWINDOW;
    else
        *out_ex_style = WS_EX_APPWINDOW;

    if (flags & ImGuiViewportFlags_TopMost)
        *out_ex_style |= WS_EX_TOPMOST;
}

WUIImGuiWindow::WUIImGuiWindow()
    : WUIWindow(Engine::windowContext.hinstance)
    , viewport(NULL)
    , deviceSurface(NULL)
{
}

WUIImGuiWindow::~WUIImGuiWindow()
{
    if (viewport) {
        viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
        viewport->RendererUserData = nullptr;
    }
    if (deviceSurface)
        delete deviceSurface;
}

void WUIImGuiWindow::initViewport(ImGuiViewport& viewport)
{
    this->viewport = &viewport;
    if (this->viewport != ImGui::GetMainViewport()) {
        DWORD style, exStyle;
        ImGui_ImplWin32_GetWin32StyleFromViewportFlags(viewport.Flags, &style, &exStyle);
        setStyle(style);
        setExStyle(exStyle);
    }

    WUIWindow* parent_window = nullptr;
    if (viewport.ParentViewportId != 0)
        if (ImGuiViewport* parent_viewport = ImGui::FindViewportByID(viewport.ParentViewportId))
            parent_window = (WUIWindow*)parent_viewport->PlatformUserData;
    if (parent_window) {
        parent_window->addControl(*this);
    }
    /*setPosAndSize({ (int)viewport.Pos.x, (int)viewport.Pos.y },
        { (int)viewport.Size.x, (int)viewport.Size.y });*/
    viewport.PlatformRequestResize = true;
    viewport.PlatformUserData = this;
}

HWND WUIImGuiWindow::create()
{
    WUIWindow::create();
    if (deviceSurface == NULL) {
        viewport->PlatformHandle = viewport->PlatformHandleRaw = hWnd;
        deviceSurfaceDesc.width = size.x;
        deviceSurfaceDesc.height = size.y;
        deviceSurfaceDesc.windowHandle = hWnd;
        deviceSurface = VendorManager::getInstance().getVendor().newDeviceSurface(deviceSurfaceDesc);
        viewport->RendererUserData = deviceSurface;
    }
    return hWnd;
}

void WUIImGuiWindow::updateWindow()
{
    DWORD style, exStyle;
    ImGui_ImplWin32_GetWin32StyleFromViewportFlags(viewport->Flags, &style, &exStyle);

    DWORD curStyle = getStyle() & ~WS_CLIPSIBLINGS & ~WS_VISIBLE;
    DWORD curExStyle = getExStyle();

    if (curStyle != style || curExStyle != exStyle)
    {
        // (Optional) Update TopMost state if it changed _after_ creation
        bool top_most_changed = (curExStyle & WS_EX_TOPMOST) != (exStyle & WS_EX_TOPMOST);
        HWND insert_after = top_most_changed ? ((viewport->Flags & ImGuiViewportFlags_TopMost) ? HWND_TOPMOST : HWND_NOTOPMOST) : 0;
        UINT swp_flag = top_most_changed ? 0 : SWP_NOZORDER;

        // Apply flags and position (since it is affected by flags)
        setStyle(style);
        setExStyle(exStyle);
        RECT rect = { (LONG)viewport->Pos.x, (LONG)viewport->Pos.y, (LONG)(viewport->Pos.x + viewport->Size.x), (LONG)(viewport->Pos.y + viewport->Size.y) };
        ::AdjustWindowRectEx(&rect, getStyle(), FALSE, getExStyle()); // Client to Screen
        pos = { (int)viewport->Pos.x, (int)viewport->Pos.y };
        size = { (int)viewport->Size.x, (int)viewport->Size.y };
        ::SetWindowPos(getHWnd(), insert_after, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, swp_flag | SWP_NOACTIVATE | SWP_FRAMECHANGED);
        ::ShowWindow(getHWnd(), SW_SHOWNA); // This is necessary when we alter the style
        viewport->PlatformRequestMove = viewport->PlatformRequestResize = true;
    }
}

void WUIImGuiWindow::setWindowAlpha(float alpha)
{
    if (alpha < 1.0f)
    {
        setExStyle(getExStyle() | WS_EX_LAYERED);
        ::SetLayeredWindowAttributes(hWnd, 0, (BYTE)(255 * alpha), LWA_ALPHA);
    }
    else
    {
        setExStyle(getExStyle() & ~WS_EX_LAYERED);
    }
}

IDeviceSurface* WUIImGuiWindow::getDeviceSurface() const
{
    return deviceSurface;
}

LRESULT WUIImGuiWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    /*----- Vendor envoke WndProc -----*/
    {
        if (VendorManager::getInstance().getVendor().wndProcFunc(hWnd, msg, wParam, lParam))
            return true;
    }

    switch (msg)
    {
    case WM_CLOSE:
        viewport->PlatformRequestClose = true;
        break;
    case WM_MOVE:
    {
        viewport->PlatformRequestMove = true;
        ImGuiViewportP* viewportP = (ImGuiViewportP*)viewport;
        RECT rc;
        GetWindowRect(hWnd, &rc);
        viewportP->Pos = viewportP->LastPlatformPos = { (float)rc.left, (float)rc.top };
        break;
    }
    case WM_SIZE:
    {
        viewport->PlatformRequestResize = true;
        ImGuiViewportP* viewportP = (ImGuiViewportP*)viewport;
        RECT rc;
        GetWindowRect(hWnd, &rc);
        viewportP->Size = viewportP->LastPlatformSize =
        { (float)(rc.right - rc.left), (float)(rc.bottom - rc.top) };
        break;
    }
    case WM_MOUSEACTIVATE:
        if (viewport->Flags & ImGuiViewportFlags_NoFocusOnClick)
            return MA_NOACTIVATE;
        break;
    case WM_NCHITTEST:
        // Let mouse pass-through the window. This will allow the backend to call io.AddMouseViewportEvent() correctly. (which is optional).
        // The ImGuiViewportFlags_NoInputs flag is set while dragging a viewport, as want to detect the window behind the one we are dragging.
        // If you cannot easily access those viewport flags from your windowing/event code: you may manually synchronize its state e.g. in
        // your main loop after calling UpdatePlatformWindows(). Iterate all viewports/platform windows and pass the flag to your windowing system.
        if (viewport->Flags & ImGuiViewportFlags_NoInputs)
            return HTTRANSPARENT;
        break;
    }

    return WUIWindow::WndProc(msg, wParam, lParam);
}

void WUIImGuiWindow::onResize(WPARAM wParam, const Unit2Di& size)
{
    if (deviceSurface)
        deviceSurface->resize(size.x, size.y);
    WUIWindow::onResize(wParam, size);
}

void WUIImGuiWindow::onResizeExit()
{
    if (deviceSurface)
        deviceSurface->resize(clientSize.x, clientSize.y);
}

void WUIImGuiWindow::onImGuiRender()
{
    IRenderContext& context = *VendorManager::getInstance().getVendor().getDefaultRenderContext();
    context.execteImGuiDraw(viewport->DrawData);
}

void WUIImGuiWindow::onLoop()
{
    if (deviceSurface) {
        RenderThreadContext context;
        context.renderGraph = RenderPool::get().renderGraph;
        context.cameraRenderData = RenderPool::get().sceneData->cameraRenderDatas.front();
        context.sceneRenderData = RenderPool::get().sceneData;

        RENDER_CONTEXT_SCOPE(context);
        
        RENDER_THREAD_ENQUEUE_TASK(DrawImGUI, ([this] (RenderThreadContext& context)
        {
            IRenderContext& renderContext = *VendorManager::getInstance().getVendor().getDefaultRenderContext();
            renderContext.bindSurface(deviceSurface);
            onImGuiRender();
            deviceSurface->swapBuffer(Engine::engineConfig.vsnyc, Engine::engineConfig.maxFPS);
        }));
    }
    WUIWindow::onLoop();
}
