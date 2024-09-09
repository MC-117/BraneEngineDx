#define ENABLE_MEMTRACER 1
#define _AFXDLL
#include <filesystem>
#include <fstream>
#include "Engine.h"
#if ENABLE_MEMTRACER
#include <memoryhook.h>
#include <memorytracer.h>
#endif // ENABLE_MEMTRACER
#include "Utility/TextureUtility.h"
#include "InitializationManager.h"
#include "Console.h"
#include "Profile/ProfileCore.h"
#include "Importer/AssimpImporter.h"
#include "WUI/LoadingUI.h"
#include "WUI/WUIMainWindow.h"
#include "Script/PythonManager.h"
#include "../ThirdParty/ImGui/imgui_internal.h"
#include "../ThirdParty/ImGui/ImGuiIconHelp.h"
#include "../ThirdParty/ImGui/ImPlot/implot.h"
#include "EngineLoop/WorldEngineLoop.h"
#include "GUI/GUIUtility.h"

void (*pInitialWorldFunc)() = NULL;

static bool enableMemTracer = false;
static int testCompilingShaders = false;

void SetTopWindow(HWND hWnd)
{
	HWND hForeWnd = GetForegroundWindow();
	DWORD dwForeID = GetWindowThreadProcessId(hForeWnd, NULL);
	DWORD dwCurID = GetCurrentThreadId();
	AttachThreadInput(dwCurID, dwForeID, TRUE);
	ShowWindow(hWnd, SW_SHOWNORMAL);
	SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	SetForegroundWindow(hWnd);
	AttachThreadInput(dwCurID, dwForeID, FALSE);
 }

Engine* Engine::globalEngine = NULL;
WindowContext Engine::windowContext =
{
	{ 100, 100 },
	{ 1280, 720 },
	{ 1280, 720 },
	"Test",
	0,
	NULL,
	NULL,
	false
};
EngineConfig Engine::engineConfig;

Engine::Engine(EngineContext& context)
	: context(context)
	, version(ENGINE_VERSION)
	, currentWorld(NULL)
{
	if (globalEngine)
		throw runtime_error("Allowed only one global engine instance");
	globalEngine = this;
}

Engine& Engine::get()
{
	if (globalEngine == NULL)
		throw runtime_error("globalEngine is NULL");
	return *globalEngine;
}

const string& Engine::getVersion()
{
	return get().version;
}

Input& Engine::getInput()
{
	return get().input;
}

WUIMainWindow* Engine::getMainWindow()
{
	return get().context.getMainWindow();
}

IDeviceSurface* Engine::getMainDeviceSurface()
{
	return get().context.getMainDeviceSurface();
}

World * Engine::getCurrentWorld()
{
	return get().currentWorld;
}

void Engine::setViewportSize(const Unit2Di& size)
{
	if (!windowContext.fullscreen)
		windowContext.screenSize = size;
	if (getCurrentWorld())
		RenderPool::get().setViewportSize({ size.x, size.y });
	/*----- Vendor resize window -----*/
	{
		if (!VendorManager::getInstance().getVendor().resizeWindow(Engine::engineConfig, Engine::windowContext, size.x, size.y))
			throw runtime_error("Vendor resie window failed");
	}
}

void Engine::toggleFullscreen()
{
	get().context.toggleFullscreen();
}

void Engine::config(const string& workingSpace, const string& configPath)
{
	filesystem::path workingPath = filesystem::absolute(workingSpace);
	if (filesystem::exists(workingPath))
		filesystem::current_path(workingPath);
	else
		workingPath = filesystem::current_path();
	windowContext.workingPath = workingPath.generic_u8string();
	
	{
#ifdef UNICODE
		wchar_t execPath[MAX_PATH];
#else
		char execPath[MAX_PATH];
#endif // UNICODE
		GetModuleFileName(windowContext.hinstance, execPath, MAX_PATH);
		windowContext.executionPath = execPath;
	}
	
	InitializationManager::instance().initialize(InitializeStage::BeforeEngineConfig);
	const char* defaultConfigPath = "Config.ini";
	
	string path = configPath.empty() ? defaultConfigPath : configPath;
	ifstream f = ifstream(path);
	if (f.fail()) {
		Console::error("%s read failed", path.c_str());
		if (configPath != defaultConfigPath) {
			path = path;
			f.open(defaultConfigPath);
			if (f.fail())
				return;
		}
		else
			return;
	}
	SerializationInfoParser sip(f);
	if (!sip.parse()) {
		Console::error("%s read failed", path.c_str());
		return;
	}
	f.close();
	SerializationInfo& conf = sip.infos[0];
	engineConfig.configInfo = conf;
	string vendorName;
	conf.get(".vendorName", vendorName);
	if (!vendorName.empty())
		engineConfig.vendorName = vendorName;
	string startMapPath = "Content/world.asset";
	if (conf.get(".startMapPath", startMapPath))
		engineConfig.startMapPath = startMapPath;
	string boolStr;
	conf.get(".fullscreen", boolStr);
	if (boolStr == "true")
		engineConfig.fullscreen = true;
	boolStr.clear();
	conf.get(".guiOnly", boolStr);
	if (boolStr == "true")
		engineConfig.guiOnly = true;
	boolStr.clear();
	conf.get(".loadDefaultAsset", boolStr);
	if (boolStr == "false")
		engineConfig.loadDefaultAsset = false;
	boolStr.clear();
	conf.get(".loadEngineAsset", boolStr);
	if (boolStr == "false")
		engineConfig.loadEngineAsset = false;
	boolStr.clear();
	conf.get(".loadContentAsset", boolStr);
	if (boolStr == "false")
		engineConfig.loadContentAsset = false;
	boolStr.clear();
	conf.get(".vsnyc", boolStr);
	if (boolStr == "true")
		engineConfig.vsnyc = true;
	float maxFPS = engineConfig.maxFPS;
	conf.get(".maxFPS", maxFPS);
	engineConfig.maxFPS = maxFPS;
	float msaa = engineConfig.msaa;
	conf.get(".msaa", msaa);
	engineConfig.msaa = msaa;
	float screenWidth = engineConfig.screenWidth, screenHeight = engineConfig.screenHeight;
	conf.get(".screenWidth", screenWidth);
	engineConfig.screenWidth = screenWidth;
	conf.get(".screenHeight", screenHeight);
	engineConfig.screenHeight = screenHeight;

	if (conf.get(".enableMemTracer", boolStr))
		enableMemTracer = boolStr == "true";

	conf.get("testCompilingShaders", testCompilingShaders);

	SerializationInfo* layers = conf.get("layers");
	if (layers != NULL) {
		for (auto b = layers->sublists.begin(), e = layers->sublists.end(); b != e; b++) {
			int layer; string name;
			if (b->get("layer", layer) && b->get("name", name)) {
				if (layer > 0 && layer < 32) {
					engineConfig.layerNames[layer] = name;
				}
			}
		}
	}
}

void Engine::setupBaseFramework()
{
	SerializationManager::finalizeSerializtion();
	InitializationManager::instance().initialize(InitializeStage::BeforeEngineSetup);
	if (enableMemTracer) {
#if ENABLE_MEMTRACER
		mtracer::install_hook(true);
#endif
	}

	CoInitialize(0);

	VendorManager::getInstance().instantiateVendor(engineConfig.vendorName);
}

void Engine::setupPhysicsFramework()
{
	InitializationManager::instance().initialize(InitializeStage::BeforePhysicsSetup);

#if ENABLE_PHYSICS
	if (!PhysicalWorld::init())
		throw runtime_error("Physics Engine init failed");
#endif
}

void Engine::setupGUIFramework(NativeIconHandle iconHandle)
{
	IVendor& vendor = VendorManager::getInstance().getVendor();

	InitializationManager::instance().initialize(InitializeStage::BeforeWindowSetup);

	RECT rect;
	GetWindowRect(GetDesktopWindow(), &rect);
	windowContext.fullscreenSize = { rect.right - rect.left, rect.bottom - rect.top };

	WUIControl::registDefaultClass(windowContext.hinstance, iconHandle);

	/*----- Vendor window setup -----*/
	{
		if (!vendor.windowSetup(engineConfig, windowContext))
			throw runtime_error("Vendor window setup failed");
	}

	ImGui::Setup();
}

void Engine::setupRenderFramework()
{
	IVendor& vendor = VendorManager::getInstance().getVendor();
	
	InitializationManager::instance().initialize(InitializeStage::BeforeRenderVendorSetup);

	/*----- Vendor setup -----*/
	{
		if (!vendor.setup(engineConfig, windowContext))
			throw runtime_error("Vendor setup failed");
	}

	/*----- Vendor ImGui init -----*/
	{
		if (!vendor.imGuiInit(engineConfig, windowContext))
			throw runtime_error("Vendor ImGui init failed");
	}
}

bool Engine::loadAssets(ImportContext& context)
{
	namespace FS = filesystem;

	if (!context.sourceFlags.has(ImportSourceFlags::Import_Source_Default))
		return true;

	if (!FS::exists("Engine")) {
		MessageBox(NULL, "Engine Folder Missing", "Error", MB_OK);
		return false;
	}

	ShaderManager::loadDefaultAdapter("Engine/Shaders/Default");

	if (!Material::loadDefaultMaterial()) {
		MessageBox(NULL, "Default material load failed", "Error", MB_OK);
		return false;
	}

	if (!Texture2D::loadDefaultTexture()) {
		MessageBox(NULL, "Default texture load failed", "Error", MB_OK);
		return false;
	}

	if (context.sourceFlags.has(ImportSourceFlags::Import_Source_Engine))
		IImporter::loadFolder("Engine", context);
	if (context.sourceFlags.has(ImportSourceFlags::Import_Source_Content)
		&& !testCompilingShaders) {
		if (FS::exists("Content")) {
			IImporter::loadFolder("Content", context);
		}
	}
	return true;
}

bool Engine::prepareGame()
{
	if (testCompilingShaders)
		return false;

	if (engineConfig.fullscreen)
		toggleFullscreen();

	input.init();

	InitializationManager::instance().initialize(InitializeStage::AfterEngineSetup);
	return true;
}

void Engine::setEngineLoop(EngineLoop& engineLoop)
{
	if (this->engineLoop) {
		this->engineLoop->release();
		delete this->engineLoop;
	}
	this->engineLoop = &engineLoop;
}

EngineLoop* Engine::getEngineLoop()
{
	return engineLoop;
}

void Engine::setupGameFramework()
{
	InitializationManager::instance().initialize(InitializeStage::BeforeGameSetup);

	PythonManager::start();
}

void Engine::setupBeforeAssetLoading()
{
	InitializationManager::instance().initialize(InitializeStage::BeforeAssetLoading);
}

void Engine::releaseGameFramework()
{
	InitializationManager::instance().finalize(FinalizeStage::BeforeEngineRelease);
	InitializationManager::instance().finalize(FinalizeStage::BeforeGameRelease);
	PythonManager::end();
	if (engineLoop)
		delete engineLoop;
	if (currentWorld) {
		delete currentWorld;
	}
}

void Engine::releasePhysicsFramework()
{
	InitializationManager::instance().finalize(FinalizeStage::BeforePhysicsRelease);
#if ENABLE_PHYSICS
	PhysicalWorld::release();
#endif
}

void Engine::releaseRenderFramework()
{
	InitializationManager::instance().finalize(FinalizeStage::BeforeRenderVenderRelease);

	IVendor& vendor = VendorManager::getInstance().getVendor();

	/*----- Vendor ImGui shutdown -----*/
	if (ImGui::GetCurrentContext())
	{
		if (!vendor.imGuiShutdown(engineConfig, windowContext))
			throw runtime_error("Vendor ImGui shutdown failed");
	}

	/*----- Vendor clean -----*/
	{
		if (!vendor.clean(engineConfig, windowContext))
			throw runtime_error("Vendor clean failed");
	}
	
	InitializationManager::instance().finalize(FinalizeStage::AfterRenderVenderRelease);
}

void Engine::releaseGUIFramework()
{
	ImGui::Release();
}

void Engine::releaseBaseFramework()
{
	input.release();
	CoUninitialize();

	InitializationManager::instance().finalize(FinalizeStage::AfterEngineRelease);
}

void Engine::changeWorld(World* world)
{
	currentWorld = world;
	setEngineLoop(*new WorldRenderEngineLoop(world, GUI::get(), RenderPool::get()));
}

// void Engine::setup()
// {
// 	InitializationManager::instance().initialize(InitializeStage::BeforeEngineSetup);
//
// 	if (enableMemTracer) {
// #if ENABLE_MEMTRACER
// 		mtracer::install_hook(true);
// #endif
// 	}
//
// 	CoInitialize(0);
// 	VendorManager::getInstance().instantiateVendor(engineConfig.vendorName);
//
// 	IVendor& vendor = VendorManager::getInstance().getVendor();
//
// 	InitializationManager::instance().initialize(InitializeStage::BeforeScriptSetup);
//
// 	PythonManager::start();
//
// 	InitializationManager::instance().initialize(InitializeStage::BeforePhysicsSetup);
//
// #if ENABLE_PHYSICS
// 	if (!PhysicalWorld::init())
// 		throw runtime_error("Physics Engine init failed");
// #endif
//
// 	InitializationManager::instance().initialize(InitializeStage::BeforeWindowSetup);
//
// 	RECT rect;
// 	GetWindowRect(GetDesktopWindow(), &rect);
// 	windowContext.fullscreenSize = { rect.right - rect.left, rect.bottom - rect.top };
//
// 	WUIControl::registDefaultClass(windowContext.hinstance);
//
// 	/*----- Vendor window setup -----*/
// 	{
// 		if (!vendor.windowSetup(engineConfig, windowContext))
// 			throw runtime_error("Vendor window setup failed");
// 	}
//
// 	imGuiSetup();
//
// 	/*----- WinAPI Viewport window setup -----*/
// 	mainWindow.setHInstance(windowContext.hinstance);
// 	ImGuiViewport& mainViewport = *ImGui::GetMainViewport();
// 	mainViewport.Flags |= ImGuiViewportFlags_NoDecoration;
// 	mainWindow.initViewport(mainViewport);
// 	mainWindow.setClientSize({ (int)engineConfig.screenWidth, (int)engineConfig.screenHeight });
// 	mainWindow.setText("BraneEngine v" + string(ENGINE_VERSION) + " | Vendor Api: " + vendor.getName());
// 	windowContext.hwnd = mainWindow.create();
// 	input.setHWND(windowContext.hwnd);
//
// 	InitializationManager::instance().initialize(InitializeStage::BeforeRenderVendorSetup);
//
// 	LoadingUI loadingUI("Engine/Banner/Banner.bmp", windowContext.hinstance);
//
// 	/*----- Vendor setup -----*/
// 	{
// 		if (!vendor.setup(engineConfig, windowContext))
// 			throw runtime_error("Vendor setup failed");
// 	}
//
// 	/*----- Vendor ImGui init -----*/
// 	{
// 		if (!vendor.imGuiInit(engineConfig, windowContext))
// 			throw runtime_error("Vendor ImGui init failed");
// 	}
//
// 	InitializationManager::instance().initialize(InitializeStage::BeforeAssetLoading);
//
// 	currentWorld = new World();
//
// 	loadingUI.setText("Start BraneEngine");
// 	loadingUI.doModelAsync([](WUIControl& control, void* ptr)
// 	{
// 		LoadingUI& ui = dynamic_cast<LoadingUI&>(control);
// 		loadAssets(ui, engineConfig.loadDefaultAsset, engineConfig.loadEngineAsset, engineConfig.loadContentAsset);
//
//
// 		if (testCompilingShaders)
// 			return;
//
// 		ui.setText("Initial...");
// 		if (currentWorld)
// 			currentWorld->setGUIOnly(engineConfig.guiOnly);
// 		if (application)
// 			application->initialize();
// 	});
//
// 	if (testCompilingShaders)
// 		return;
//
// 	if (engineConfig.fullscreen)
// 		toggleFullscreen();
//
// 	input.init();
//
// 	InitializationManager::instance().initialize(InitializeStage::AfterEngineSetup);
// }

#if ENABLE_MEMTRACER
void dumpTraceFile()
{
	mtracer::MemoryTracer* tracer = mtracer::MemoryTracer::instance();
	if (tracer == NULL)
		return;
	const char* traceFolder = "MemTracer";
	if (!filesystem::exists(traceFolder)) {
		filesystem::create_directory(traceFolder);
	}
	Time nowTime = Time::now();
	string timeStr = nowTime.toString();
	for (char& c : timeStr)
		if (c == ':')
			c = '_';
	string filename = string(traceFolder) + '/' + timeStr + ".memtrace";
	tracer->dump_trace_file(filename.c_str());
}
#endif

void Engine::mainLoop(float deltaTime)
{
	Timer timer;
	if (engineLoop && engineLoop->willQuit()) {
		PostQuitMessage(0);
	}
	Time::update();
	input.update();
	ProfilerManager::instance().tick();
	if (engineLoop) {
		engineLoop->loop(deltaTime);
	}
	timer.record("CPU");
	Console::getTimer("Engine") = timer;
	Console::resetNewLogCount();
	Console::resetNewPyLogCount();

#if ENABLE_MEMTRACER
	if (input.getKeyPress(VK_F8))
		dumpTraceFile();
#endif
}
