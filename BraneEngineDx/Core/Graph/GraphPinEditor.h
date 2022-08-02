#pragma once

#include "Node.h"
#include "GraphBaseEditor.h"

class GraphPinEditor;

struct LinkInfo
{
	ax::NodeEditor::LinkId linkID;
	ImU32 color;
	GraphPinEditor* fromEditor;
	GraphPinEditor* toEditor;
	GraphPin* fromPin;
	GraphPin* toPin;
};

class GraphPinEditor : public GraphBaseEditor
{
public:
	GraphPinEditor() = default;
	virtual ~GraphPinEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onInspectGUI(EditorInfo& info);
	virtual void onGraphGUI(EditorInfo& info, GraphInfo& graphInfo);
	virtual void onPinGUI(EditorInfo& info, GraphInfo& graphInfo);
	virtual void onLinkGUI(EditorInfo& info, GraphInfo& graphInfo, LinkInfo& linkInfo);
	virtual void onContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo);
protected:
	GraphPin* pin = NULL;
};

GraphPin* getGraphPin(ax::NodeEditor::PinId id);
ax::NodeEditor::LinkId getLinkID(ax::NodeEditor::PinId fromID, ax::NodeEditor::PinId toID);
void getLinkedPinID(ax::NodeEditor::LinkId linkID, ax::NodeEditor::PinId& fromID, ax::NodeEditor::PinId& toID);
