#include "CMDWindow.h"
#include "../../ThirdParty/ImGui/imgui_stdlib.h"
#include "../Engine.h"
#include "../GUI/GUIUtility.h"
#include "../Script/PythonManager.h"

CMDWindow::CMDWindow(string name, bool defaultShow) : UIWindow(*Engine::getCurrentWorld(), name, defaultShow)
{
}

CMDWindow::~CMDWindow()
{
}

template<class T>
T& getListNode(list<T>& l, unsigned int index) {
	if (index >= l.size())
		throw overflow_error("out of range when access list");
	for (auto b = l.begin(), e = l.end(); b != e; b++, index--) {
		if (index == 0)
			return *b;
	}
}

void CMDWindow::onRenderWindow(GUIRenderInfo& info)
{
	if (historyWinSize == 0 && commandWinSize == 0) {
		float height = ImGui::GetWindowHeight();
		historyWinSize = height * 0.7;
		commandWinSize = height * 0.3;
	}

	ImGui::Splitter(false, 2, &historyWinSize, &commandWinSize, 1, 1);

	ImGui::BeginChild("CMDHistory", ImVec2(-1, historyWinSize));
	if (Console::getNewPyLogCount > 0) {
		if (!historyList.empty()) {
			string& output = historyList.back().output;
			int pyLogStart = Console::pyLogs.size() - Console::getNewPyLogCount();
			for (int i = pyLogStart; i < Console::pyLogs.size(); i++) {
				output += (output.empty() ? "" : "\n") + getListNode(Console::pyLogs, i).text;
			}
		}
		scroll = true;
	}
	int id = 0;
	for (auto b = historyList.begin(), e = historyList.end(); b != e; b++, id++) {
		ImGui::PushID(id);
		{
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2, 0.2, 0.2, 0.4));
			float h = ImGui::CalcTextSize(b->command.c_str()).y;
			h += ImGui::GetStyle().FramePadding.y * 2;
			ImGui::InputTextMultiline("##codeline", &b->command, ImVec2(-1, h),
				ImGuiInputTextFlags_ReadOnly);
			ImGui::PopStyleColor();
		}
		if (!b->output.empty())
		{
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2, 0.2, 0.2, 0.2));
			float h = ImGui::CalcTextSize(b->output.c_str()).y;
			h += ImGui::GetStyle().FramePadding.y * 2;
			ImGui::InputTextMultiline("##outputline", &b->output, ImVec2(-1, h),
				ImGuiInputTextFlags_ReadOnly);
			ImGui::PopStyleColor();
		}
		ImGui::PopID();
	}
	if (scroll) {
		ImGui::SetScrollHereY(1);
		scroll = false;
	}
	ImGui::EndChild();

	ImGui::BeginChild("CMD");

	if (ImGui::InputTextMultiline("##commandline", &command, ImVec2(-1, -1),
			ImGuiInputTextFlags_CtrlEnterForNewLine | ImGuiInputTextFlags_EnterReturnsTrue)) {
		string cmd = command;
		trimSpace(cmd);
		if (!cmd.empty()) {
			historyList.emplace_back(History{ cmd, "" });
			PythonManager::run(cmd);
			command = "";
			scroll = true;
		}
	}

	ImGui::EndChild();
}
