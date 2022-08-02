#pragma once

#include "StateMachine.h"
#include "StateNodeEditor.h"
#include "../GraphEditor.h"

class StateMachineEditor : public GraphEditor
{
public:
	StateMachineEditor() = default;
	virtual ~StateMachineEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onExtraContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo);
	virtual void onCreateLink(GraphPin* fromPin, GraphPin* toPin);

	virtual void onGraphCanvasGUI(EditorInfo& info, GraphInfo& graphInfo);

	virtual void onCanvasContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo);
protected:
	StateMachine* stateMachine = NULL;

	string stateName;
	GraphPin* linkFromPin = NULL;
	GraphPin* linkToPin = NULL;
};
