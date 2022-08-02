#pragma once

#include "StateNode.h"
#include "../GraphNodeEditor.h"

class StateTransitionEditor : public GraphBaseEditor
{
public:
	StateTransitionEditor() = default;
	virtual ~StateTransitionEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onGraphGUI(EditorInfo& info, GraphInfo& graphInfo);
	virtual void onLinkGUI(EditorInfo& info, GraphInfo& graphInfo, LinkInfo& linkInfo);

	virtual void onDoubleClicked(EditorInfo& info, GraphInfo& graphInfo, LinkInfo& linkInfo);
protected:
	StateTransition* transition = NULL;
};

class StateGraphTransitionEditor : public StateTransitionEditor
{
public:
	StateGraphTransitionEditor() = default;
	virtual ~StateGraphTransitionEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onDoubleClicked(EditorInfo& info, GraphInfo& graphInfo, LinkInfo& linkInfo);
protected:
	StateGraphTransition* graphTransition = NULL;
};

class StateTransitionPinEditor : public GraphPinEditor
{
public:
	StateTransitionPinEditor() = default;
	virtual ~StateTransitionPinEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onInspectGUI(EditorInfo& info);
	virtual void onGraphGUI(EditorInfo& info, GraphInfo& graphInfo);
	virtual void onLinkGUI(EditorInfo& info, GraphInfo& graphInfo, LinkInfo& linkInfo);
	virtual void onContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo);
protected:
	StateTransitionPin* transitionPin = NULL;
};

class StateNodeEditor : public GraphNodeEditor
{
public:
	StateNodeEditor() = default;
	virtual ~StateNodeEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onInspectGUI(EditorInfo & info);
	virtual void onGraphGUI(EditorInfo & info, GraphInfo & graphInfo);
	virtual void onContextMenuGUI(EditorInfo & info, GraphInfo & graphInfo);
protected:
	StateNode* state = NULL;
};

class StateEntryNodeEditor : public GraphNodeEditor
{
public:
	StateEntryNodeEditor() = default;
	virtual ~StateEntryNodeEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onInspectGUI(EditorInfo& info);
	virtual void onGraphGUI(EditorInfo& info, GraphInfo& graphInfo);
	virtual void onContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo);
protected:
	StateEntryNode* entryState = NULL;
};
