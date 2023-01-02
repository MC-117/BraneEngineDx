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

	virtual void onPersistentGizmo(GizmoInfo& info);
	virtual void onHandleGizmo(GizmoInfo& info);

	virtual void onLightGUI(EditorInfo& info);
protected:
	static Texture2D* icon;
	PointLight* pointLight = NULL;

	Texture2D* getIcon();
};