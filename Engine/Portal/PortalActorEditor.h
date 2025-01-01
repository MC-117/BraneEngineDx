#pragma once

#include "../Core/Editor/ActorEditor.h"
#include "PortalActor.h"

class PortalActorEditor : public ActorEditor
{
public:
    PortalActorEditor() = default;
    virtual ~PortalActorEditor() = default;

    EditorInfoMethod();
    virtual void setInspectedObject(void* object);

    virtual void onPersistentGizmo(GizmoInfo& info);
    virtual void onHandleGizmo(GizmoInfo& info);
    virtual void onActorGUI(EditorInfo& info);
    virtual void onRenderersGUI(EditorInfo & info);
protected:
    PortalActor* portalActor = NULL;
};
