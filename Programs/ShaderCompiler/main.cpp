#include <filesystem>

#include "resource.h"
#include "Core/Application.h"
#include "Core/Engine.h"
#include "Core/EngineLoop/GUIEngineLoop.h"
#include "Core/Importer/Importer.h"
#include "Core/GUI/ShaderManagerWindow.h"
#include "Core/WUI/WUIControl.h"
#include "Core/WUI/WUIMainWindow.h"

class ShaderApplication : public EngineWindowApplication
{
public:
	ShaderApplication() : EngineWindowApplication(MAKEINTRESOURCE(IDI_ICON1)) {}
	
	virtual void loadAssets()
	{
		ShaderManager::loadDefaultAdapter("Engine/Shaders/Default");

		if (!Material::loadDefaultMaterial()) {
			MessageBox(NULL, "Default material load failed", "Error", MB_OK);
		}

		IImporter::loadFolder("Engine/Shaders");
	}
	
	virtual void initializeEngineLoop()
	{
		GUI::get().setMainControl(&window);
		engine.setEngineLoop(*new GUIOnlyEngineLoop(GUI::get()));
		Engine::get().getEngineLoop()->init();
	}
protected:
	ShaderManagerWindow window;
};

RegisterApplication(ShaderApplication);

// int main(int argc, char** argv) {
// 	using namespace filesystem;
// 	
// 	if (argc > 1) {
// 		path workingSpace = u8path(argv[1]);
// 		if (exists(workingSpace))
// 			current_path(workingSpace);
// 	}
// 	Engine::config();
// 	Engine::windowContext.hinstance = GetModuleHandle(NULL);
//
// 	int retureCode = 0;
//
// 	for (;;) {
// 		if (!exists("Engine")) {
// 			MessageBox(NULL, "Engine Folder Missing", "Error", MB_OK);
// 			retureCode = -1;
// 			break;
// 		}
//
// 		VendorManager::getInstance().instantiateVendor(Engine::engineConfig.vendorName);
//
// 		IVendor& vendor = VendorManager::getInstance().getVendor();
//
// 		WUIControl::registDefaultClass(Engine::windowContext.hinstance);
//
// 		/*----- Vendor window setup -----*/
// 		{
// 			if (!vendor.windowSetup(Engine::engineConfig, Engine::windowContext))
// 				throw runtime_error("Vendor window setup failed");
// 		}
//
// 		imGuiSetup();
// 		/*----- WinAPI Viewport window setup -----*/
// 		Engine::mainWindow.setHInstance(Engine::windowContext.hinstance);
// 		ImGuiViewport& mainViewport = *ImGui::GetMainViewport();
// 		mainViewport.Flags |= ImGuiViewportFlags_NoDecoration;
// 		Engine::mainWindow.initViewport(mainViewport);
// 		//Engine::mainWindow.setClientSize({ (int)Engine::engineConfig.screenWidth, (int)Engine::engineConfig.screenHeight });
// 		//Engine::mainWindow.setText("BraneEngine v" + string(ENGINE_VERSION) + " | Vendor Api: " + vendor.getName());
// 		Engine::windowContext.hwnd = Engine::mainWindow.create();
//
// 		/*----- Vendor setup -----*/
// 		{
// 			if (!vendor.setup(Engine::engineConfig, Engine::windowContext))
// 				throw runtime_error("Vendor setup failed");
// 		}
//
// 		/*----- Vendor ImGui init -----*/
// 		{
// 			if (!vendor.imGuiInit(Engine::engineConfig, Engine::windowContext))
// 				throw runtime_error("Vendor ImGui init failed");
// 		}
//
// 		ShaderManager::loadDefaultAdapter("Engine/Shaders/Default");
//
// 		if (!Material::loadDefaultMaterial()) {
// 			MessageBox(NULL, "Default material load failed", "Error", MB_OK);
// 			retureCode = -1;
// 			break;
// 		}
//
// 		IImporter::loadFolder("Engine/Shaders");
//
// 		GUI gui;
// 		ShaderManagerWindow window;
// 		window.show = true;
// 		WUIImGuiWindow* wui = NULL;
//
// 		auto loopFunc = [](ShaderManagerWindow* window, GUI* gui, WUIImGuiWindow* wui)
// 		{
// 			if (!window->show)
// 				wui->close();
// 			IVendor& vendor = VendorManager::getInstance().getVendor();
// 			/*----- Vendor ImGui new frame -----*/
// 			{
// 				if (!vendor.imGuiNewFrame(Engine::engineConfig, Engine::windowContext))
// 					throw runtime_error("Vendor ImGui new frame failed");
// 			}
//
// 			GUIRenderInfo info = { {}, NULL, NULL, NULL, *gui, NULL };
// 		
// 			window->onPreAction(info);
// 			window->render(info);
// 			
// 			ImGui::Render();
// 			if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
// 			{
// 				ImGui::UpdatePlatformWindows();
// 			}
// 		};
//
// 		for (int i = 0; i < 2; i++)
// 		{
// 			loopFunc(&window, &gui, wui);
// 			if (Engine::mainWindow.controls.empty())
// 				continue;
// 			wui = dynamic_cast<WUIImGuiWindow*>(Engine::mainWindow.controls[0]);
// 		}
//
// 		if (wui == NULL)
// 			break;
//
// 		wui->loopDelegate = bind(loopFunc, &window, &gui, wui);
//
// 		wui->doModel();
//
// 		break;
// 	}
//
// 	Engine::clean();
// 	return retureCode;
// }
