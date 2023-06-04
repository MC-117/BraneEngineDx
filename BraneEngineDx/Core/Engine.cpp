#define ENABLE_MEMTRACER 0
#define _AFXDLL
#include <filesystem>
#include <tchar.h>
#include <fstream>
#include "Engine.h"
#if ENABLE_MEMTRACER
#include <memoryhook.h>
#include <memorytracer.h>
#endif // ENABLE_MEMTRACER
#include "Utility/TextureUtility.h"
#include "InitializationManager.h"
#include "../resource.h"
#include "Console.h"
#include "Profile/ProfileCore.h"
#include "Importer/AssimpImporter.h"
#include "WUI/LoadingUI.h"
#include "WUI/WUIMainWindow.h"
#include "Script/PythonManager.h"
#include "../ThirdParty/ImGui/imgui_internal.h"
#include "../ThirdParty/ImGui/ImGuiIconHelp.h"
#include "../ThirdParty/ImGui/ImPlot/implot.h"

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

void loadAssets(LoadingUI& log, bool loadDefaultAsset, bool loadEngineAsset, bool loadContentAsset)
{
	namespace FS = filesystem;
	vector<FS::path> delayLoadAsset;
	if (!loadDefaultAsset)
		return;

	if (!FS::exists("Engine")) {
		MessageBox(NULL, "Engine Folder Missing", "Error", MB_OK);
		Engine::clean();
		exit(-1);
	}

	ShaderManager::loadDefaultAdapter("Engine/Shaders/Default");

	if (!Material::loadDefaultMaterial()) {
		MessageBox(NULL, "Default material load failed", "Error", MB_OK);
		Engine::clean();
		exit(-1);
	}

	if (!Texture2D::loadDefaultTexture()) {
		MessageBox(NULL, "Default texture load failed", "Error", MB_OK);
		Engine::clean();
		exit(-1);
	}

	LongProgressWork work;
	work.data = &log;
	work.callback += [](const LongProgressWork& work) {
		LoadingUI& log = *(LoadingUI*)work.data;
		log.setText(work.text);
	};

	if (loadEngineAsset)
		IImporter::loadFolder("Engine", &work);
	if (loadContentAsset && !testCompilingShaders) {
		if (FS::exists("Content")) {
			IImporter::loadFolder("Content", &work);
		}
	}
}

string configPath;

void main(int argc, char** argv) {
	if (argc > 1) {
		configPath = argv[1];
	}
	else {
		if (MessageBoxA(NULL, "Load Tool?", "Config", MB_YESNO) == IDYES) {
			configPath = "ConfigTool.ini";
		}
	}
	Engine::config();
	Engine::setup();
	Engine::start();
	Engine::clean();
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	if (__argc > 1) {
		configPath = __targv[1];
	}
	else {
		if (MessageBoxA(NULL, "Load Tool?", "Config", MB_YESNO) == IDYES) {
			configPath = "ConfigTool.ini";
		}
	}
	//try {
		Engine::windowContext.hinstance = hInstance;
		Engine::config();
		Engine::setup();
		Engine::start();
		Engine::clean();
	/*}
	catch (exception e) {
		MessageBoxA(NULL, e.what(), "Error", MB_ICONERROR);
		return -1;
	}*/
	return 0;
}
string Engine::version = ENGINE_VERSION;
Input Engine::input;
World* Engine::currentWorld = NULL;
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
WUIMainWindow Engine::mainWindow;

IDeviceSurface* Engine::getMainDeviceSurface()
{
	return mainWindow.getDeviceSurface();
}

World * Engine::getCurrentWorld()
{
	return currentWorld;
}

void Engine::setViewportSize(const Unit2Di& size)
{
	if (!windowContext.fullscreen)
		windowContext.screenSize = size;
	if (currentWorld)
		currentWorld->setViewportSize(size.x, size.y);
	/*----- Vendor resize window -----*/
	{
		if (!VendorManager::getInstance().getVendor().resizeWindow(Engine::engineConfig, Engine::windowContext, size.x, size.y))
			throw runtime_error("Vendor resie window failed");
	}
}

void Engine::toggleFullscreen()
{
	mainWindow.toggleFullscreen();
}

void Engine::config()
{
	InitializationManager::instance().initialize(InitializeStage::BeforeEngineConfig);
	if (configPath.empty())
		configPath = "Config.ini";
	ifstream f = ifstream(configPath);
	if (f.fail()) {
		Console::error("%s read failed", configPath.c_str());
		if (configPath != "Config.ini") {
			f.open("Config.ini");
			if (f.fail())
				return;
		}
		else
			return;
	}
	SerializationInfoParser sip(f);
	if (!sip.parse()) {
		Console::error("Config.ini read failed");
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

void SetStyleColors(ImGuiStyle* dst = 0)
{
	ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
	ImVec4* colors = style->Colors;
	style->FrameRounding = 3;
	style->FramePadding = { 5, 5 };
	style->WindowTitleAlign = { 0.5, 0.5 };
	style->WindowRounding = 1;
	//style->ScaleAllSizes(1.3);

	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 0.95f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.20f, 0.46f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.85f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.22f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 0.53f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.12f, 0.12f, 0.12f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.48f, 0.48f, 0.48f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.79f, 0.79f, 0.79f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.48f, 0.47f, 0.47f, 0.91f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.55f, 0.55f, 0.62f);
	colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 0.63f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.67f, 0.67f, 0.68f, 0.63f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.26f, 0.26f, 0.26f, 0.63f);
	colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.20f, 0.58f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.64f, 0.65f, 0.65f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.25f, 0.25f, 0.80f);
	colors[ImGuiCol_Separator] = ImVec4(0.00f, 0.68f, 0.84f, 0.50f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.64f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.81f, 0.81f, 0.81f, 0.64f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.87f, 0.87f, 0.87f, 0.53f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.87f, 0.87f, 0.87f, 0.74f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.87f, 0.87f, 0.87f, 0.74f);
	colors[ImGuiCol_Tab] = ImVec4(0.20f, 0.20f, 0.20f, 0.86f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.00f, 0.68f, 0.84f, 0.50f);
	colors[ImGuiCol_TabActive] = ImVec4(0.00f, 0.68f, 0.84f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.68f, 0.68f, 0.68f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.77f, 0.33f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.87f, 0.55f, 0.08f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.47f, 0.60f, 0.76f, 0.47f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.58f, 0.58f, 0.58f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

	//for (int i = ImGuiCol_WindowBg; i <= ImGuiCol_ModalWindowDimBg; i++)
	//	if (i != ImGuiCol_PopupBg)
	//		colors[i].w *= 0.96;
}

static bool alutInited = true;

void imGuiSetup()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigWindowsMoveFromTitleBarOnly = true;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

	io.Fonts->AddFontFromFileTTF("Engine/Fonts/arialuni.ttf",
		20, NULL, io.Fonts->GetGlyphRangesChineseFull());

	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
	io.Fonts->AddFontFromFileTTF(("Engine/Fonts/fa-solid-900.ttf"), 15, &icons_config, icons_ranges);

	io.Fonts->AddFontFromFileTTF("Engine/Fonts/ChakraPetch-Light.ttf", 72, NULL, io.Fonts->GetGlyphRangesChineseFull());

	// Setup Dear ImGui style
	SetStyleColors();
}

void imGuiClean()
{
	ImPlot::DestroyContext();
	ImGui::DestroyContext();
}

void Engine::setup()
{
	InitializationManager::instance().initialize(InitializeStage::BeforeEngineSetup);

	if (enableMemTracer) {
#if ENABLE_MEMTRACER
		mtracer::install_hook(true);
#endif
	}

	CoInitialize(0);
	{

#ifdef UNICODE
		wchar_t execPath[MAX_PATH];
#else
		char execPath[MAX_PATH];
#endif // UNICODE
		GetModuleFileName(NULL, execPath, MAX_PATH);
		windowContext.executionPath = execPath;
	}
	VendorManager::getInstance().instantiateVendor(engineConfig.vendorName);

	IVendor& vendor = VendorManager::getInstance().getVendor();

	InitializationManager::instance().initialize(InitializeStage::BeforeScriptSetup);

	PythonManager::start();

	InitializationManager::instance().initialize(InitializeStage::BeforePhysicsSetup);

#if ENABLE_PHYSICS
	if (!PhysicalWorld::init())
		throw runtime_error("Physics Engine init failed");
#endif

	InitializationManager::instance().initialize(InitializeStage::BeforeWindowSetup);

	RECT rect;
	GetWindowRect(GetDesktopWindow(), &rect);
	windowContext.fullscreenSize = { rect.right - rect.left, rect.bottom - rect.top };

	WUIControl::registDefaultClass(windowContext.hinstance);

	/*----- Vendor window setup -----*/
	{
		if (!vendor.windowSetup(engineConfig, windowContext))
			throw runtime_error("Vendor window setup failed");
	}

	imGuiSetup();

	/*----- WinAPI Viewport window setup -----*/
	mainWindow.setHInstance(windowContext.hinstance);
	ImGuiViewport& mainViewport = *ImGui::GetMainViewport();
	mainViewport.Flags |= ImGuiViewportFlags_NoDecoration;
	mainWindow.initViewport(mainViewport);
	mainWindow.setClientSize({ (int)engineConfig.screenWidth, (int)engineConfig.screenHeight });
	mainWindow.setText("BraneEngine v" + string(ENGINE_VERSION) + " | Vendor Api: " + vendor.getName());
	windowContext.hwnd = mainWindow.create();
	input.setHWND(windowContext.hwnd);

	InitializationManager::instance().initialize(InitializeStage::BeforeRenderVendorSetup);

	LoadingUI loadingUI("Engine/Banner/Banner.bmp", windowContext.hinstance);

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

	InitializationManager::instance().initialize(InitializeStage::BeforeAssetLoading);

	currentWorld = new World();

	loadingUI.setText("Start BraneEngine");
	loadingUI.doModelAsync([](WUIControl& control, void* ptr)
	{
		LoadingUI& ui = dynamic_cast<LoadingUI&>(control);
		loadAssets(ui, engineConfig.loadDefaultAsset, engineConfig.loadEngineAsset, engineConfig.loadContentAsset);


		if (testCompilingShaders)
			return;

		ui.setText("Initial...");
		if (engineConfig.guiOnly) {
			if (currentWorld)
				currentWorld->setGUIOnly(true);
			InitialTool();
		}
		else
			InitialWorld();
	});

	if (testCompilingShaders)
		return;

	if (engineConfig.fullscreen)
		toggleFullscreen();

	input.init();

	InitializationManager::instance().initialize(InitializeStage::AfterEngineSetup);
}

void Engine::start()
{
	if (testCompilingShaders)
		return;
	currentWorld->begin();
	mainWindow.doModel(false);
}

void Engine::clean()
{
	InitializationManager::instance().finalize(FinalizeStage::BeforeEngineRelease);

	input.release();

	InitializationManager::instance().finalize(FinalizeStage::BeforeScriptRelease);
	PythonManager::end();

	currentWorld->end();
	delete currentWorld;

	InitializationManager::instance().finalize(FinalizeStage::BeforePhysicsRelease);
#if ENABLE_PHYSICS
	currentWorld->physicalWorld.physicsScene->release();
	PhysicalWorld::release();
#endif

	InitializationManager::instance().finalize(FinalizeStage::BeforeRenderVenderRelease);

	IVendor& vendor = VendorManager::getInstance().getVendor();

	/*----- Vendor ImGui shutdown -----*/
	{
		if (!vendor.imGuiShutdown(engineConfig, windowContext))
			throw runtime_error("Vendor ImGui shutdown failed");
	}

	imGuiClean();

	/*----- Vendor clean -----*/
	{
		if (!vendor.clean(engineConfig, windowContext))
			throw runtime_error("Vendor clean failed");
	}

	InitializationManager::instance().finalize(FinalizeStage::AfterRenderVenderRelease);

	CoUninitialize();

	InitializationManager::instance().finalize(FinalizeStage::AfterEngineRelease);
}

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
	if (currentWorld->willQuit()) {
		PostQuitMessage(currentWorld->willRestart() ? 42 : 0);
	}
	Time::update();
	input.update();
	ProfilerManager::instance().tick();
	currentWorld->tick(deltaTime);
	currentWorld->afterTick();
	timer.record("CPU");
	Console::getTimer("Engine") = timer;
	Console::resetNewLogCount();
	Console::resetNewPyLogCount();

#if ENABLE_MEMTRACER
	if (input.getKeyPress(VK_F8))
		dumpTraceFile();
#endif
}
