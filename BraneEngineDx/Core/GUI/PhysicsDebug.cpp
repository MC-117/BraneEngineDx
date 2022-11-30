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

void PhysicsDebug::onRenderWindow(GUIRenderInfo& info)
{
	ImGui::SetWindowSize(ImVec2(info.viewSize.x, info.viewSize.y));
	ImGui::SetWindowPos(ImVec2(0, 0));

	World& world = *Engine::getCurrentWorld();
	PhysicalWorld::drawDedug(ImGui::GetBackgroundDrawList(), world.physicalWorld.physicsScene->getRenderBuffer(), *info.camera);
}
