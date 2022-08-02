#pragma once

#include "ObjectEditor.h"
#include "../Transform.h"

class PhysicalCollider;

class TransformEditor : public ObjectEditor
{
public:
	TransformEditor() = default;
	virtual ~TransformEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onTransformGUI(EditorInfo& info);

	virtual void onPersistentGizmo(GizmoInfo& info);
	virtual void onHandleGUI(EditorInfo& info);
	virtual void onHandleGizmo(GizmoInfo& info);

	virtual void onPhysicsGUI(EditorInfo& info);

	virtual void onDetailGUI(EditorInfo & info);
	virtual void onGizmo(GizmoInfo& info);

	static Vector3f* getSnapVector();
protected:
	Transform* transform = NULL;
	PhysicalCollider* selectedCollider = NULL;
	static Gizmo::HandleType handleOperation;
	static TransformSpace handleSpace;
	static bool snapped;
	static Vector3f positionSnap;
	static Vector3f rotationSnap;
	static Vector3f scalingSnap;
	static Vector3f* snapVector;
};