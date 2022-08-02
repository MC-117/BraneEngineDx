#include "ShotTrackEditor.h"
#include "../GUI/GUIUtility.h"

RegistEditor(ShotTrack);

ShotTrackEditor::ShotTrackEditor()
{
    trackColor = Color{ 0xC9, 0x04, 0x76 };
}

void ShotTrackEditor::setInspectedObject(void* object)
{
    shotTrack = dynamic_cast<ShotTrack*>((Base*)object);
    TimelineTrackEditor::setInspectedObject(shotTrack);
}

void ShotTrackEditor::onTrackContentGUI(EditorInfo& info, TimelineEditorInfo& timelineInfo, const ImVec2& min_p, const ImVec2& max_p)
{
    TimelineTrackEditor::onTrackContentGUI(info, timelineInfo, min_p, max_p);

    ImGui::Spring();

    Asset* selectedAsset = EditorManager::getSelectedAsset();
    AnimationClipData* data = NULL;
    bool disabled = selectedAsset == NULL || selectedAsset->assetInfo.type != "AnimationClipData";
    if (!disabled) {
        data = (AnimationClipData*)selectedAsset->load();
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 2, 2 });
    ImGui::BeginDisabled(disabled);
    if (ImGui::ButtonEx(ICON_FA_PLUS, { timelineInfo.barHeight, timelineInfo.barHeight })) {
        ShotPlayable* playable = new ShotPlayable();
        playable->setAnimation(data);
        float startTime = track->startTime + track->duration;
        float duration = data == NULL ? (150 / TimelineEditorInfo::frameRate) : data->duration;
        TimelineClip* clip = shotTrack->addClip(ClipInfo{ startTime, duration, playable });
        if (data != NULL)
            clip->name = data->name;
        if (timelineInfo.timeline != NULL)
            timelineInfo.timeline->apply();
    }
    ImGui::EndDisabled();
    ImGui::PopStyleVar();
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        if (data == NULL) {
            ImGui::Text("Add shot");
        }
        else {
            ImGui::Text("Add shot with animation '%s'", selectedAsset->name.c_str());
            ImGui::Text(selectedAsset->path.c_str());
        }
        ImGui::EndTooltip();
    }
}

void ShotTrackEditor::onTrackInspectGUI(EditorInfo& info)
{
}
