#pragma once

#include "BoneConstraintEditor.h"
#include "TwoBoneIKConstraint.h"

class TwoBoneIKConstraintEditor : public BoneConstraintEditor
{
public:
	TwoBoneIKConstraintEditor() = default;
	virtual ~TwoBoneIKConstraintEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onGUI(EditorInfo& info);
	virtual void onGizmo(GizmoInfo& info);
protected:
	TwoBoneIKConstraint* ik = NULL;
};