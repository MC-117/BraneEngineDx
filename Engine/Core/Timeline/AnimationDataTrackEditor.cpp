#include "AnimationDataTrackEditor.h"
#include "../GUI/GUIUtility.h"
#include "../Console.h"

RegistEditor(AnimationDataTrack);

AnimationDataTrackEditor::AnimationDataTrackEditor()
{
	trackColor = Color{ 0x03, 0xa7, 0x7a };
    trackHeight = 100;
    trackMinHeight = 100;
}

void AnimationDataTrackEditor::setInspectedObject(void* object)
{
    animDataTrack = dynamic_cast<AnimationDataTrack*>((Base*)object);
    TimelineTrackEditor::setInspectedObject(animDataTrack);
}

bool AnimationDataTrackEditor::onTrackGUI(EditorInfo& info, TimelineEditorInfo& timelineInfo)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 size = { timelineInfo.trackViewWidth, trackHeight };
    ImVec2 endPos = { pos.x + size.x, pos.y + size.y };
    ImVec2 contentPos = { pos.x + 5, pos.y };

    drawList->AddRectFilled(pos, endPos, (ImColor&)(EditorManager::getSelectedBase() == track ?
        viewColor * selectedTintColor : viewColor), 3, ImDrawCornerFlags_BotLeft | ImDrawCornerFlags_TopLeft);
    drawList->AddRectFilled(pos, { pos.x + 5, endPos.y }, (ImColor&)trackColor, 3,
        ImDrawCornerFlags_BotLeft | ImDrawCornerFlags_TopLeft);

    ImGui::SetCursorScreenPos(contentPos);

    onTrackContentGUI(info, timelineInfo, contentPos, endPos);

    ImGui::SetCursorScreenPos(pos);
    bool re = ImGui::InvisibleButton(track->name.c_str(), { -1, -1 });
    return re;
}

bool isIntersected(float aMin, float aMax, float bMin, float bMax)
{
    return !(aMin > bMax || aMax < bMin);
}

void AnimationDataTrackEditor::onTrackContentGUI(EditorInfo& info, TimelineEditorInfo& timelineInfo, const ImVec2& min_p, const ImVec2& max_p)
{
    ImGui::BeginVertical("AnimationDataH");
    SkeletonMeshActor* actor = animDataTrack->getActor();
    if (ImGui::ObjectCombo("Target##AnimationTarget", (Object*&)actor, info.world->getObject(), "SkeletonMeshActor"))
        animDataTrack->setActor(actor);
    ImGui::SameLine();
    if (ImGui::ArrowButton("##AnimationTargetAssign", ImGuiDir_Left)) {
        SkeletonMeshActor* assignActor = dynamic_cast<SkeletonMeshActor*>(EditorManager::getSelectedObject());
        if (assignActor) {
            actor = assignActor;
            animDataTrack->setActor(actor);
        }
    }

    AnimationClipData* data = animDataTrack->getAnimationData();
    Asset* asset = AnimationClipDataAssetInfo::getInstance().getAsset(data);
    if (ImGui::AssetCombo("Data##AnimationData", asset, "AnimationClipData")) {
        data = (AnimationClipData*)asset->load();
        animDataTrack->setAnimationData(data);
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##AnimationDataAssign", ImGuiDir_Left)) {
        Asset* assignAsset = EditorManager::getSelectedAsset();
        if (assignAsset) {
            data = (AnimationClipData*)assignAsset->load();
            animDataTrack->setAnimationData(data);
        }
    }

    trackHeight = trackMinHeight;
    curveClips.clear();
    if (ImGui::CollapsingHeader("Channels")) {
        if (data) {
            bool applyNewCurveName = false;
            float trackHeightMax = ImGui::GetCursorPosY();
            for (auto& item : data->curveNames) {
                bool selected = item.first == selectedCurveName;
                Curve<float, float>* pCurve = &data->curves[item.second];
                ImGui::PushID(item.first.c_str());
                if (ImGui::Selectable(item.first.c_str(), &selected)) {
                    selectedCurveName = item.first;
                    selectedCurve = pCurve;
                }
                float winY = ImGui::GetWindowPos().y;
                float itemMinY = ImGui::GetItemRectMin().y;
                float itemMaxY = ImGui::GetItemRectMax().y;
                float winHeight = ImGui::GetWindowHeight();
                trackHeightMax += itemMaxY - itemMinY;
                if (isIntersected(winY, winY + winHeight, itemMinY, itemMaxY)) {
                    curveClips.push_back({ pCurve, { itemMinY, itemMaxY } });
                }
                if (ImGui::BeginPopupContextItem("CurveItemPop")) {
                    oldCurveName = item.first;
                    ImGui::Text("OldName: %s", oldCurveName.c_str());
                    ImGui::SameLine();
                    if (ImGui::Button("Replace")) {
                        newCurveName = oldCurveName;
                    }
                    ImGui::InputText("NewName", &newCurveName);
                    if (!newCurveName.empty() &&
                        data->curveNames.find(newCurveName) == data->curveNames.end()) {
                        if (ImGui::Button("Apply", { -1, 36 })) {
                            applyNewCurveName = true;
                        }
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopID();
            }

            if (applyNewCurveName) {
                auto iter = data->curveNames.find(oldCurveName);
                float index = iter->second;
                data->curveNames.erase(iter);
                data->curveNames.insert(make_pair(newCurveName, index));
            }

            trackHeight = trackHeightMax;
        }
    }
    ImGui::EndVertical();
}

void AnimationDataTrackEditor::onTrackInspectGUI(EditorInfo& info)
{
    TimelineTrackEditor::onTrackInspectGUI(info);
    SkeletonMeshActor* actor = animDataTrack->getActor();
    if (ImGui::ObjectCombo("Target##AnimationTarget", (Object*&)actor, info.world->getObject(), "SkeletonMeshActor"))
        animDataTrack->setActor(actor);

    Asset* asset = AnimationClipDataAssetInfo::getInstance().getAsset(animDataTrack->getAnimationData());
    if (ImGui::AssetCombo("Data##AnimationData", asset, "AnimationClipData")) {
        AnimationClipData* data = (AnimationClipData*)asset->load();
        animDataTrack->setAnimationData(data);
    }
}

void AnimationDataTrackEditor::onClipGUI(EditorInfo& info, TimelineEditorInfo& timelineInfo)
{
    float frameHeight = ImGui::GetFrameHeight();
    ImGui::Dummy({ -1, frameHeight });
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    float orginX = ImGui::GetCursorScreenPos().x;
    for (auto& clip : curveClips) {
        ImGui::PushID(clip.curve);
        ImRect rect = { orginX, clip.yRange.x, orginX + timelineInfo.clipViewWidth, clip.yRange.y };
        bool foundFrame = false;
        float posYCenter = (clip.yRange.x + clip.yRange.y) * 0.5f;
        for (auto& frame : clip.curve->keys) {
            if (frame.first >= timelineInfo.viewStartTime || frame.first <= timelineInfo.viewEndTime) {
                foundFrame = true;
            }
            else if (foundFrame)
                break;

            float posXCenter = timelineInfo.timeToSize(frame.first - timelineInfo.viewStartTime) + orginX;
            const float radius = 5;

            ImGuiID id = ImGui::GetID(&frame);

            bool hovered = false, held = false;
            ImGui::ButtonBehavior({ posXCenter - radius, posYCenter - radius,
                posXCenter + radius, posYCenter + radius }, id, &hovered, &held);
            
            const ImU32 normalColor = 0xFFFFFFFF;
            const ImU32 hoverColor = 0xFF47B8F6;

            drawList->AddCircleFilled({ posXCenter, posYCenter }, radius, hovered ? hoverColor : normalColor);
            drawList->AddCircle({ posXCenter, posYCenter }, radius, (ImColor&)trackColor);
        }
        ImGui::PopID();
    }
}
