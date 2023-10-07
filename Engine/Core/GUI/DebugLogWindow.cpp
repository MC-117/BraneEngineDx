#include "DebugLogWindow.h"
#include "../Engine.h"
#include "../Console.h"

DebugLogWindow::DebugLogWindow(string name, bool defaultShow)
	: UIWindow(*Engine::getCurrentWorld(), name, defaultShow)
{
	startTime = Time::duration();
	backgroundColor = { 0, 0, 0, 0 };
	showCloseButton = false;
	setStyle(ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMouseInputs |
		ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_UnsavedDocument |
		ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
}

void DebugLogWindow::onRenderWindow(GUIRenderInfo & info)
{
	ImGui::SetWindowPos(ImGui::GetMainViewport()->WorkPos, ImGuiCond_Always);
	Time time = Time::duration() - Time(chrono::seconds(10));
	for (auto b = Console::logs.rbegin(), e = Console::logs.rend(); b != e; b++) {
		if (b->timeStamp < time || b->timeStamp < startTime)
			break;
		ImVec4 col;
		switch (b->state)
		{
		case LogState::Log_Normal:
			col = { 0, 0, 1, 1 };
			break;
		case LogState::Log_Warning:
			col = { 1, 1, 0, 1 };
			break;
		case LogState::Log_Error:
			col = { 1, 0, 0, 1 };
			break;
		}
		ImGui::TextColored(col, b->text.c_str());
	}
}
