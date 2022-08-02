#include "TimelineEditor.h"
#include "../GUI/GUIUtility.h"

#include "TimelineTrackEditor.h"
#include "../Engine.h"

#include <fstream>

RegistEditor(Timeline);

void TimelineEditor::setInspectedObject(void* object)
{
	timeline = dynamic_cast<Timeline*>((Base*)object);
	BaseEditor::setInspectedObject(timeline);
	if (timeline != NULL) {
		if (player == NULL) {
			if (internalPlayer == NULL) {
				internalPlayer = new TimelinePlayer();
				player = internalPlayer;
			}
			if (player->getTimeline() != timeline)
				player->setTimeline(timeline);
		}
	}
	viewStartTime = 0;
	viewPreviewTime = 0;
	//setViewSize(getFullViewSize());
}

TimelinePlayer* TimelineEditor::getPlayer() const
{
	return player;
}

void TimelineEditor::setPlayer(TimelinePlayer* player)
{
	if (this->player != player) {
		if (player != NULL) {
			Timeline* timeline = player->getTimeline();
			this->player = player;
			if (this->timeline != timeline) {
				setInspectedObject(timeline);
			}
		}
	}
}

void TimelineEditor::onToolBarTopGUI(EditorInfo& info)
{
	float buttonHeight = barHeight - 1;
	ImVec2 buttonSize = { buttonHeight, buttonHeight };
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 2, 2 });
	ImGui::BeginHorizontal("ToolBarHorizontal");

	if (ImGui::ButtonEx(ICON_FA_FILE, buttonSize)) {
		Timeline* newTimeline = new Timeline();
		setInspectedObject(newTimeline);
	}
	
	ImGui::BeginDisabled(timeline == NULL);
	if (ImGui::ButtonEx(ICON_FA_SAVE, buttonSize)) {
		thread td = thread([](Timeline* tar) {
			FileDlgDesc desc;
			desc.title = "Save Timeline";
			desc.filter = "timeline(*.timeline)|*.timeline";
			desc.initDir = "./Content";
			desc.defFileExt = "timeline";
			desc.save = true;
			if (openFileDlg(desc)) {
				if (AssetManager::saveAsset(*tar, desc.filePath) == NULL) {
					MessageBox(NULL, "Serialize failed", "Error", MB_OK);
					return;
				}
				else {
					MessageBox(NULL, "Complete", "Info", MB_OK);
				}
			}
		}, timeline);
		td.detach();
	}
	if (ImGui::ButtonEx(ICON_FA_BACKWARD, buttonSize)) {
		viewPreviewTime = 0;
	}
	if (player == NULL) {
		ImGui::ButtonEx(ICON_FA_PLAY, buttonSize);
	}
	else {
		if (player->isPlay()) {
			if (ImGui::ButtonEx(ICON_FA_PAUSE, buttonSize)) {
				player->pause();
			}
		}
		else {
			if (ImGui::ButtonEx(ICON_FA_PLAY, buttonSize)) {
				player->play();
			}
		}
	}
	if (ImGui::ButtonEx(ICON_FA_FORWARD, buttonSize)) {
		viewPreviewTime = getFrameCount() / frameRate;
	}

	ImGui::Spring();
	ImGui::EndDisabled();

	if (ImGui::BeginPopup("##TrackPopup"))
	{
		vector<Serialization*> trackTypes;
		TimelineTrack::TimelineTrackSerialization::serialization.getChildren(trackTypes);
		for (int i = 0; i < trackTypes.size(); i++) {
			Serialization* serialization = trackTypes[i];
			ImGui::PushID(i);
			if (ImGui::BeginMenu(serialization->type.c_str())) {
				if (lastOpenTimeTrackSerialization != serialization) {
					lastOpenTimeTrackSerialization = serialization;
					newTimelineTrackName = serialization->type;
				}
				ImGui::InputText("Name", &newTimelineTrackName);
				if (ImGui::Button("Add", { -1, 36 })) {
					TimelineTrack* track = timeline->createTrack(*serialization);
					if (track != NULL) {
						track->name = newTimelineTrackName;
						timeline->apply();
					}
				}
				ImGui::EndMenu();
			}
			ImGui::PopID();
		}
		ImGui::EndPopup();
	}

	ImGui::BeginDisabled(timeline == NULL);
	if (ImGui::ButtonEx(ICON_FA_PLUS, buttonSize)) {
		ImGui::OpenPopup("##TrackPopup");
	}
	ImGui::EndDisabled();

	ImGui::EndHorizontal();
	ImGui::PopStyleVar();
}

void TimelineEditor::onTimeBarGUI(EditorInfo& info)
{
	auto drawList = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImVec2 size = { clipViewWidth, barHeight };
	float height = size.y * 0.8f;

	float viewSize = getViewSize();
	float framePerUnit = getFramePreUnit();

	float unitSize = framePerUnit / viewSize * unitRefSize;

	float unitStart = viewStartTime * frameRate / framePerUnit;

	int unit = ceil(unitStart);
	int count = size.x / unitSize;
	float offset = (unitStart - floor(unitStart)) * unitSize;
	for (int i = 0; i <= count; i++, unit++, offset += unitSize) {
		bool isIndex = (unit % 10) == 0;
		bool isHalf = (unit % 5) == 0;
		float x = pos.x + offset;
		float h = height * (isIndex ? 1.0f : (isHalf ? 0.5f : 0.25f));
		drawList->AddLine({ x, pos.y + size.y }, { x, pos.y + size.y - h }, 0xFF606060, 1);
		if (isIndex) {
			char str[64] = { 0 };
			sprintf_s(str, "%d", (int)(unit * framePerUnit));
			drawList->AddText({ x + 3.0f, pos.y }, 0xFFBBBBBB, str);
		}
		windowDrawList->AddLine({ x, pos.y + size.y }, { x, pos.y + size.y + scrollViewHeight }, 0xFF202020, 1);
	}
}

void TimelineEditor::onClipViewGUI(EditorInfo& info, TimelineTrack& track, const Color& trackColor)
{
	auto drawList = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImVec2 endPos = { pos.x + clipViewWidth, pos.y + trackHeight };
	int framePerUnit = getFramePreUnit();
	bool dragging = ImGui::IsMouseDragging(0);
	ClipOperation operation;
	foregroundDrawList->PushClipRect(pos, endPos);

	for (int i = 0; i < track.clips.size(); i++) {
		TimelineClip* clip = track.clips[i];
		if (clip->duration == 0 ||
			clip->startTime > viewEndTime ||
			((clip->startTime + clip->duration) < viewStartTime))
			continue;
		ImGui::PushID(i);
		float x0 = pos.x + timeToSize(clip->startTime - viewStartTime);
		float w = timeToSize(clip->duration);
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

		ImVec2 textMin = { x0 + timeToSize(clip->blendInDuration), pos.y };
		textMin.x = max(pos.x, textMin.x);
		ImVec2 textMax = { x0 + w - timeToSize(clip->blendOutDuration), endPos.y };
		textMax.x = min(endPos.x, textMax.x);

		ImGui::AddTextCentered(drawList, textMin, textMax, clip->name.c_str());

		if (clip->blendInDuration > 0) {
			float xb0 = x0 + timeToSize(clip->blendInDuration);
			ImVec2 max_pb = { xb0, pos.y + trackHeight };
			drawList->AddRectFilled(min_p_min, max_pb, 0x55AAAAAA);
			drawList->AddLine(min_p_min, max_pb, 0xFFAAAAAA, 1.5);
			drawList->AddLine({ x0, pos.y + trackHeight }, { xb0, pos.y }, 0xFFAAAAAA, 1.5);
		}

		foregroundDrawList->AddRectFilled(min_p_min, min_p_max, 0xFFAAAAAA,
			3, ImDrawCornerFlags_BotLeft | ImDrawCornerFlags_TopLeft);
		foregroundDrawList->AddRectFilled(max_p_min, max_p_max, 0xFFAAAAAA,
			3, ImDrawCornerFlags_BotRight | ImDrawCornerFlags_TopRight);

		bool leftHovering = ImGui::IsMouseHoveringRect(min_p_min, min_p_max);
		bool rightHovering = ImGui::IsMouseHoveringRect(max_p_min, max_p_max);
		bool hovering = ImGui::IsItemHovered();
		bool clicking = ImGui::IsMouseClicked(0);

		if (hovering) {
			ImGui::BeginTooltip();
			float startFrame = clip->startTime * frameRate;
			float durationFrame = clip->duration * frameRate;
			float endFrame = startFrame + durationFrame;
			float blendInFrame = clip->blendInDuration * frameRate;
			float blendOutFrame = clip->blendOutDuration * frameRate;
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
				if (operation.type < ClipOperation::LeftMove) {
					operation.type = ClipOperation::LeftMove;
					operation.clip = clip;
				}
			}
			else if (rightHovering) {
				if (operation.type < ClipOperation::RightMove) {
					operation.type = ClipOperation::RightMove;
					operation.clip = clip;
				}
			}
			else if (hovering) {
				if (operation.type <= ClipOperation::Move) {
					operation.type = ClipOperation::Move;
					operation.clip = clip;
				}
			}
		}
		else if (hovering && clicking) {
			operation.type = ClipOperation::Click;
			operation.clip = clip;
		}
		ImGui::PopID();
	}
	foregroundDrawList->PopClipRect();

	if (operation.clip == NULL && clipOperation.clip != NULL && !dragging) {
		clipOperation.clip = NULL;
		clipOperation.type = ClipOperation::None;
	}

	if (operation.clip != NULL && clipOperation.clip == NULL) {
		clipOperation.clip = operation.clip;
		clipOperation.type = operation.type;
		if (operation.clip != NULL) {
			clipOperation.startFrame = operation.clip->startTime * frameRate;
			clipOperation.durationFrame = operation.clip->duration * frameRate;
		}
	}

	if (clipOperation.clip != NULL) {
		float deltaX = ImGui::GetMouseDragDelta().x;
		float deltaFrame = deltaX / frameSize;
		float startFrame = clipOperation.startFrame;
		float durationFrame = clipOperation.durationFrame;
		float endFrame = startFrame + durationFrame;
		switch (clipOperation.type)
		{
		default:
			EditorManager::selectBase(clipOperation.clip);
		case ClipOperation::LeftMove:
			startFrame += deltaFrame;
			startFrame = round(startFrame / framePerUnit) * framePerUnit;
			if (startFrame >= endFrame) {
				startFrame = round(endFrame / framePerUnit) * framePerUnit - framePerUnit;
			}
			durationFrame = endFrame - startFrame;
			break;
		case ClipOperation::RightMove:
			endFrame += deltaFrame;
			endFrame = round(endFrame / framePerUnit) * framePerUnit;
			if (endFrame <= startFrame) {
				endFrame = round(startFrame / framePerUnit) * framePerUnit + framePerUnit;
			}
			durationFrame = endFrame - startFrame;
			break;
		case ClipOperation::Move:
			startFrame += deltaFrame;
			startFrame = round(startFrame / framePerUnit) * framePerUnit;
			endFrame = startFrame + durationFrame;
			break;
		}

		if (clipOperation.type != ClipOperation::None && clipOperation.type != ClipOperation::Click) {
			clipOperation.clip->startTime = startFrame / frameRate;
			clipOperation.clip->duration = durationFrame / frameRate;

			timeline->apply();
		}
	}
}

void TimelineEditor::onToolBarBottomGUI(EditorInfo& info)
{
}

void TimelineEditor::onClipScrollBarGUI(EditorInfo& info)
{
	auto drawList = ImGui::GetWindowDrawList();
	float fullViewSize = getFullViewSize();
	float frameCount = getFrameCount();
	frameCount = frameCount == 0 ? defaultFrameCount : frameCount;

	float viewStartNorm = viewStartTime * frameRate / frameCount;
	float viewSizeNorm = clipViewWidth / frameSize / frameCount;
	float viewEndNorm = viewStartNorm + viewSizeNorm;

	const ImU32 inactive_color = ImColor{ 0.5f, 0.5f, 0.5f };
	const ImU32 active_color = ImColor{ 0.7f, 0.7f, 0.7f };
	const ImU32 line_color = ImColor{ 0.5f, 0.5f, 0.5f };

	const float padding = 2;
	float radius = barHeight * 0.5 - padding;

	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImVec2 size = { clipViewWidth - (padding + radius) * 2, radius * 2 };
	pos.x += padding + viewStartNorm * size.x;
	pos.y += padding;

	ImVec2 a, b, centerA, centerB;
	ImU32 sliderColor, aColor, bColor;

	ImGui::SetCursorScreenPos(pos);
	ImGui::InvisibleButton("##SliderButtonLeft", { radius * 2, radius * 2 });
	bool activeA = ImGui::IsItemActive();
	bool hoverA = ImGui::IsItemHovered();
	bool drag = ImGui::IsMouseDragging(0);
	if (activeA && drag)
	{
		viewStartNorm += ImGui::GetIO().MouseDelta.x / size.x;
		viewStartNorm = max(viewStartNorm, 0);
		viewSizeNorm = viewEndNorm - viewStartNorm;
		if (viewSizeNorm < 0) {
			viewSizeNorm = 0;
			viewStartNorm = viewEndNorm - viewSizeNorm;
		}
	}
	centerA = { pos.x + radius, pos.y + radius };
	aColor = activeA || hoverA ? active_color : inactive_color;

	ImVec2 _pos = pos;
	_pos.x += viewSizeNorm * size.x;
	ImGui::SetCursorScreenPos(_pos);
	ImGui::InvisibleButton("##SliderButtonRight", { radius * 2, radius * 2 });
	bool activeB = ImGui::IsItemActive() && !activeA;
	bool hoverB = ImGui::IsItemHovered() && !hoverA;
	if (activeB && drag)
	{
		viewEndNorm += ImGui::GetIO().MouseDelta.x / size.x;
		viewEndNorm = min(viewEndNorm, 1);
		viewSizeNorm = viewEndNorm - viewStartNorm;
		if (viewSizeNorm < 0) {
			viewSizeNorm = 0;
			viewEndNorm = viewStartNorm + viewSizeNorm;
		}
	}
	centerB = { _pos.x + radius, _pos.y + radius };
	bColor = activeB || hoverB ? active_color : inactive_color;

	pos.x += radius;
	ImGui::SetCursorScreenPos(pos);
	ImGui::InvisibleButton("##SliderButton", { size.x * viewSizeNorm, size.y });
	bool active = ImGui::IsItemActive() && !activeA && !activeB;
	bool hover = ImGui::IsItemHovered() && !hoverA && !hoverB;
	if (active && drag)
	{
		float delta = ImGui::GetIO().MouseDelta.x / size.x;
		viewStartNorm += delta;
		viewStartNorm = max(viewStartNorm, 0);
		viewEndNorm = viewStartNorm + viewSizeNorm;
		viewEndNorm = min(viewEndNorm, 1);
		viewStartNorm = viewEndNorm - viewSizeNorm;
	}
	a = pos;
	b = { pos.x + size.x * viewSizeNorm, pos.y + size.y };
	sliderColor = active || hover ? active_color : inactive_color;

	drawList->AddRectFilled(a, b, sliderColor);
	drawList->AddCircleFilled(centerA, radius, aColor);
	if (!activeA && !hoverA)
		drawList->AddCircleFilled(centerA, radius * 0.5f, 0xFF000000);
	drawList->AddCircleFilled(centerB, radius, bColor);
	if (!activeB && !hoverB)
		drawList->AddCircleFilled(centerB, radius * 0.5f, 0xFF000000);

	if (drag && (activeA || activeB || active)) {
		frameSize = clipViewWidth / viewSizeNorm / frameCount;
		viewStartTime = viewStartNorm * frameCount / frameRate;
	}
}

void TimelineEditor::onPreviewLineGUI(EditorInfo& info)
{
	float radio = (viewPreviewTime - viewStartTime) / (viewEndTime - viewStartTime);
	if (ImGui::IsMouseHoveringRect(clipViewPos,
		{ clipViewPos.x + clipViewWidth, clipViewPos.y + barHeight }) &&
		(ImGui::IsMouseClicked(0) || ImGui::IsMouseDragging(0))) {
		float pos = ImGui::GetMousePos().x - clipViewPos.x;
		radio = pos / clipViewWidth;
		viewPreviewTime = viewStartTime + (viewEndTime - viewStartTime) * radio;
	}
	if (radio < 0 || radio > 1)
		return;
	float x = clipViewPos.x + clipViewWidth * radio;
	const float pointerHalfWidth = 6;
	foregroundDrawList->AddLine({ x, clipViewPos.y + barHeight },
		{ x, clipViewPos.y + barHeight + scrollViewHeight }, 0xFF888888, 2);
	foregroundDrawList->AddRectFilled({ x - pointerHalfWidth, clipViewPos.y },
		{ x + pointerHalfWidth, clipViewPos.y + barHeight }, 0x78FFFFFF, 2);
}

void TimelineEditor::onTimelineGUI(EditorInfo& info)
{
	if (internalPlayer != NULL) {
		internalPlayer->tick(Time::delta().toSecond());
	}
	if (player != NULL) {
		viewPreviewTime = player->getTime();
	}

	float splitterSize = 3;
	winPos = ImGui::GetWindowPos();
	ImVec2 contentPos = ImGui::GetWindowContentRegionMin();
	float height = ImGui::GetWindowContentRegionMax().y -
		contentPos.y - ImGui::GetStyle().WindowPadding.y;
	scrollViewHeight = height - barHeight * 2;
	float width = ImGui::GetWindowContentRegionMax().x - contentPos.x;
	float viewWidth = width - splitterSize;

	trackViewWidth = viewWidth * trackViewWidthRadio;
	clipViewWidth = viewWidth - trackViewWidth;

	if (ImGui::Splitter(true, splitterSize, &trackViewWidth, &clipViewWidth, 20, 20)) {
		trackViewWidthRadio = trackViewWidth / viewWidth;
	}

	clipViewPos = { winPos.x + contentPos.x + trackViewWidth + splitterSize + ImGui::GetStyle().WindowPadding.y * 0.5f, winPos.y + contentPos.y };

	windowDrawList = ImGui::GetWindowDrawList();
	foregroundDrawList = ImGui::GetForegroundDrawList();
	viewEndTime = viewStartTime + sizeToTime(clipViewWidth);

	ImGui::BeginChild("ToolBarTop", ImVec2(trackViewWidth, barHeight));
	onToolBarTopGUI(info);
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("TimeBar", ImVec2(clipViewWidth, barHeight));
	onTimeBarGUI(info);
	ImGui::EndChild();

	ImGui::BeginChild("ScrollView", ImVec2(width, scrollViewHeight));

	Base* selectBase = EditorManager::getSelectedBase();
	if (selectBase != NULL) {
		TimelineTrack* track = dynamic_cast<TimelineTrack*>(selectBase);
		if (track == NULL) {
			TimelineClip* clip = dynamic_cast<TimelineClip*>(selectBase);
			if (clip == NULL)
				selectBase = NULL;
		}
	}

	if (timeline != NULL) {
		TimelineEditorInfo timelineInfo = *this;
		for (int i = 0; i < timeline->tracks.size(); i++) {
			TimelineTrack* track = timeline->tracks[i];

			ImGui::PushID(i);

			ImGui::BeginChild("TrackView", ImVec2(trackViewWidth, trackHeight), false,
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			TimelineTrackEditor* trackEditor = (TimelineTrackEditor*)EditorManager::getEditor(*track);
			if (trackEditor->onTrackGUI(info, timelineInfo)) {
				EditorManager::selectBase(track);
			}
			Color trackColor = trackEditor == NULL ? Color{ 0x26, 0x26, 0x26 } : trackEditor->getTrackColor();
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::BeginChild("ClipView", ImVec2(clipViewWidth, trackHeight), false,
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			onClipViewGUI(info, *track, trackColor);
			ImGui::EndChild();

			ImGui::PopID();
		}
	}

	Base* newSelectBase = EditorManager::getSelectedBase();

	if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) &&
		ImGui::IsMouseClicked(0) && selectBase == newSelectBase) {
		EditorManager::selectBase(NULL);
	}

	ImGui::EndChild();

	onPreviewLineGUI(info);

	ImGui::BeginChild("ToolBarBottom", ImVec2(trackViewWidth, barHeight));
	onToolBarBottomGUI(info);
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("ClipScrollBar", ImVec2(clipViewWidth, barHeight));
	onClipScrollBarGUI(info);
	ImGui::EndChild();

	if (player != NULL) {
		if (!player->isPlay() && player->getTime() != viewPreviewTime) {
			player->setTime(viewPreviewTime);
		}
	}

	if (Engine::input.getKeyPress(VK_DELETE)) {
		Base* base = EditorManager::getSelectedBase();
		if (dynamic_cast<TimelineClip*>(base) != NULL) {
			timeline->removeClip((TimelineClip*)base);
			EditorManager::selectBase(NULL);
		}
		else if (dynamic_cast<TimelineTrack*>(base) != NULL) {
			timeline->removeTrack((TimelineTrack*)base);
			EditorManager::selectBase(NULL);
		}
	}
}

void TimelineEditor::onGUI(EditorInfo& info)
{
}

float TimelineEditor::getFrameCount() const
{
	return timeline == NULL ? 0 : timeline->getDuration() * frameRate;
}

float TimelineEditor::getFullViewSize() const
{
	float frameCount = getFrameCount();
	frameCount = frameCount == 0 ? defaultFrameCount : frameCount;
	return unitRefSize / (clipViewWidth / frameCount);
}

float TimelineEditor::getViewSize() const
{
	return unitRefSize / frameSize;
}

void TimelineEditor::setViewSize(float size)
{
	frameSize = unitRefSize / size;
}

float TimelineEditor::timeToSize(float time) const
{
	return time * frameRate * frameSize;
}

float TimelineEditor::sizeToTime(float size) const
{
	return size / frameSize / frameRate;
}

int TimelineEditor::getFramePreUnit() const
{
	float viewSize = getViewSize();
	float framePerUnit = floor(viewSize);

	framePerUnit = max(framePerUnit, 1);
	return framePerUnit;
}
