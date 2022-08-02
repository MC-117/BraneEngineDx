#pragma once

#include "ActorEditor.h"
#include "../SkeletonMeshActor.h"

class SkeletonMeshActorEditor : public ActorEditor
{
public:
	SkeletonMeshActorEditor() = default;
	virtual ~SkeletonMeshActorEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onSkeletonGUI(EditorInfo& info);
	virtual void onAnimationGUI(EditorInfo& info);
	virtual void onSkeletonPhysicsGUI(EditorInfo& info);

	virtual void onHandleGizmo(GizmoInfo& info);

	virtual void onSkeletonMeshActorGUI(EditorInfo& info);
	virtual void onActorGUI(EditorInfo& info);
	virtual void onRenderersGUI(EditorInfo & info);
protected:
	SkeletonMeshActor* skeletonMeshActor = NULL;
	string blendSpaceName;
	Serialization* selectedBoneConstraintType = NULL;
	bool editingBoneConstraint = false;
	BoneConstraint* selectedBoneConstraint = NULL;
	bool editingSkeletonPhysics = false;
	SkeletonPhysics::RigidBodyInfo* selectedRigidBodyInfo = NULL;
	int selectedRigidBodyIndex = -1;


	SkeletonPhysics::RigidBodyInfo* getSelectedRigidBodyInfo();
	void selectRigidBodyInfo(int index);
};