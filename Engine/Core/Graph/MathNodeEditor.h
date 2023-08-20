#pragma once

#include "MathNode.h"
#include "GraphNodeEditor.h"

class MathOperationNodeEditor : public GraphNodeEditor
{
public:
	MathOperationNodeEditor() = default;
	virtual ~MathOperationNodeEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onGraphGUI(EditorInfo & info, GraphInfo & graphInfo);
protected:
	MathOperationNode* mathOperationNode = NULL;
};

class ComparisonNodeEditor : public GraphNodeEditor
{
public:
	ComparisonNodeEditor() = default;
	virtual ~ComparisonNodeEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onGraphGUI(EditorInfo& info, GraphInfo& graphInfo);
protected:
	ComparisonNode* comparisonNode = NULL;
};

class NotNodeEditor : public GraphNodeEditor
{
public:
	NotNodeEditor() = default;
	virtual ~NotNodeEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onGraphGUI(EditorInfo& info, GraphInfo& graphInfo);
protected:
	NotNode* notNode = NULL;
};