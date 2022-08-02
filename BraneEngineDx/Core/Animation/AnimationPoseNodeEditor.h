#pragma once

#include "AnimationPoseNode.h"
#include "../Graph/GraphNodeEditor.h"

class AnimationBaseNodeEditor : public GraphNodeEditor
{
public:
	AnimationBaseNodeEditor() = default;
	virtual ~AnimationBaseNodeEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onGraphGUI(EditorInfo& info, GraphInfo& graphInfo);
	virtual void onContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo);
protected:
	AnimationBaseNode* animationBaseNode = NULL;
};

class AnimationClipNodeEditor : public AnimationBaseNodeEditor
{
public:
	AnimationClipNodeEditor() = default;
	virtual ~AnimationClipNodeEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo);
protected:
	AnimationClipNode* animationClipNode = NULL;
};