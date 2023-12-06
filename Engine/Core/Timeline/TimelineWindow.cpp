#include "TimelineWindow.h"
#include "../Engine.h"

TimelineWindow::TimelineWindow(string name, bool defaultShow)
	: UIWindow(*(Object*)NULL, name, defaultShow)
{
}

void TimelineWindow::setTimeline(Timeline* timeline)
{
	editor.setInspectedObject(timeline);
}

void TimelineWindow::setTimelinePlayer(TimelinePlayer* player)
{
	editor.setPlayer(player);
}

void TimelineWindow::onWindowGUI(GUIRenderInfo& info)
{
	Object* selectObject = EditorManager::getSelectedObject();
	TimelinePlayer* player = dynamic_cast<TimelinePlayer*>(selectObject);

	if (player != NULL) {
		editor.setPlayer(player);
	}

	EditorInfo editorInfo = { &info.gui, info.gizmo, info.camera, Engine::getCurrentWorld() };
	editor.onTimelineGUI(editorInfo);
}

void TimelineWindow::showTimeline(GUI& gui, Timeline* timeline)
{
	TimelineWindow* win = dynamic_cast<TimelineWindow*>(gui.getUIControl("Timeline"));
	if (win == NULL) {
		win = new TimelineWindow();
		gui.addUIControl(*win);
	}
	win->show = true;
	win->setTimeline(timeline);
}
