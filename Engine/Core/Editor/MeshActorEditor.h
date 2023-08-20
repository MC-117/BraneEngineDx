#pragma once

#include "ActorEditor.h"
#include "../MeshActor.h"

class MeshActorEditor : public ActorEditor
{
public:
	MeshActorEditor() = default;
	virtual ~MeshActorEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onPersistentGizmo(GizmoInfo& info);
	virtual void onHandleGizmo(GizmoInfo& info);
	virtual void onRenderersGUI(EditorInfo & info);
protected:
	MeshActor* meshActor = NULL;
};