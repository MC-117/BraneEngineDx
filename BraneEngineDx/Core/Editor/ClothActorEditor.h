#pragma once

#include "MeshActorEditor.h"
#include "../../Actors/ClothActor.h"

class ClothActorEditor : public MeshActorEditor
{
public:
	ClothActorEditor() = default;
	virtual ~ClothActorEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onHandleGizmo(GizmoInfo& info);
	virtual void onActorGUI(EditorInfo& info);
protected:
	ClothActor* clothActor = NULL;
	bool editingCollider = false;
	PhysicalCollider* selectedCollider = NULL;
	ClothVertexCluster* selectedCluster = NULL;
};