#pragma once

#include "ActorEditor.h"
#include "../InstancedMeshActor.h"

class InstancedMeshActorEditor : public ActorEditor
{
public:
    InstancedMeshActorEditor() = default;
    virtual ~InstancedMeshActorEditor() = default;

    EditorInfoMethod();
    virtual void setInspectedObject(void* object);

    virtual void onPersistentGizmo(GizmoInfo& info);
    virtual void onHandleGizmo(GizmoInfo& info);
    
	virtual void onActorGUI(EditorInfo & info);
    virtual void onRenderersGUI(EditorInfo & info);
protected:
    InstancedMeshActor* instancedMeshActor = NULL;
    Asset* selectedAsset = NULL;
    Mesh* selectedMesh = NULL;
    string selectMeshFilter;

    bool enableInstanceBrushMode = false;
    float brushRadius = 50;
};