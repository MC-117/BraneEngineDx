#include "Application.h"
#include "WUI/LoadingUI.h"
#include "WUI/WUIMainWindow.h"

AppArguments::AppArguments(int argc, char** argv)
{
    init(argc, argv);
}

AppArguments& AppArguments::get()
{
    static AppArguments instance;
    return instance;
}

void AppArguments::init(int argc, char** argv)
{
    this->argv.resize(argc);
    for (int i = 0; i < argc; i++)
        this->argv[i] = argv[i];
}

int AppArguments::size() const
{
    return argv.size();
}

string& AppArguments::operator[](int i)
{
    return argv[i];
}

const string& AppArguments::operator[](int i) const
{
    return argv[i];
}

Application* Application::globalApplication = NULL;

Application::Application()
{
    if (globalApplication)
        throw runtime_error("Allowed only one global application instance");
    globalApplication = this;
}

Application& Application::get()
{
    if (globalApplication == NULL)
        throw runtime_error("None application is registered");
    return *globalApplication;
}

EngineApplication::EngineApplication(Engine& engine, NativeIconHandle iconHandle)
    : engine(engine)
    , iconHandle(iconHandle)
{
}

void EngineApplication::main()
{
    string workingSpace;
    if (AppArguments::get().size() > 1)
        workingSpace = AppArguments::get()[1];

    engine.config(workingSpace, "");
    engine.setupBaseFramework();
    engine.setupPhysicsFramework();
    engine.setupGUIFramework(iconHandle);
    initializeGUI();
    engine.setupRenderFramework();
    engine.setupGameFramework();
    engine.setupBeforeAssetLoading();
    loadAssets();
    if (engine.prepareGame()) {
        initializeEngineLoop();
        startMainLoop();
    }
    engine.releaseGameFramework();
    engine.releasePhysicsFramework();
    engine.releaseRenderFramework();
    engine.releaseGUIFramework();
    engine.releaseBaseFramework();
}

WUIMainWindow* EngineWindowApplication::WindowEngineContext::getMainWindow()
{
    return &mainWindow;
}

IDeviceSurface* EngineWindowApplication::WindowEngineContext::getMainDeviceSurface()
{
    return mainWindow.getDeviceSurface();
}

void EngineWindowApplication::WindowEngineContext::toggleFullscreen()
{
    mainWindow.toggleFullscreen();
}

WUIMainWindow EngineWindowApplication::mainWindow;

EngineWindowApplication::EngineWindowApplication(NativeIconHandle iconHandle)
    : EngineApplication(defaultEngine, iconHandle)
    , defaultEngine(context)
{
}

void EngineWindowApplication::initializeGUI()
{
    /*----- WinAPI Viewport window setup -----*/
    IVendor& vendor = VendorManager::getInstance().getVendor();
    mainWindow.setHInstance(Engine::windowContext.hinstance);
    ImGuiViewport& mainViewport = *ImGui::GetMainViewport();
    mainViewport.Flags |= ImGuiViewportFlags_NoDecoration;
    mainWindow.initViewport(mainViewport);
    mainWindow.setClientSize({ (int)Engine::engineConfig.screenWidth, (int)Engine::engineConfig.screenHeight });
    mainWindow.setText("BraneEngine v" + string(ENGINE_VERSION) + " | Vendor Api: " + vendor.getName());
    HWND hwnd = mainWindow.create();
    Engine::windowContext.hwnd = hwnd;
    Engine::getInput().setHWND(hwnd);
}

struct UIImportContext : public ImportContext
{
    LoadingUI& ui;

    UIImportContext(LoadingUI& ui) : ui(ui)
    {
        sourceFlags = ImportSourceFlags::Import_Source_None;
        if (Engine::engineConfig.loadDefaultAsset)
            sourceFlags |= ImportSourceFlags::Import_Source_Default;
        if (Engine::engineConfig.loadEngineAsset)
            sourceFlags |= ImportSourceFlags::Import_Source_Engine;
        if (Engine::engineConfig.loadContentAsset)
            sourceFlags |= ImportSourceFlags::Import_Source_Content;
    }

    virtual void report(const string& text)
    {
        ui.setText(text);
    }
};

void EngineWindowApplication::startMainLoop()
{
    mainWindow.doModel(false);
}

WorldApplication::WorldApplication(NativeIconHandle iconHandle)
    : EngineWindowApplication(iconHandle)
{
}

void WorldApplication::loadAssets()
{
    LoadingUI loadingUI("Engine/Banner/Banner.bmp", engine.windowContext.hinstance);
    loadingUI.setText("Start BraneEngine");
    loadingUI.doModelAsync([](WUIControl& control, void* ptr)
    {
        LoadingUI& ui = dynamic_cast<LoadingUI&>(control);
        UIImportContext importContext(ui);
        Engine::get().loadAssets(importContext);
    });
}

void WorldApplication::initializeEngineLoop()
{
    World* world = new World();
    Engine::get().changeWorld(world);
    initializeWorld();
    Engine::get().getEngineLoop()->init();
}
