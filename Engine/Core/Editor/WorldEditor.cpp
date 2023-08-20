#include "WorldEditor.h"
#include "../Engine.h"
#include "../GUI/GUIUtility.h"

RegistEditor(World);

void WorldEditor::setInspectedObject(void* object)
{
	world = dynamic_cast<World*>((Object*)object);
	TransformEditor::setInspectedObject(world);
}

void WorldEditor::onWorldGUI(EditorInfo& info)
{
	Vector3f gravity = world->physicalWorld.getGravity();
	if (ImGui::DragFloat3("Gravity", gravity.data(), 0.01f)) {
		world->physicalWorld.setGravity(gravity);
	}
	int maxFPS = Engine::engineConfig.maxFPS;
	if (ImGui::DragInt("MaxFPS", &maxFPS)) {
		maxFPS = max(maxFPS, 0);
		Engine::engineConfig.maxFPS = maxFPS;
	}
	ImGui::Checkbox("VSnyc", &Engine::engineConfig.vsnyc);
	if (ImGui::CollapsingHeader("RenderGraph")) {
		Editor* editor = EditorManager::getEditor(*world->renderPool.renderGraph);
		if (editor)
			editor->onGUI(info);
	}
}

void WorldEditor::onPersistentGizmo(GizmoInfo& info)
{
	TransformEditor::onPersistentGizmo(info);

	for (auto& AABB : debugAABBs) {
		info.gizmo->drawAABB(AABB, Matrix4f::Identity(), Color(0.0f, 1.0f, 0.0f));
	}
}

void WorldEditor::onDetailGUI(EditorInfo& info)
{
	TransformEditor::onDetailGUI(info);
	if (ImGui::CollapsingHeader("World")) {
		onWorldGUI(info);
	}
	if (ImGui::CollapsingHeader("Debug")) {
		int index = 0;
		for (auto& AABB : debugAABBs) {
			string name = "AABB" + to_string(index);
			ImGui::PushID(name.c_str());
			ImGui::BeginGroupPanel(name.c_str());
			ImGui::InputFloat3("Min", AABB.minPoint.data());
			ImGui::InputFloat3("Max", AABB.maxPoint.data());
			ImGui::EndGroupPanel();
			ImGui::PopID();
			index++;
		}
		if (ImGui::Button(ICON_FA_PLUS))
		{
			debugAABBs.emplace_back();
		}
	}
}
