#pragma once

#include "../GUI/UIWindow.h"
#include "GraphEditor.h"

class GraphWindow : public UIWindow
{
public:
	GraphWindow(string name = "Graph", bool defaultShow = false);
	virtual ~GraphWindow();

	void setGraph(Graph* graph);

	virtual void onRenderWindow(GUIRenderInfo& info);

	static void showGraph(GUI& gui, Graph* graph);
protected:
	GraphInfo graphInfo;
	Ref<Graph> graph = NULL;
	Graph* targetGraph = NULL;
	GraphEditor* editor = NULL;
	float inspectViewWidthRadio = 0.25f;
};