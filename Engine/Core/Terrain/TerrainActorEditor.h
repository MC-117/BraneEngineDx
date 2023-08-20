#pragma once

#include "../Editor/ActorEditor.h"
#include "TerrainActor.h"

class TerrainActorEditor : public ActorEditor
{
public:
	TerrainActorEditor() = default;
	virtual ~TerrainActorEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onPersistentGizmo(GizmoInfo& info);
	virtual void onHandleGizmo(GizmoInfo& info);
	virtual void onActorGUI(EditorInfo& info);

	virtual void onTerrainActorGUI(EditorInfo& info);
	virtual void onRenderersGUI(EditorInfo& info);
protected:
	TerrainActor* terrainActor = NULL;
};