#pragma once

#include "../GUI/UIWindow.h"
#include "TimelineEditor.h"

class ENGINE_API TimelineWindow : public UIWindow
{
public:
	TimelineWindow(string name = "Timeline", bool defaultShow = false);

	void setTimeline(Timeline* timeline);
	void setTimelinePlayer(TimelinePlayer* player);

	virtual void onWindowGUI(GUIRenderInfo& info);

	static void showTimeline(GUI& gui, Timeline* timeline);
protected:
	TimelineEditor editor;
};