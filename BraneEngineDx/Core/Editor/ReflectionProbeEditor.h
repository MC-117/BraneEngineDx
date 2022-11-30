#pragma once

#include "TransformEditor.h"
#include "../ReflectionProbe.h"

class ReflectionProbeEditor : public TransformEditor
{
public:
	ReflectionProbeEditor() = default;
	virtual ~ReflectionProbeEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onPersistentGizmo(GizmoInfo& info);
	virtual void onHandleGizmo(GizmoInfo& info);

	virtual void onReflectionProbeGUI(EditorInfo& info);

	virtual void onDetailGUI(EditorInfo& info);
protected:
	static Texture2D* icon;
	static Material* visualMaterial;
	static Mesh* sphereMesh;
	static bool isInited;

	ReflectionProbe* reflectionProbe = NULL;
	Material* instanceMaterial = NULL;

	Texture2D* getIcon();
	static void loadDefaultResource();
};