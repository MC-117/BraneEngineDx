#pragma once

#include "../Editor/BaseEditor.h"
#include "../../ThirdParty/ImGui/ImGuiNodeEditor/Builders.h"
#include "../../ThirdParty/ImGui/ImGuiNodeEditor/Widgets.h"

class Graph;

struct GraphInfo
{
	ax::NodeEditor::EditorContext* context = NULL;
	ax::NodeEditor::Utilities::BlueprintNodeBuilder* builder = NULL;
	ax::NodeEditor::PinId newLinkPinID = 0;
	ax::NodeEditor::PinId contextPinID = 0;
	ax::NodeEditor::NodeId contextNodeID = 0;
	ax::NodeEditor::NodeId deleteNodeID = 0;
	list<pair<ax::NodeEditor::PinId, ax::NodeEditor::PinId>> links;
	list<Graph*> graphList;

	virtual ~GraphInfo();

	bool isTargetGraph(Graph* graph) const;

	void tickClear();
	void resetContext();
	void setGraph(Graph* graph);
	void openGraph(Graph* graph);
};

class GraphBaseEditor : public BaseEditor
{
public:
	GraphBaseEditor() = default;
	virtual ~GraphBaseEditor() = default;

	virtual void onInspectGUI(EditorInfo& info);
	virtual void onGraphGUI(EditorInfo& info, GraphInfo& graphInfo);
	virtual void onContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo);
	virtual void onGUI(EditorInfo& info);
};