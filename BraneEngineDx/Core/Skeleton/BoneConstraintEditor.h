#pragma once

#include "../Editor/Editor.h"
#include "BoneConstraint.h"
#include "../SkeletonMeshActor.h"

class BoneConstraintEditor : public Editor
{
public:
	BoneConstraintEditor() = default;
	virtual ~BoneConstraintEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	void setSkeletonMeshActor(SkeletonMeshActor* skeletonMeshActor);

	virtual void onGUI(EditorInfo& info);
	virtual void onGizmo(GizmoInfo& info);
protected:
	BoneConstraint* constraint = NULL;
	SkeletonMeshActor* skeletonMeshActor = NULL;
};