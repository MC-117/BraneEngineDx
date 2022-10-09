#include "TimelineClipTrackEditor.h"
#include "../GUI/GUIUtility.h"

RegistEditor(TimelineClipTrack);

void TimelineClipTrackEditor::setInspectedObject(void* object)
{
	clipTrack = dynamic_cast<TimelineClipTrack*>((Base*)object);
	TimelineTrackEditor::setInspectedObject(clipTrack);
}

bool TimelineClipTrackEditor::onTrackGUI(EditorInfo& info, TimelineEditorInfo& timelineInfo)
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

	ImGui::PushStyleVar(ImGuiStyleVar_LayoutAlign, 0.5f);
	ImGui::BeginHorizontal("TrackContentScopeH", { size.x - 5, size.y });
	onTrackContentGUI(info, timelineInfo, contentPos, endPos);
	ImGui::EndHorizontal();
	ImGui::PopStyleVar();

	ImGui::SetCursorScreenPos(pos);
	bool re = ImGui::InvisibleButton(track->name.c_str(), { -1, -1 });
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();

		if (clipTrack->activedClips.empty())
			ImGui::Text("No acitve clip");
		else for (auto b = clipTrack->activedClips.begin(), e = clipTrack->activedClips.end();
			b != e; b++) {
			ImGui::Text("%s: %f", b->clip->name.c_str(), b->weight);
		}

		ImGui::EndTooltip();
	}
	return re;
}

void TimelineClipTrackEditor::onClipGUI(EditorInfo& info, TimelineEditorInfo& timelineInfo)
{
	auto drawList = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImVec2 endPos = { pos.x + timelineInfo.clipViewWidth, pos.y + trackHeight };
	int framePerUnit = timelineInfo.getFramePreUnit();
	bool dragging = ImGui::IsMouseDragging(0);
	TimelineEditorInfo::ClipOperation operation;

	drawList->ChannelsSplit(2);
	drawList->ChannelsSetCurrent(0);

	for (int i = 0; i < clipTrack->clips.size(); i++) {
		TimelineClip* clip = clipTrack->clips[i];
		if (clip->duration == 0 ||
			clip->startTime > timelineInfo.viewEndTime ||
			((clip->startTime + clip->duration) < timelineInfo.viewStartTime))
			continue;
		ImGui::PushID(i);
		float x0 = pos.x + timelineInfo.timeToSize(clip->startTime - timelineInfo.viewStartTime);
		float w = timelineInfo.timeToSize(clip->duration);
		float x1 = x0 + w;
		ImGui::SetCursorScreenPos({ x0, pos.y });
		ImGui::InvisibleButton(clip->name.c_str(), { w, trackHeight });

		ImVec2 min_p_min = { x0, pos.y };
		ImVec2 min_p_max = { x0 + 5, pos.y + trackHeight };
		ImVec2 max_p_min = { x0 + w - 5, pos.y };
		ImVec2 max_p_max = { x0 + w, pos.y + trackHeight };

		Color selectedTintColor = Color{ 2.0f, 2.0f, 2.0f };
		Color viewColor = Color{ 0x25, 0x25, 0x26 };

		drawList->AddRectFilled(min_p_min, max_p_max, (ImColor&)(EditorManager::getSelectedBase() == clip ?
			viewColor * selectedTintColor : viewColor), 3);
		drawList->AddRectFilled({ min_p_min.x, max_p_max.y - 5 }, max_p_max, (ImColor&)trackColor, 3);

		ImVec2 textMin = { x0 + timelineInfo.timeToSize(clip->blendInDuration), pos.y };
		textMin.x = max(pos.x, textMin.x);
		ImVec2 textMax = { x0 + w - timelineInfo.timeToSize(clip->blendOutDuration), endPos.y };
		textMax.x = min(endPos.x, textMax.x);

		ImGui::AddTextCentered(drawList, textMin, textMax, clip->name.c_str());

		if (clip->blendInDuration > 0) {
			float xb0 = x0 + timelineInfo.timeToSize(clip->blendInDuration);
			ImVec2 max_pb = { xb0, pos.y + trackHeight };
			drawList->AddRectFilled(min_p_min, max_pb, 0x55AAAAAA);
			drawList->AddLine(min_p_min, max_pb, 0xFFAAAAAA, 1.5);
			drawList->AddLine({ x0, pos.y + trackHeight }, { xb0, pos.y }, 0xFFAAAAAA, 1.5);
		}

		drawList->ChannelsSetCurrent(1);
		drawList->AddRectFilled(min_p_min, min_p_max, 0xFFAAAAAA,
			3, ImDrawCornerFlags_BotLeft | ImDrawCornerFlags_TopLeft);
		drawList->AddRectFilled(max_p_min, max_p_max, 0xFFAAAAAA,
			3, ImDrawCornerFlags_BotRight | ImDrawCornerFlags_TopRight);
		drawList->ChannelsSetCurrent(0);

		bool leftHovering = ImGui::IsMouseHoveringRect(min_p_min, min_p_max);
		bool rightHovering = ImGui::IsMouseHoveringRect(max_p_min, max_p_max);
		bool hovering = ImGui::IsItemHovered();
		bool clicking = ImGui::IsMouseClicked(0);

		if (hovering) {
			ImGui::BeginTooltip();
			float startFrame = clip->startTime * timelineInfo.frameRate;
			float durationFrame = clip->duration * timelineInfo.frameRate;
			float endFrame = startFrame + durationFrame;
			float blendInFrame = clip->blendInDuration * timelineInfo.frameRate;
			float blendOutFrame = clip->blendOutDuration * timelineInfo.frameRate;
			ImGui::Text("Name: %s", clip->name.c_str());
			ImGui::Text("StartTime: %f sec, %f frame", clip->startTime, startFrame);
			ImGui::Text("Duration: %f sec, %f frame", clip->duration, durationFrame);
			ImGui::Text("EndTime: %f sec, %f frame", clip->startTime + clip->duration, endFrame);
			ImGui::Text("BlendIn: %f sec, %f frame", clip->blendInDuration, blendInFrame);
			ImGui::Text("BlendOut: %f sec, %f frame", clip->blendOutDuration, blendOutFrame);
			ImGui::EndTooltip();
		}

		if (leftHovering || rightHovering)
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

		if (dragging) {
			if (leftHovering) {
				if (operation.type < TimelineEditorInfo::ClipOperation::LeftMove) {
					operation.type = TimelineEditorInfo::ClipOperation::LeftMove;
					operation.clip = clip;
				}
			}
			else if (rightHovering) {
				if (operation.type < TimelineEditorInfo::ClipOperation::RightMove) {
					operation.type = TimelineEditorInfo::ClipOperation::RightMove;
					operation.clip = clip;
				}
			}
			else if (hovering) {
				if (operation.type <= TimelineEditorInfo::ClipOperation::Move) {
					operation.type = TimelineEditorInfo::ClipOperation::Move;
					operation.clip = clip;
				}
			}
		}
		else if (hovering && clicking) {
			operation.type = TimelineEditorInfo::ClipOperation::Click;
			operation.clip = clip;
		}
		ImGui::PopID();
	}

	if (operation.clip == NULL && timelineInfo.clipOperation.clip != NULL && !dragging) {
		timelineInfo.clipOperation.clip = NULL;
		timelineInfo.clipOperation.type = TimelineEditorInfo::ClipOperation::None;
	}

	if (operation.clip != NULL && timelineInfo.clipOperation.clip == NULL) {
		timelineInfo.clipOperation.clip = operation.clip;
		timelineInfo.clipOperation.type = operation.type;
		if (operation.clip != NULL) {
			timelineInfo.clipOperation.startFrame = operation.clip->startTime * timelineInfo.frameRate;
			timelineInfo.clipOperation.durationFrame = operation.clip->duration * timelineInfo.frameRate;
		}
	}

	if (timelineInfo.clipOperation.clip != NULL) {
		float deltaX = ImGui::GetMouseDragDelta().x;
		float deltaFrame = deltaX / timelineInfo.frameSize;
		float startFrame = timelineInfo.clipOperation.startFrame;
		float durationFrame = timelineInfo.clipOperation.durationFrame;
		float endFrame = startFrame + durationFrame;
		switch (timelineInfo.clipOperation.type)
		{
		default:
			EditorManager::selectBase(timelineInfo.clipOperation.clip);
		case TimelineEditorInfo::ClipOperation::LeftMove:
			startFrame += deltaFrame;
			startFrame = round(startFrame / framePerUnit) * framePerUnit;
			if (startFrame >= endFrame) {
				startFrame = round(endFrame / framePerUnit) * framePerUnit - framePerUnit;
			}
			durationFrame = endFrame - startFrame;
			break;
		case TimelineEditorInfo::ClipOperation::RightMove:
			endFrame += deltaFrame;
			endFrame = round(endFrame / framePerUnit) * framePerUnit;
			if (endFrame <= startFrame) {
				endFrame = round(startFrame / framePerUnit) * framePerUnit + framePerUnit;
			}
			durationFrame = endFrame - startFrame;
			break;
		case TimelineEditorInfo::ClipOperation::Move:
			startFrame += deltaFrame;
			startFrame = round(startFrame / framePerUnit) * framePerUnit;
			endFrame = startFrame + durationFrame;
			break;
		}

		if (timelineInfo.clipOperation.type != TimelineEditorInfo::ClipOperation::None &&
			timelineInfo.clipOperation.type != TimelineEditorInfo::ClipOperation::Click) {
			timelineInfo.clipOperation.clip->startTime = startFrame / timelineInfo.frameRate;
			timelineInfo.clipOperation.clip->duration = durationFrame / timelineInfo.frameRate;

			timelineInfo.timeline->apply();
		}
	}

	drawList->ChannelsMerge();
}
