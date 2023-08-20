#pragma once

#include "TransformEditor.h"
#include "../World.h"

class WorldEditor : public TransformEditor
{
public:
	WorldEditor() = default;
	virtual ~WorldEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);
	virtual void onWorldGUI(EditorInfo& info);

	virtual void onPersistentGizmo(GizmoInfo& info);

	virtual void onDetailGUI(EditorInfo& info);
protected:
	World* world = NULL;

	vector<BoundBox> debugAABBs;
};