#pragma once

#include "ActorEditor.h"
#include "../Character.h"

class CharacterEditor : public ActorEditor
{
public:
	CharacterEditor() = default;
	virtual ~CharacterEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onPersistentGizmo(GizmoInfo& info);
	virtual void onActorGUI(EditorInfo& info);
protected:
	Character* character = NULL;
};