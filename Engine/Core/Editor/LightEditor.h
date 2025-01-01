#pragma once

#include "TransformEditor.h"
#include "../Light.h"

class LightEditor : public TransformEditor
{
public:
	LightEditor() = default;
	virtual ~LightEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onPersistentGizmo(GizmoInfo& info);
	virtual void onLightGUI(EditorInfo& info);

	virtual void onDetailGUI(EditorInfo & info);
protected:
	Light* light = NULL;

	virtual Texture2D* getIcon();
};