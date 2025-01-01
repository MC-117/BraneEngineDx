#pragma once

#include "PointLightEditor.h"
#include "../SpotLight.h"

class SpotLightEditor : public PointLightEditor
{
public:
    SpotLightEditor() = default;
    virtual ~SpotLightEditor() = default;

    EditorInfoMethod();
    virtual void setInspectedObject(void* object);

    virtual void onPersistentGizmo(GizmoInfo& info);
    virtual void onLocalLightShapeGizmo(GizmoInfo& info);

    virtual void onLightGUI(EditorInfo& info);
protected:
    static Texture2D* spotLightIcon;
    SpotLight* spotLight = NULL;

    Texture2D* getIcon();
};