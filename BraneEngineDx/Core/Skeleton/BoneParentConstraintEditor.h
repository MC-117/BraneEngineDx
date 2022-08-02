#pragma once

#include "BoneConstraintEditor.h"
#include "BoneParentConstraint.h"

class BoneParentConstraintEditor : public BoneConstraintEditor
{
public:
	BoneParentConstraintEditor() = default;
	virtual ~BoneParentConstraintEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onGUI(EditorInfo & info);
	virtual void onGizmo(GizmoInfo & info);
protected:
	BoneParentConstraint* parentConstraint = NULL;
};