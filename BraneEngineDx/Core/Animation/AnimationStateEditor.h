#pragma once

#include "AnimationState.h"
#include "../Graph/StateMachine/StateNodeEditor.h"

class AnimationTransitionEditor : public StateGraphTransitionEditor
{
public:
	AnimationTransitionEditor() = default;
	virtual ~AnimationTransitionEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo);
protected:
	AnimationTransition* animationTransition = NULL;
};

class AnimationStateEditor : public StateNodeEditor
{
public:
	AnimationStateEditor() = default;
	virtual ~AnimationStateEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onDoubleClicked(EditorInfo& info, GraphInfo& graphInfo);
protected:
	AnimationState* animationState = NULL;
};