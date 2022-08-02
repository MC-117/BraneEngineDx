#pragma once

#include "ValueNode.h"
#include "GraphNodeEditor.h"

class CastToNodeEditor : public GraphNodeEditor
{
public:
	CastToNodeEditor() = default;
	virtual ~CastToNodeEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onGraphGUI(EditorInfo& info, GraphInfo& graphInfo);
	virtual void onContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo);
protected:
	CastToNode* castToNode = NULL;
};