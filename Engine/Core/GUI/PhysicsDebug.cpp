#include "PhysicsDebug.h"
#include "../Engine.h"

PhysicsDebug::PhysicsDebug(string name, bool defaultShow) : UIWindow(*Engine::getCurrentWorld(), name, defaultShow)
{
	setStyle(ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_UnsavedDocument);
}

PhysicsDebug::~PhysicsDebug()
{
}

void PhysicsDebug::onWindowGUI(GUIRenderInfo& info)
{
	ImGui::SetWindowSize(ImGui::GetWindowViewport()->WorkSize);
	ImGui::SetWindowPos(ImGui::GetWindowViewport()->WorkPos);

	World& world = *Engine::getCurrentWorld();
	PhysicalWorld::drawDedug(ImGui::GetBackgroundDrawList(), world.physicalWorld.physicsScene->getRenderBuffer(), *info.camera);
}
