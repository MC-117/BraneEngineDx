#pragma once

#include "LightEditor.h"
#include "../PointLight.h"

class PointLightEditor : public LightEditor
{
public:
	PointLightEditor() = default;
	virtual ~PointLightEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onHandleGizmo(GizmoInfo& info);

	virtual void onLocalLightShapeGizmo(GizmoInfo& info);

	virtual void onLightGUI(EditorInfo& info);
protected:
	static Texture2D* pointLightIcon;
	PointLight* pointLight = NULL;

	virtual Texture2D* getIcon();
};