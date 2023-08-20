#pragma once

#include "TransformEditor.h"
#include "../Actor.h"

class ActorEditor : public TransformEditor
{
public:
	ActorEditor() = default;
	virtual ~ActorEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onAudioGUI(EditorInfo& info);
	virtual void onActorGUI(EditorInfo & info);
	virtual void onRenderersGUI(EditorInfo& info);

	virtual void onDetailGUI(EditorInfo & info);
protected:
	Actor* actor = NULL;
};