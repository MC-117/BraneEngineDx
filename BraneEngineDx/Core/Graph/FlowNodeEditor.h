#pragma once

#include "FlowNode.h"
#include "GraphNodeEditor.h"

class SequenceNodeEditor : public GraphNodeEditor
{
public:
	SequenceNodeEditor() = default;
	virtual ~SequenceNodeEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onGraphGUI(EditorInfo & info, GraphInfo & graphInfo);
protected:
	SequenceNode* sequenceNode = NULL;
};
