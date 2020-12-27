#define _AFXDLL
#include <filesystem>
#include <fstream>
#include "Engine.h"
#include "Console.h"
#include "../ThirdParty/ImGui/imgui_internal.h"

World world;

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

void loadAssets(const char* path, vector<string>& delayLoadAsset)
{
	namespace FS = filesystem;
	for (auto& p : FS::recursive_directory_iterator(path)) {
		string path = p.path().generic_u8string();
#ifdef UNICODE
		USES_CONVERSION;
		wchar_t* pathStr = A2W(path.c_str());
#else
		const char* pathStr = path.c_str();
#endif // UNICODE
		if (p.status().type() == filesystem::file_type::regular) {
			string ext = p.path().extension().generic_string();
			string name = p.path().filename().generic_string();
			name = name.substr(0, name.size() - ext.size());
			Asset* asset = NULL;
			bool ignore = false;
			bool delay = false;
			SerializationInfo *ini = NULL;
			string iniPath = path + ".ini";
			if (FS::exists(iniPath.c_str())) {
				ifstream f(iniPath);
				if (!f.fail()) {
					SerializationInfoParser iniParse(f);
					if (iniParse.parse())
						ini = new SerializationInfo(iniParse.infos[0]);
					f.close();
				}
			}
			if (!_stricmp(ext.c_str(), ".obj") || !_stricmp(ext.c_str(), ".fbx") ||
				!_stricmp(ext.c_str(), ".pmx")) {
				Importer imp = Importer(path);
				bool animationOnly = false;
				if (ini != NULL) {
					string b;
					ini->get("animationOnly", b);
					animationOnly = b == "true";
				}
				if (imp.isLoad()) {
					if (!animationOnly) {
						if (imp.isSkeletonMesh()) {
							SkeletonMesh* mesh = new SkeletonMesh();
							if (imp.getSkeletonMesh(*mesh)) {
								Asset* ass = new Asset(&SkeletonMeshAssetInfo::assetInfo, name, path);
								ass->asset[0] = mesh;
								if (AssetManager::registAsset(*ass)) {
									asset = ass;
								}
								else {
									delete ass;
									delete mesh;
								}
							}
							else {
								delete mesh;
							}
						}
						else {
							Mesh* mesh = new Mesh();
							if (imp.getMesh(*mesh)) {
								Asset* ass = new Asset(&MeshAssetInfo::assetInfo, name, path);
								ass->asset[0] = mesh;
								if (AssetManager::registAsset(*ass)) {
									asset = ass;
								}
								else {
									delete ass;
									delete mesh;
								}
							}
							else {
								delete mesh;
							}
						}
					}
					if (imp.hasAnimation()) {
						vector<AnimationClipData*> anims;
						if (imp.getAnimation(anims, true)) {
							for (int i = 0; i < anims.size(); i++) {
								Asset* ass = new Asset(&AnimationClipDataAssetInfo::assetInfo, name + ":" + anims[i]->name, path + ":Animation" + to_string(i));
								ass->asset[0] = anims[i];
								if (AssetManager::registAsset(*ass))
									asset = ass;
							}
						}
					}
				}
			}
			else if (!_stricmp(ext.c_str(), ".camanim")) {
				AnimationClipData* data = AnimationLoader::loadCameraAnimation(path);
				if (data != NULL) {
					Asset* ass = new Asset(&AnimationClipDataAssetInfo::assetInfo, data->name, path);
					ass->asset[0] = data;
					if (AssetManager::registAsset(*ass))
						asset = ass;
				}
			}
			else if (!_stricmp(ext.c_str(), ".charanim")) {
				AnimationClipData* data = AnimationLoader::readAnimation(path);
				if (data == NULL)
					data = AnimationLoader::loadMotionAnimation(path);
				if (data != NULL) {
					Asset* ass = new Asset(&AnimationClipDataAssetInfo::assetInfo, data->name, path);
					ass->asset[0] = data;
					if (AssetManager::registAsset(*ass))
						asset = ass;
				}
			}
			else if (!_stricmp(ext.c_str(), ".png") || !_stricmp(ext.c_str(), ".tga") ||
				!_stricmp(ext.c_str(), ".jpg") || !_stricmp(ext.c_str(), ".bmp") ||
				!_stricmp(ext.c_str(), ".mip")) {
				const char* stdStr = "false";
				const char* filterStr = "TF_Linear_Mip_Linear";
				if (name.find("_N") != string::npos)
					stdStr = "false";
				if (name.find("_lut") != string::npos)
					filterStr = "TF_Linear";
				asset = AssetManager::loadAsset("Texture2D", name, path, { stdStr, "TW_Repeat", "TW_Repeat", filterStr, filterStr }, {});
			}
			else if (!_stricmp(ext.c_str(), ".mat")) {
				asset = AssetManager::loadAsset("Material", name, path, { "" }, {});
			}
			else if (!_stricmp(ext.c_str(), ".wav")) {
				asset = AssetManager::loadAsset("AudioData", name, path, {}, {});
			}
			else if (!_stricmp(ext.c_str(), ".asset")) {
				asset = AssetManager::loadAsset("AssetFile", name, path, {}, {});
			}
			else if (!_stricmp(ext.c_str(), ".imat")) {
				delayLoadAsset.push_back(path);
				delay = true;
			}
			else if (!_stricmp(ext.c_str(), ".ttf") || !_stricmp(ext.c_str(), ".ini")) {
				ignore = true;
			}
			else {
				Console::warn("%s unknown file type", path.c_str());
			}
			if (!delay && !ignore) {
				if (asset == NULL) {
#ifdef UNICODE
					wchar_t* namewc = A2W(name.c_str());
#else
					const char* nameStr = name.c_str();
#endif // UNICODE
					Console::error("%s %s load failed", path.c_str(), name.c_str());
				}
				else {
					Console::log("%s load", path.c_str());
				}
			}
			if (ini != NULL)
				delete ini;
		}
		else {
			Console::log("%s folder", path.c_str());
		}
	}
}

void loadAssets(bool loadDefaultAsset, bool loadEngineAsset, bool loadContentAsset)
{
	vector<string> delayLoadAsset;
	namespace FS = filesystem;
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

	if (loadEngineAsset)
		loadAssets("Engine", delayLoadAsset);
	if (loadContentAsset) {
		if (FS::exists("Content")) {
			loadAssets("Content", delayLoadAsset);
		}
	}
	for (int i = 0; i < delayLoadAsset.size(); i++) {
		FS::path p = FS::path(delayLoadAsset[i]);
		string path = p.generic_string();
		string ext = p.extension().generic_string();
		string name = p.filename().generic_string();
		name = name.substr(0, name.size() - ext.size());
		Asset* asset = NULL;
		if (!_stricmp(ext.c_str(), ".imat")) {
			asset = AssetManager::loadAsset("Material", name, path, { "" }, {});
		}
		else {
			Console::warn("%s unknown file type", path.c_str());
			continue;
		}
		if (asset == NULL) {
			const wchar_t* namewc = p.filename().c_str();
			Console::error("%s %s load failed", path.c_str(), name.c_str());
		}
		else {
			Console::log("%s load", path.c_str());
		}
	}
}

void main(int argc, char** argv) {
	Engine::config();
	Engine::setup();
	Engine::start();
	Engine::clean();
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	try {
		Engine::windowContext._hinstance = hInstance;
		Engine::config();
		Engine::setup();
		Engine::start();
		Engine::clean();
	}
	catch (exception e) {
		MessageBoxA(NULL, e.what(), "Error", MB_ICONERROR);
		return -1;
	}
	return 0;
}

string Engine::version = ENGINE_VERSION;
World* Engine::currentWorld = &world;
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

World * Engine::getCurrentWorld()
{
	return currentWorld;
}

void Engine::setViewportSize(int width, int height)
{
	if (!windowContext._fullscreen)
		windowContext.screenSize = { width, height };
	world.setViewportSize(width, height);
}

void Engine::createWindow(unsigned int width, unsigned int height)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEXW);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = wndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = windowContext._hinstance;
	wcex.hIcon = LoadIcon(wcex.hInstance, "IDI_ICON1");
	wcex.hCursor = LoadCursor(nullptr, "IDC_ARROW");
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = "BraneEngine";
	wcex.hIconSm = LoadIcon(wcex.hInstance, "IDI_ICON1");
	if (!RegisterClassEx(&wcex))
		throw runtime_error("Register Window Class Failed");

	RECT rc = { 0, 0, width, height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	HWND hwnd = CreateWindowEx(0, "BraneEngine", "BraneEngine", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, wcex.hInstance,
		nullptr);
	if (hwnd == NULL)
		throw runtime_error("Create Window Failed");
	windowContext._hwnd = hwnd;
}

void Engine::toggleFullscreen()
{
	windowContext._fullscreen = !windowContext._fullscreen;
	/*----- Vendor toggle fullscreen -----*/
	{
		if (!VendorManager::getInstance().getVendor().toggleFullscreen(Engine::engineConfig, Engine::windowContext, windowContext._fullscreen))
			throw runtime_error("Vendor toggle fullscreen failed");
	}
}

void Engine::config()
{
	ifstream f = ifstream("Config.ini");
	if (f.fail())
		return;
	SerializationInfoParser sip(f);
	if (!sip.parse()) {
		Console::error("Config.ini read failed");
		return;
	}
	f.close();
	SerializationInfo& conf = sip.infos[0];
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
	float screenWidth = engineConfig.screenWidth, screenHeight = engineConfig.screenHeight;
	conf.get(".screenWidth", screenWidth);
	engineConfig.screenWidth = screenWidth;
	conf.get(".screenHeight", screenHeight);
	engineConfig.screenHeight = screenHeight;
}

void SetStyleColors(ImGuiStyle* dst = 0)
{
	ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
	ImVec4* colors = style->Colors;
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

void Engine::setup()
{
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
#if ENABLE_PHYSICS
	if (!PhysicalWorld::init())
		throw runtime_error("Physics Engine init failed");
#endif

	setViewportSize(engineConfig.screenWidth, engineConfig.screenHeight);
	RECT rect;
	GetWindowRect(GetDesktopWindow(), &rect);
	windowContext.fullscreenSize = { rect.right - rect.left, rect.bottom - rect.top };

	/*----- Vendor window setup -----*/
	{
		if (!vendor.windowSetup(engineConfig, windowContext))
			throw runtime_error("Vendor window setup failed");
	}

	createWindow(windowContext.screenSize.x, windowContext.screenSize.y);

	world.input.setHWND(windowContext._hwnd);
	//ShowWindow(_hwnd, SW_HIDE);

	/*----- Vendor setup -----*/
	{
		if (!vendor.setup(engineConfig, windowContext))
			throw runtime_error("Vendor setup failed");
	}

	//if (!alutInit(NULL, NULL)) {
	//	//throw runtime_error("ALUT init failed");
	//}

	// Setup Dear ImGui context
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

	io.Fonts->AddFontFromFileTTF("Engine/Fonts/arialuni.ttf", 20, NULL, io.Fonts->GetGlyphRangesChineseFull());
	io.Fonts->AddFontFromFileTTF("Engine/Fonts/ChakraPetch-Light.ttf", 72, NULL, io.Fonts->GetGlyphRangesChineseFull());

	// Setup Dear ImGui style
	SetStyleColors();

	/*----- Vendor ImGui init -----*/
	{
		if (!vendor.imGuiInit(engineConfig, windowContext))
			throw runtime_error("Vendor ImGui init failed");
	}

	loadAssets(engineConfig.loadDefaultAsset, engineConfig.loadEngineAsset, engineConfig.loadContentAsset);

	if (engineConfig.guiOnly) {
		world.setGUIOnly(true);
		InitialTool();
	}
	else
		InitialWorld();

	if (engineConfig.fullscreen)
		toggleFullscreen();
}

void Engine::start()
{
	world.begin();
	bool init = false, show = false;
	Timer timer;
	MSG msg = {};
	while (WM_QUIT != msg.message) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			engineMainLoop();
			if (!init) {
				if (!show) {
					ShowWindow(windowContext._hwnd, SW_SHOWDEFAULT);
					SetTopWindow(windowContext._hwnd);
					show = true;
				}
			}
			else
				init = true;
		}

		timer.record("DeltaTime");
		Console::getTimer("DeltaTime") = timer;
		timer.reset();
	}
}

void Engine::clean()
{
#if ENABLE_PHYSICS
	world.physicalWorld.physicsScene->release();
	PhysicalWorld::release();
#endif

	IVendor& vendor = VendorManager::getInstance().getVendor();

	/*----- Vendor ImGui shutdown -----*/
	{
		if (!vendor.imGuiShutdown(engineConfig, windowContext))
			throw runtime_error("Vendor ImGui shutdown failed");
	}

	ImGui::DestroyContext();

	/*----- Vendor clean -----*/
	{
		if (!vendor.clean(engineConfig, windowContext))
			throw runtime_error("Vendor clean failed");
	}

	//alutExit();
}

void resizeWindow(int width, int height)
{
	Engine::setViewportSize(width, height);
	/*----- Vendor resize window -----*/
	{
		if (!VendorManager::getInstance().getVendor().resizeWindow(Engine::engineConfig, Engine::windowContext, width, height))
			throw runtime_error("Vendor resie window failed");
	}
}

void engineMainLoop()
{
	Timer timer;
	if (world.willQuit()) {
		CloseWindow(Engine::windowContext._hwnd);
	}
	timer.record("PollEvent");
	world.tick(0);
	world.afterTick();
	timer.record("CPU");

	/*----- Vendor swap -----*/
	{
		if (!VendorManager::getInstance().getVendor().swap(Engine::engineConfig, Engine::windowContext))
			throw runtime_error("Vendor swap failed");
	}

	timer.record("GPU Wait");
	Console::getTimer("Engine") = timer;
}

LRESULT wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	/*----- Vendor envoke WndProc -----*/
	{
		if (VendorManager::getInstance().getVendor().getWndProcFunc()(hWnd, message, wParam, lParam))
			return 1;
	}
	static bool m_Minimized = false;
	static bool m_Maximized = false;
	static bool m_Resizing = false;
	static unsigned int width;
	static unsigned int height;
	switch (message)
	{
	case WM_SIZE:
		width = LOWORD(lParam);
		height = HIWORD(lParam);
		if (wParam == SIZE_MINIMIZED)
		{
			m_Minimized = true;
			m_Maximized = false;
		}
		else if (wParam == SIZE_MAXIMIZED) {
			m_Minimized = false;
			m_Maximized = true;
			resizeWindow(width, height);
		}
		else if (wParam == SIZE_RESTORED) {
			if (m_Minimized) {
				m_Minimized = false;
				resizeWindow(width, height);
			}
			else if (m_Maximized) {
				m_Maximized = false;
				resizeWindow(width, height);
			}
			else if (m_Resizing) {

			}
			else {
				resizeWindow(width, height);
			}
		}
		return 0;
	case WM_ENTERSIZEMOVE:
		m_Resizing = true;
		return 0;
	case WM_EXITSIZEMOVE:
		m_Resizing = false;
		resizeWindow(width, height);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		break;
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 960;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 480;
		return 0;
	}
	return 0;
}
