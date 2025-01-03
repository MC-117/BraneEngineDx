#include "ObjectPreviewWindow.h"
#include "../Engine.h"
#include "../Editor/Editor.h"
#include "../Asset.h"
#include "../GUI/Gizmo.h"
#include "../GUI/GUIUtility.h"

ObjectPreviewWindow::ObjectPreviewWindow(string name, bool defaultShow)
	: UIWindow(*Engine::getCurrentWorld(), name, defaultShow)
	, editorWorld(name)
{
	editorWorld.camera.clearColor = Color(88, 88, 88, 255);
	//editorWorld.camera.renderTarget.setMultisampleLevel(4);
	editorWorld.getGizmo().setCameraControl(Gizmo::CameraControlMode::Turn);
	editorWorld.begin();
	this->styleVars.insert(make_pair(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 }));
}

ObjectPreviewWindow::~ObjectPreviewWindow()
{
	editorWorld.end();
}

void ObjectPreviewWindow::setSerializable(const SerializationInfo& info)
{
	if (info.serialization == NULL ||
		info.serialization->type == "World")
		return;
	Serializable* serializable = info.serialization->instantiate(info);
	if (serializable == NULL)
		return;
	if (targetObject != NULL)
		targetObject->destroy(true);
	targetObject = dynamic_cast<Object*>(serializable);
	targetObject->deserialize(info);
	editorWorld.addChild(*targetObject);
}

void ObjectPreviewWindow::setAsset(Asset& asset)
{
	Object* object = asset.createObject();
	if (object == NULL)
		return;
	if (targetObject != NULL)
		targetObject->destroy(true);
	targetObject = object;
	editorWorld.addChild(*targetObject);
}

void ObjectPreviewWindow::onUpdateScene(float width, float height)
{
	editorWorld.setViewportSize((int)width, (int)height);
	editorWorld.update();
}

void ObjectPreviewWindow::onToolBarGUI(GUIRenderInfo& info)
{
}

void ObjectPreviewWindow::onInspectorBarGUI(GUIRenderInfo& info)
{
	if (targetObject == NULL)
		return;
	EditorInfo editorInfo = { &info.gui, &editorWorld.getGizmo(), &editorWorld.camera, &editorWorld, false };

	if (ImGui::BeginTabBar("InspectorTab")) {

		if (ImGui::BeginTabItem("Inspector")) {
			Editor* editor = EditorManager::getEditor(*targetObject);
			if (editor != NULL) {
				editor->onGUI(editorInfo);
			}
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Setting")) {
			Editor* editor = EditorManager::getEditor(editorWorld.camera);
			if (editor != NULL) {
				editor->onGUI(editorInfo);
			}
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}

void ObjectPreviewWindow::onSceneGUI(GUIRenderInfo& info, float width, float height)
{
	Texture* texture = NULL;
	if (targetObject == NULL)
		return;
	Editor* editor = EditorManager::getEditor(*targetObject);
	if (editor != NULL) {
		texture = editor->getPreviewTexture({ width, height });
	}
	if (texture == NULL) {
		GUISurface& guiSurface = editorWorld.getGUISurface();
		texture = guiSurface.getSceneTexture();
	}
	if (texture != NULL) {
		texture->bind();
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0, 0 });

		auto list = ImGui::GetWindowDrawList();
		float padding = 0;
		ImVec2 pos = ImGui::GetWindowPos();
		float tw = texture->getWidth();
		float th = texture->getHeight();
		float aspect = tw / th;
		if (aspect < width / height) {
			tw = height * aspect;
			th = height;
		}
		else {
			tw = width;
			th = width / aspect;
		}
		float hpw = (width - tw) / 2.0f;
		float hph = (height - th) / 2.0f;

		unsigned long long id = texture->getTextureID();
		if (id != 0)
			list->AddImage((ImTextureID)id, { pos.x + padding + hpw, pos.y + padding + hph },
				{ pos.x + padding + hpw + tw, pos.y + padding + hph + th });
		ImGui::PopStyleVar(2);
		editorWorld.onGUI(list);
	}
}

void ObjectPreviewWindow::onWindowGUI(GUIRenderInfo& info)
{
	float splitterSize = 3;
	float height = ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y - splitterSize;
	float width = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x - splitterSize;

	float toolBarSize = height * toolBarSizeRate;
	float inspectorSize = width * inspectorSizeRate;
	float screenWidth = width - inspectorSize;
	float screenHeight = height - toolBarSize;

	if (ImGui::Splitter(false, splitterSize, &toolBarSize, &screenHeight, 20, 512)) {
		toolBarSizeRate = toolBarSize / height;
	}

	ImGui::BeginChild("ToolBar", ImVec2(width, toolBarSize));
	onToolBarGUI(info);
	ImGui::EndChild();

	ImGui::BeginChild("ToolBarRest", ImVec2(width, screenHeight));
	if (ImGui::Splitter(true, splitterSize, &screenWidth, &inspectorSize, 512, 20)) {
		inspectorSizeRate = inspectorSize / width;
	}

	onUpdateScene(screenWidth, screenHeight);

	ImGui::BeginChild("Scene", ImVec2(screenWidth, screenHeight));
	onSceneGUI(info, screenWidth, screenHeight);
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("Inspector", ImVec2(inspectorSize - splitterSize, screenHeight));
	onInspectorBarGUI(info);
	ImGui::EndChild();
	
	ImGui::EndChild();
}

void ObjectPreviewWindow::onRender(RenderInfo& info)
{
	UIWindow::onRender(info);
	editorWorld.render(info);
}

void ObjectPreviewWindow::showObject(GUI& gui, const SerializationInfo& info)
{
	ObjectPreviewWindow* win = dynamic_cast<ObjectPreviewWindow*>(gui.getUIControl("ObjectPreviewWindow"));
	if (win == NULL) {
		win = new ObjectPreviewWindow();
		gui.addUIControl(*win);
	}
	win->show = true;
	win->setSerializable(info);
}

void ObjectPreviewWindow::showObject(GUI& gui, Asset& asset)
{
	ObjectPreviewWindow* win = dynamic_cast<ObjectPreviewWindow*>(gui.getUIControl("ObjectPreviewWindow"));
	if (win == NULL) {
		win = new ObjectPreviewWindow();
		gui.addUIControl(*win);
	}
	win->show = true;
	win->setAsset(asset);
}
