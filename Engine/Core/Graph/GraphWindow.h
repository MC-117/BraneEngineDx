#pragma once

#include "../GUI/UIWindow.h"
#include "GraphEditor.h"
#include "../Editor/Previewer//MaterialPreviewer.h"

class ENGINE_API GraphWindow : public UIWindow
{
public:
	GraphWindow(string name = "Graph", bool defaultShow = false);
	virtual ~GraphWindow();

	void setGraph(Graph* graph);

	virtual void onWindowGUI(GUIRenderInfo& info);
	virtual void onRender(RenderInfo& info);

	static void showGraph(GUI& gui, Graph* graph);
protected:
	MaterialPreviewer previewer;
	GraphInfo graphInfo;
	Ref<Graph> graph = NULL;
	Graph* targetGraph = NULL;
	GraphEditor* editor = NULL;
	float inspectViewWidthRadio = 0.25f;
};
