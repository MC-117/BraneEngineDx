#pragma once

#include "GraphPinEditor.h"

class GraphNodeEditor : public GraphBaseEditor
{
public:
	GraphNodeEditor() = default;
	virtual ~GraphNodeEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	void setNodePosition(const Vector2f& position);
	Vector2f getNodePosition() const;

	virtual void onDoubleClicked(EditorInfo& info, GraphInfo& graphInfo);

	virtual void onInspectGUI(EditorInfo & info);
	virtual void onGraphGUI(EditorInfo & info, GraphInfo& graphInfo);
	virtual void onContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo);
protected:
	GraphNode* node = NULL;
	Vector2f nodePosition;
};

GraphNode* getGraphNode(ax::NodeEditor::NodeId id);