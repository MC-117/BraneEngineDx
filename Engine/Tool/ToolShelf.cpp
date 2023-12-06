#include "ToolShelf.h"
#include "../Core/Engine.h"

ToolShelf::ToolShelf(string name, bool defaultShow)
	: UIWindow(*Engine::getCurrentWorld(), name, defaultShow)
{
}

void ToolShelf::registTool(UIWindow & tool)
{
	tools.push_back({ &tool, false });
	
}

void ToolShelf::onWindowGUI(GUIRenderInfo & info)
{
	for (int i = 0; i < tools.size(); i++) {
		if (!tools[i].second) {
			info.gui.addUIControl(*tools[i].first);
			tools[i].second = true;
			tools[i].first->show = false;
		}
		if (ImGui::Button(tools[i].first->name.c_str(), { -1, 40 })) {
			tools[i].first->setFocus();
		}
	}
	if (ImGui::Button("Quit", { -1, 40 })) {
		Engine::getCurrentWorld()->quit();
	}
	if (ImGui::Button("Restart", { -1, 40 })) {
		Engine::getCurrentWorld()->quit(1);
	}
}
