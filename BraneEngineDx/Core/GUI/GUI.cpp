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
	ImGui::Begin("DockSpace Demo", NULL, window_flags);
	ImGui::PopStyleVar();
	ImGui::PopStyleVar(2);

	WUIImGuiWindow* wuiWindow = (WUIImGuiWindow*)viewport->PlatformUserData;

	wuiWindow->setHitState(WUIWindow::Hit_Client);

	ImVec2 bkCurPos = ImGui::GetCursorPos();
	const ImGuiWindow* window = ImGui::GetCurrentWindow();
	const ImRect titleBarRect = window->TitleBarRect();
	if (ImGui::IsMouseHoveringRect(titleBarRect.Min, titleBarRect.Max, false))
		wuiWindow->setHitState(WUIWindow::Hit_Caption);
	ImGui::PushClipRect(titleBarRect.Min, titleBarRect.Max, false);
	ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));
	ImGui::BeginHorizontal("MainTitleHor", titleBarRect.GetSize());
	ImGui::Spring();
	ImVec2 buttonSize = { titleBarRect.GetHeight() * 1.2f, titleBarRect.GetHeight() };
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	/*ImGui::Button(ICON_FA_WINDOW_MINIMIZE, buttonSize);
	if (ImGui::IsMouseHoveringRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), false))
		wuiWindow->setHitState(WUIWindow::Hit_MinBox);*/
	ImGui::Button(wuiWindow->isMaximize() ? ICON_FA_WINDOW_RESTORE : ICON_FA_WINDOW_MAXIMIZE, buttonSize);
	if (ImGui::IsMouseHoveringRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), false))
		wuiWindow->setHitState(WUIWindow::Hit_MaxBox);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, 0xe81123ff);
	ImGui::Button(ICON_FA_XMARK, buttonSize);
	if (ImGui::IsMouseHoveringRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), false))
		wuiWindow->setHitState(WUIWindow::Hit_Close);
	ImGui::PopStyleColor(2);
	ImGui::EndHorizontal();
	ImGui::PopClipRect();
	ImGui::SetCursorPos(bkCurPos);

	// DockSpace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	ImGui::End();
	// ImGui Dock End

	gizmo.begineWindow();
	gizmo.onGUI(Engine::getCurrentWorld());
	gizmo.endWindow();
	
	focusControl = NULL;

	for (auto b = uiControls.begin(), e = uiControls.end(); b != e; b++)
		b->second->onPreAction(_info);

	for (auto b = uiControls.begin(), e = uiControls.end(); b != e; b++) {
		if (b->second->show)
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
