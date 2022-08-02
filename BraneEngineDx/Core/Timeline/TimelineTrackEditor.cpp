#include "TimelineTrackEditor.h"
#include "../GUI/GUIUtility.h"

RegistEditor(TimelineTrack);

void TimelineTrackEditor::setInspectedObject(void* object)
{
	track = dynamic_cast<TimelineTrack*>((Base*)object);
	BaseEditor::setInspectedObject(track);
}

Color TimelineTrackEditor::getTrackColor() const
{
	return trackColor;
}

bool TimelineTrackEditor::onTrackGUI(EditorInfo& info, TimelineEditorInfo& timelineInfo)
{
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImVec2 size = { timelineInfo.trackViewWidth, timelineInfo.trackHeight };
	ImVec2 endPos = { pos.x + size.x, pos.y + size.y };
	ImVec2 contentPos = { pos.x + 5, pos.y };

	drawList->AddRectFilled(pos, endPos, (ImColor&)(EditorManager::getSelectedBase() == track ?
		viewColor * selectedTintColor : viewColor), 3, ImDrawCornerFlags_BotLeft | ImDrawCornerFlags_TopLeft);
	drawList->AddRectFilled(pos, { pos.x + 5, endPos.y }, (ImColor&)trackColor, 3,
		ImDrawCornerFlags_BotLeft | ImDrawCornerFlags_TopLeft);

	ImGui::SetCursorScreenPos(contentPos);

	ImGui::PushStyleVar(ImGuiStyleVar_LayoutAlign, 0.5f);
	ImGui::BeginHorizontal("TrackContentScopeH", { size.x - 5, size.y });
	onTrackContentGUI(info, timelineInfo, contentPos, endPos);
	ImGui::EndHorizontal();
	ImGui::PopStyleVar();

	ImGui::SetCursorScreenPos(pos);
	bool re = ImGui::InvisibleButton(track->name.c_str(), { -1, -1 });
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();

		if (track->activedClips.empty())
			ImGui::Text("No acitve clip");
		else for (auto b = track->activedClips.begin(), e = track->activedClips.end();
			b != e; b++) {
			ImGui::Text("%s: %f", b->clip->name.c_str(), b->weight);
		}

		ImGui::EndTooltip();
	}
	return re;
}

void TimelineTrackEditor::onTrackContentGUI(EditorInfo& info, TimelineEditorInfo& timelineInfo, const ImVec2& min_p, const ImVec2& max_p)
{
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImGui::AddTextCentered(drawList, min_p, max_p, track->name.c_str());
}

void TimelineTrackEditor::onTrackInspectGUI(EditorInfo& info)
{
	ImGui::Header(track->getSerialization().type.c_str(), { 0, 0 }, 3);
	ImGui::InputText("Name", &track->name);
	float startTime = track->startTime;
	float duration = track->duration;
	float startFrame = startTime * TimelineEditorInfo::frameRate;
	float durationFrame = duration * TimelineEditorInfo::frameRate;
	ImGui::InputFloat("StartTime", &startTime, 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly);
	ImGui::InputFloat("StartFrame", &startFrame, 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly);
	ImGui::InputFloat("Duration", &duration, 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly);
	ImGui::InputFloat("DurationFrame", &durationFrame, 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly);
}

void TimelineTrackEditor::onGUI(EditorInfo& info)
{
	if (track == NULL)
		return;
	BaseEditor::onGUI(info);
	onTrackInspectGUI(info);
}
