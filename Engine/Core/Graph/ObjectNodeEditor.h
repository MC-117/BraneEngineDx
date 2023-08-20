#pragma once

#include "ObjectNode.h"
#include "ObjectNodes/TransformNode.h"
#include "GraphNodeEditor.h"
#include "VariableEditor.h"

class RefPinEditor : public GraphPinEditor
{
public:
	RefPinEditor() = default;
	virtual ~RefPinEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onInspectGUI(EditorInfo& info);

	virtual void onPinGUI(EditorInfo& info, GraphInfo& graphInfo);
protected:
	RefPin* refPin = NULL;
};

DEC_OBJECT_VAR_EDITOR(Object*, ObjectRefVariable, ImGui::ObjectCombo(valueVariable->getName().c_str(), value, info.world->getObject(), "Object"));

class RefVariableEditor : public GraphVariableEditor
{
public:
	RefVariableEditor() = default;
	virtual ~RefVariableEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onInspectGUI(EditorInfo& info);
protected:
	RefVariable* valueVariable = NULL;
};