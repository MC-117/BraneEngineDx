#include "BoneConstraintEditor.h"

RegistEditor(BoneConstraint);

void BoneConstraintEditor::setInspectedObject(void* object)
{
    constraint = (BoneConstraint*)object;
}

void BoneConstraintEditor::setSkeletonMeshActor(SkeletonMeshActor* skeletonMeshActor)
{
    this->skeletonMeshActor = skeletonMeshActor;
}

void BoneConstraintEditor::onGUI(EditorInfo& info)
{
    if (constraint == NULL)
        return;
    ImGui::Checkbox("Enable", &constraint->enable);
}

void BoneConstraintEditor::onGizmo(GizmoInfo& info)
{
}
