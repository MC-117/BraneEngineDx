#include "AnimationTrackEditor.h"
#include "../GUI/GUIUtility.h"
#include "../Engine.h"

RegistEditor(AnimationTrack);

AnimationTrackEditor::AnimationTrackEditor()
{
    trackColor = Color{ 0x03, 0xa7, 0x7a };
}

void AnimationTrackEditor::setInspectedObject(void* object)
{
    animTrack = dynamic_cast<AnimationTrack*>((Base*)object);
    TimelineClipTrackEditor::setInspectedObject(animTrack);
}

void AnimationTrackEditor::onTrackContentGUI(EditorInfo& info, TimelineEditorInfo& timelineInfo, const ImVec2& min_p, const ImVec2& max_p)
{
    SkeletonMeshActor* actor = animTrack->getActor();
    ImGui::ObjectCombo("Target##AnimationTarget", (Object*&)actor, info.world->getObject(), "SkeletonMeshActor");
    animTrack->setActor(actor);

    ImGui::Spring();

    Asset* selectedAsset = EditorManager::getSelectedAsset();
    bool disabled = selectedAsset == NULL || selectedAsset->assetInfo.type != "AnimationClipData";
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 2, 2 });
    ImGui::BeginDisabled(disabled);
    if (ImGui::ButtonEx(ICON_FA_PLUS, { timelineInfo.barHeight, timelineInfo.barHeight })) {
        AnimationClipData* data = (AnimationClipData*)selectedAsset->load();
        AnimationPlayable* playable = new AnimationPlayable();
        playable->setAnimation(data);
        float startTime = track->startTime + track->duration;
        TimelineClip* clip = animTrack->addClip(ClipInfo{ startTime, data->duration, playable });
        clip->name = data->name;
        if (timelineInfo.timeline != NULL)
            timelineInfo.timeline->apply();
    }
    ImGui::EndDisabled();
    ImGui::PopStyleVar();
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        if (disabled) {
            ImGui::Text("Add clip with animation '%s'", selectedAsset->name.c_str());
            ImGui::Text(selectedAsset->path.c_str());
        }
        else {
            ImGui::Text("No animation selected");
        }
        ImGui::EndTooltip();
    }
}

void AnimationTrackEditor::onTrackInspectGUI(EditorInfo& info)
{
    TimelineClipTrackEditor::onTrackInspectGUI(info);
    SkeletonMeshActor* actor = animTrack->getActor();
    if (ImGui::ObjectCombo("Target##AnimationTarget", (Object*&)actor, info.world->getObject(), "SkeletonMeshActor"))
        animTrack->setActor(actor);
}
