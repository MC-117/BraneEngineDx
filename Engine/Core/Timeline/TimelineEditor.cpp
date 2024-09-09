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
	if (ImGui::ButtonEx(ICON_FA_XMARK, buttonSize)) {
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
		viewStartNorm = std::max(viewStartNorm, 0.0f);
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
		viewEndNorm = std::min(viewEndNorm, 1.0f);
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
		viewStartNorm = std::max(viewStartNorm, 0.0f);
		viewEndNorm = viewStartNorm + viewSizeNorm;
		viewEndNorm = std::min(viewEndNorm, 1.0f);
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
		auto drawList = ImGui::GetWindowDrawList();
		TimelineEditorInfo& timelineInfo = *this;
		for (int i = 0; i < timeline->tracks.size(); i++) {
			TimelineTrack* track = timeline->tracks[i];
			TimelineClipTrack* clipTrack = dynamic_cast<TimelineClipTrack*>(track);

			ImGui::PushID(i);

			TimelineTrackEditor* trackEditor = (TimelineTrackEditor*)EditorManager::getEditor(*track);
			float trackHeight = trackEditor->getTrackHeight();

			ImGui::BeginChild("TrackView", ImVec2(trackViewWidth, trackHeight), false,
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			if (trackEditor->onTrackGUI(info, timelineInfo)) {
				EditorManager::selectBase(track);
			}
			Color trackColor = trackEditor == NULL ? Color{ 0x26, 0x26, 0x26 } : trackEditor->getTrackColor();
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::BeginChild("ClipView", ImVec2(clipViewWidth, trackHeight), false,
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			trackEditor->onClipGUI(info, timelineInfo);
			ImGui::EndChild();

			const float resizeButtonExtend = 4;
			ImVec2 cursorPos = ImGui::GetCursorScreenPos();
			ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
			ImRect resizeButtonRect = { cursorPos.x,
				cursorPos.y - resizeButtonExtend,
				cursorPos.x + trackViewWidth,
				cursorPos.y };
			ImRect resizeButtonDrawRect = { cursorScreenPos.x,
				cursorScreenPos.y - resizeButtonExtend,
				cursorScreenPos.x + trackViewWidth,
				cursorScreenPos.y };
			ImGuiID resizeButtonID = ImGui::GetID("ResizeButton");
			bool hover = false, held = false;
			
			ImGui::KeepAliveID(resizeButtonID);
			ImGui::ButtonBehavior(resizeButtonRect, resizeButtonID, &hover, &held);
			bool resizing = held && ImGui::IsMouseDragging(0);

			if (hover || held) {
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);

				drawList->AddRectFilled(resizeButtonDrawRect.Min,
					resizeButtonDrawRect.Max, 0xFFFFFFAA);
			}

			if (resizing) {
				trackHeight += ImGui::GetIO().MouseDelta.y;
				trackEditor->setTrackHeight(trackHeight);
			}

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

	if (Engine::getInput().getKeyPress(VK_DELETE)) {
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
