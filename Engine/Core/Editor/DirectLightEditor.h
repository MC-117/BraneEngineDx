#pragma once

#include "LightEditor.h"
#include "../DirectLight.h"

class DirectLightEditor : public LightEditor
{
public:
	DirectLightEditor() = default;
	virtual ~DirectLightEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onPersistentGizmo(GizmoInfo& info);
	virtual void onLightGUI(EditorInfo& info);
protected:
	static Texture2D* directLightIcon;
	DirectLight* directLight = NULL;

	virtual Texture2D* getIcon();
};