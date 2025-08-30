#pragma once

#include "ActorEditor.h"
#include "../DecalActor.h"

class DecalActorEditor : public ActorEditor
{
public:
    DecalActorEditor() = default;
    virtual ~DecalActorEditor() = default;

    EditorInfoMethod();
    virtual void setInspectedObject(void* object);

    virtual void onPersistentGizmo(GizmoInfo& info);
    virtual void onHandleGizmo(GizmoInfo& info);
    virtual void onRenderersGUI(EditorInfo & info);
protected:
    static Texture2D* decalIcon;
    DecalActor* decalActor = NULL;

	virtual Texture2D* getIcon();
};