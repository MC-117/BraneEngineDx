#include "WorldEditor.h"
#include "../Engine.h"

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
}

void WorldEditor::onDetailGUI(EditorInfo& info)
{
	TransformEditor::onDetailGUI(info);
	if (ImGui::CollapsingHeader("World")) {
		onWorldGUI(info);
	}
}
