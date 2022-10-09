#pragma once

#include "../Editor/BaseEditor.h"
#include "TimelineEditorInfo.h"

class TimelineEditor : public BaseEditor, public TimelineEditorInfo
{
public:
	TimelineEditor() = default;
	virtual ~TimelineEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual TimelinePlayer* getPlayer() const;
	virtual void setPlayer(TimelinePlayer* player);

	virtual void onToolBarTopGUI(EditorInfo& info);
	virtual void onTimeBarGUI(EditorInfo& info);
	virtual void onToolBarBottomGUI(EditorInfo& info);
	virtual void onClipScrollBarGUI(EditorInfo& info);

	virtual void onPreviewLineGUI(EditorInfo& info);

	virtual void onTimelineGUI(EditorInfo& info);

	virtual void onGUI(EditorInfo& info);

protected:
	float trackViewWidthRadio = 0.3;

	float scrollViewHeight = 0;

	ImVec2 winPos;
	ImVec2 clipViewPos;

	Serialization* lastOpenTimeTrackSerialization = NULL;
	string newTimelineTrackName;

	TimelinePlayer* internalPlayer;
};