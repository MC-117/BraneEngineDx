#include "TimelineClipEditor.h"
#include "TimelineEditorInfo.h"
#include "../GUI/GUIUtility.h"

RegistEditor(TimelineClip);

void TimelineClipEditor::setInspectedObject(void* object)
{
    clip = dynamic_cast<TimelineClip*>((Base*)object);
    BaseEditor::setInspectedObject(clip);
}

void TimelineClipEditor::onGUI(EditorInfo& info)
{
	if (clip == NULL)
		return;
	BaseEditor::onGUI(info);
	ImGui::Header("TimelineClip", { 0, 0 }, 3);
	ImGui::InputText("Name", &clip->name);
	float startTime = clip->startTime;
	float duration = clip->duration;
	float startFrame = startTime * TimelineEditorInfo::frameRate;
	float durationFrame = duration * TimelineEditorInfo::frameRate;
	ImGui::InputFloat("StartTime", &startTime, 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly);
	ImGui::InputFloat("StartFrame", &startFrame, 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly);
	ImGui::InputFloat("Duration", &duration, 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly);
	ImGui::InputFloat("DurationFrame", &durationFrame, 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly);

	Editor* editor = EditorManager::getEditor(*clip->playable);
	if (editor != NULL) {
		editor->onGUI(info);
	}
}
