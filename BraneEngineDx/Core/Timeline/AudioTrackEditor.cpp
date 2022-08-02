#include "AudioTrackEditor.h"
#include "../GUI/GUIUtility.h"
#include "../Engine.h"

RegistEditor(AudioTrack);

AudioTrackEditor::AudioTrackEditor()
{
	trackColor = Color{ 0xb3, 0x62, 0x05 };
}

void AudioTrackEditor::setInspectedObject(void* object)
{
    audioTrack = dynamic_cast<AudioTrack*>((Base*)object);
    TimelineTrackEditor::setInspectedObject(audioTrack);
}

void AudioTrackEditor::onTrackContentGUI(EditorInfo& info, TimelineEditorInfo& timelineInfo, const ImVec2& min_p, const ImVec2& max_p)
{
    Transform* transform = audioTrack->getTransform();

    float volume = audioTrack->getMaxVolume();
    bool mute = audioTrack->isMute();
    const char* icon = mute ? ICON_FA_VOLUME_OFF : ICON_FA_VOLUME_UP;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 2, 2 });
    if (ImGui::Button(icon, { timelineInfo.barHeight, timelineInfo.barHeight })) {
        audioTrack->setMute(!mute);
    }
    ImGui::SetNextItemWidth(timelineInfo.barHeight);
    if (ImGui::DragFloat("##Volume", &volume, 0.01, 0, 1)) {
        audioTrack->setMaxVolume(volume);
    }

    ImGui::SetNextItemWidth(timelineInfo.trackViewWidth * 0.3f);
    ImGui::ObjectCombo("Transform##AudioTrackTransformTarget", (Object*&)transform, info.world->getObject(), "Transform");
    audioTrack->setTransform(transform);

    ImGui::Spring();

    Asset* selectedAsset = EditorManager::getSelectedAsset();
    bool disabled = selectedAsset == NULL || selectedAsset->assetInfo.type != "AudioData";
    ImGui::BeginDisabled(disabled);
    if (ImGui::ButtonEx(ICON_FA_PLUS, { timelineInfo.barHeight, timelineInfo.barHeight })) {
        AudioData* data = (AudioData*)selectedAsset->load();
        AudioPlayable* playable = new AudioPlayable();
        playable->setAudio(data);
        float startTime = track->startTime + track->duration;
        TimelineClip* clip = audioTrack->addClip(ClipInfo{ startTime, data->duration, playable });
        clip->name = data->name;
        if (timelineInfo.timeline != NULL)
            timelineInfo.timeline->apply();
    }
    ImGui::EndDisabled();
    ImGui::PopStyleVar();
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        if (disabled) {
            ImGui::Text("Add clip with audio '%s'", selectedAsset->name.c_str());
            ImGui::Text(selectedAsset->path.c_str());
        }
        else {
            ImGui::Text("No audio selected");
        }
        ImGui::EndTooltip();
    }
}

void AudioTrackEditor::onTrackInspectGUI(EditorInfo& info)
{
    TimelineTrackEditor::onTrackInspectGUI(info);
    float volume = audioTrack->getMaxVolume();
    if (ImGui::DragFloat("MaxVolume", &volume, 0.01, 0, 1)) {
        audioTrack->setMaxVolume(volume);
    }
    Transform* transform = audioTrack->getTransform();
    ImGui::ObjectCombo("Transform##AudioTrackTransformTarget", (Object*&)transform, info.world->getObject(), "Transform");
    audioTrack->setTransform(transform);
}
