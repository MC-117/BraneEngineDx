#include "GUI.h"
#include "UIWindow.h"
#include "Gizmo.h"
#include "../Camera.h"
#include "../Engine.h"
#include "../WUI/WUIMainWindow.h"
#include "GUIUtility.h"

bool GUI::mouseOnUI = false;
bool GUI::anyItemFocus = false;

GUI::GUI() : gizmo("_default_gizmo_")
{
	//ImGuiStyle& style = ImGui::GetStyle();
	//style.WindowPadding = { 0, 0 };
}

GUI::~GUI()
{
}

bool GUI::isMouseOnUI()
{
	return mouseOnUI;
}

bool GUI::isAnyItemFocus()
{
	return anyItemFocus;
}

void GUI::onGUI(RenderInfo& info)
{
	if (uiControls.empty())
		return;

	IVendor& vendor = VendorManager::getInstance().getVendor();
	
	/*----- Vendor ImGui new frame -----*/
	{
		if (!vendor.imGuiNewFrame(Engine::engineConfig, Engine::windowContext))
			throw runtime_error("Vendor ImGui new frame failed");
	}

	GUIRenderInfo _info = { viewSize, sceneBlurTex, info.sceneData, info.renderGraph, *this, info.camera };

	for (auto b = uiControls.begin(), e = uiControls.end(); b != e; b++)
		b->second->onPreAction(_info);
	
	// ImGui Dock Begin
	ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;// ImGuiWindowFlags_NoDocking;
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background 
	// and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("##MainWindow", NULL, window_flags);

	WUIImGuiWindow* wuiWindow = (WUIImGuiWindow*)viewport->PlatformUserData;

	ImVec2 bkCurPos = ImGui::GetCursorPos();
	const ImGuiWindow* window = ImGui::GetCurrentWindow();
	const ImRect titleBarRect = window->TitleBarRect();
	const Vector3f windowPos(window->Pos.x, window->Pos.y);
	wuiWindow->setHitRect(0, WUIWindow::Hit_Caption, BoundBox(
		Vector3f(titleBarRect.Min.x, titleBarRect.Min.y) - windowPos,
		Vector3f(titleBarRect.Max.x, titleBarRect.Max.y) - windowPos));
	ImGui::PushClipRect(titleBarRect.Min, titleBarRect.Max, false);
	ImGui::SetCursorPos(ImVec2(0, 0));
	ImGui::BeginHorizontal("MainTitleHor", titleBarRect.GetSize());
	static Texture2D* logoIcon = getAssetByPath<Texture2D>("Engine/Icons/Logo_40x40.png");
	if (logoIcon) {
		const float padding = 4.0f;
		ImGui::SetCursorPos(ImVec2(padding, padding));
		ImGui::Image(logoIcon->getTextureID(),
			{ titleBarRect.GetHeight() - padding * 2,
			titleBarRect.GetHeight() - padding * 2 });
		ImGui::SetCursorPos(ImVec2(titleBarRect.GetHeight(), 0));
	}
	ImGui::BeginVertical("TitleVer");
	ImGui::Spring();
	ImGui::Text("BraneEngine v%s | Vendor Api : %s", ENGINE_VERSION, vendor.getName().c_str());
	ImGui::Spring();
	ImGui::EndVertical();
	ImGui::Spring();
	ImVec2 buttonSize = { titleBarRect.GetHeight() * 1.2f, titleBarRect.GetHeight() };
	ImVec2 rectMin, rectMax;
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	/*ImGui::Button(ICON_FA_WINDOW_MINIMIZE, buttonSize);
	if (ImGui::IsItemHovered())
		wuiWindow->setHitState(WUIWindow::Hit_MinBox);*/
	ImGui::Button(wuiWindow->isMaximize() ? ICON_FA_WINDOW_RESTORE : ICON_FA_WINDOW_MAXIMIZE, buttonSize);
	rectMin = ImGui::GetItemRectMin();
	rectMax = ImGui::GetItemRectMax();
	wuiWindow->setHitRect(1, WUIWindow::Hit_MaxBox, BoundBox(
		Vector3f(rectMin.x, rectMin.y) - windowPos,
		Vector3f(rectMax.x, rectMax.y) - windowPos));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, 0xe81123ff);
	ImGui::Button(ICON_FA_XMARK, buttonSize);
	rectMin = ImGui::GetItemRectMin();
	rectMax = ImGui::GetItemRectMax();
	wuiWindow->setHitRect(2, WUIWindow::Hit_Close, BoundBox(
		Vector3f(rectMin.x, rectMin.y) - windowPos,
		Vector3f(rectMax.x, rectMax.y) - windowPos));
	ImGui::PopStyleColor(2);
	ImGui::EndHorizontal();
	ImGui::PopClipRect();
	ImGui::SetCursorPos(bkCurPos);

	if (mainControl) {
		UIWindow* windoiw = dynamic_cast<UIWindow*>(mainControl);
		ImGui::BeginChild(windoiw->name.c_str());
		if (windoiw)
			windoiw->onRenderWindow(_info);
		else
			mainControl->render(_info);
		ImGui::EndChild();
	}
	else {
		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		// ImGui Dock End
	}

	ImGui::End();
	ImGui::PopStyleVar(3);

	gizmo.begineWindow();
	gizmo.onGUI(Engine::getCurrentWorld());
	gizmo.endWindow();
	
	focusControl = NULL;

	for (auto b = uiControls.begin(), e = uiControls.end(); b != e; b++) {
		if (b->second->show && b->second != mainControl)
			b->second->render(_info);
		UIWindow* win = dynamic_cast<UIWindow*>(b->second);
		if (win->isFocus())
			focusControl = win;
	}

	gizmo.onRender2D();

	GUIPostInfo postInfo = { focusControl, *this };

	for (auto b = uiControls.begin(), e = uiControls.end(); b != e; b++)
		b->second->onPostAction(postInfo);

	mouseOnUI = ImGui::IsWindowHovered();
	anyItemFocus = ImGui::IsAnyItemFocused();

	//ImGui::ShowDemoWindow();

	gizmo.reset();

	//ImGuiIO& io = ImGui::GetIO();
}

void GUI::render(RenderInfo& info)
{
	gizmo.onRender3D(info);
	ImGui::Render();
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
	}
	if (info.renderGraph)
		info.renderGraph->setImGuiDrawData(ImGui::GetDrawData());
}

void GUI::onSceneResize(Unit2Di size)
{
	viewSize = size;
	for (auto b = uiControls.begin(), e = uiControls.end(); b != e; b++)
		b->second->onSceneResize(size);
}

void GUI::setSceneBlurTex(Texture2D * tex)
{
	sceneBlurTex = tex;
}

void GUI::setMainControl(UIControl* uc)
{
	mainControl = uc;
	if (uc)
		addUIControl(*uc);
}

void GUI::addUIControl(UIControl & uc)
{
	uiControls.insert(pair<string, UIControl*>(uc.name, &uc));
	uc.onAttech(*this);
}

UIControl* GUI::getUIControl(const string & name)
{
	auto re = uiControls.find(name);
	if (re == uiControls.end())
		return NULL;
	return re->second;
}

vector<UIControl*> GUI::getUIControls(const string & name)
{
	vector<UIControl*> re;
	auto range = uiControls.equal_range(name);
	for (auto b = range.first, e = range.second; b != e; b++)
		re.push_back(b->second);
	return re;
}

bool GUI::destroyUIControl(const string & name)
{
	auto range = uiControls.equal_range(name);
	if (range.first == range.second)
		return false;
	uiControls.erase(range.first, range.second);
	return true;
}

Events * GUI::getUIControlEvent(const string & name)
{
	UIControl* ui = getUIControl(name);
	return ui == NULL ? NULL : &ui->events;
}

bool GUI::showUIControl(const string & name)
{
	auto range = uiControls.equal_range(name);
	if (range.first == range.second)
		return false;
	for (auto b = range.first, e = range.second; b != e; b++)
		b->second->show = true;
	return true;
}

bool GUI::hideUIControl(const string & name)
{
	auto range = uiControls.equal_range(name);
	if (range.first == range.second)
		return false;
	for (auto b = range.first, e = range.second; b != e; b++)
		b->second->show = false;
	return true;
}

void GUI::showAllUIControl()
{
	for (auto b = uiControls.begin(), e = uiControls.end(); b != e; b++)
		b->second->show = true;
}

void GUI::hideAllUIControl()
{
	for (auto b = uiControls.begin(), e = uiControls.end(); b != e; b++)
		b->second->show = false;
}

bool GUI::isShowUIControl(const string & name)
{
	auto re = uiControls.find(name);
	if (re == uiControls.end())
		return false;
	return re->second->show;
}

bool GUI::isAnyWindowFocus()
{
	return focusControl != NULL;
}

void GUI::setParameter(const string & name, void * data)
{
	parameters[name] = data;
}

void * GUI::getParameter(const string & name)
{
	auto iter = parameters.find(name);
	if (iter == parameters.end())
		return NULL;
	return iter->second;
}

GUI & GUI::operator+=(UIControl & uc)
{
	addUIControl(uc);
	return *this;
}

GUI & GUI::operator+=(UIControl * uc)
{
	addUIControl(*uc);
	return *this;
}

GUIEngineLoop::GUIEngineLoop(GUI& gui)
	: gui(gui)
{
}

bool GUIEngineLoop::willQuit()
{
	return false;
}

void GUIEngineLoop::init()
{
	
}

void GUIEngineLoop::loop(float deltaTime)
{
	RenderInfo info;
	info.camera = NULL;
	info.renderGraph = NULL;
	info.sceneData = NULL;
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

void GUIEngineLoop::release()
{
}
