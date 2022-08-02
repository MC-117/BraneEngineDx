#include "BoneParentConstraintEditor.h"
#include "../Editor/TransformEditor.h"
#include "../GUI/GUIUtility.h"

RegistEditor(BoneParentConstraint);

void BoneParentConstraintEditor::setInspectedObject(void* object)
{
	parentConstraint = dynamic_cast<BoneParentConstraint*>((BoneConstraint*)object);
}

void BoneParentConstraintEditor::onGUI(EditorInfo& info)
{
	if (parentConstraint == NULL)
		return;

	BoneConstraintEditor::onGUI(info);

	bool resetup = false;

	Bone* parentBone = parentConstraint->parentBone;
	if (ImGui::ObjectCombo("ParentBone", (Object*&)parentBone, skeletonMeshActor, "Bone")) {
		parentConstraint->parentBone = parentBone;
		resetup = true;
	}
	Bone* childBone = parentConstraint->childBone;
	if (ImGui::ObjectCombo("ChildBone", (Object*&)childBone, skeletonMeshActor, "Bone")) {
		parentConstraint->childBone = childBone;
		resetup = true;
	}

	if (resetup)
		parentConstraint->setup();

	ImGui::DragFloat3("OffsetPosition", parentConstraint->offsetPostion.data());
	ImGui::DragFloat4("OffsetRotation", parentConstraint->offsetRotation.data());
	ImGui::DragFloat("Weight", &parentConstraint->weight);

	const char* flagNames[] = { "None", "Position", "Rotation" };

	string flags;
	for (int i = 0; i < 2; i++) {
		if (parentConstraint->flags.has(1 << i))
			flags += flagNames[i + 1];
	}
	if (flags.empty())
		flags = flagNames[0];

	if (ImGui::BeginCombo("Flags", flags.c_str())) {
		for (int i = 0; i < 2; i++) {
			bool has = parentConstraint->flags.has(1 << i);
			if (ImGui::Selectable(flagNames[i + 1], &has)) {
				if (has)
					parentConstraint->flags |= 1 << i;
				else
					parentConstraint->flags ^= 1 << i;
			}
		}
		ImGui::EndCombo();
	}
}

void BoneParentConstraintEditor::onGizmo(GizmoInfo& info)
{
}
