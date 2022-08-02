#include "TwoBoneIKConstraintEditor.h"
#include "../Editor/TransformEditor.h"
#include "../GUI/GUIUtility.h"

RegistEditor(TwoBoneIKConstraint);

void TwoBoneIKConstraintEditor::setInspectedObject(void* object)
{
	ik = dynamic_cast<TwoBoneIKConstraint*>((BoneConstraint*)object);
}

void TwoBoneIKConstraintEditor::onGUI(EditorInfo& info)
{
	if (ik == NULL)
		return;

	BoneConstraintEditor::onGUI(info);

	bool resetup = false;

	Bone* rootBone = ik->rootBone;
	if (ImGui::ObjectCombo("RootBone", (Object*&)rootBone, skeletonMeshActor, "Bone")) {
		ik->rootBone = rootBone;
		resetup = true;
	}
	Bone* midBone = ik->midBone;
	if (ImGui::ObjectCombo("MidBone", (Object*&)midBone, skeletonMeshActor, "Bone")) {
		ik->midBone = midBone;
		resetup = true;
	}
	Bone* endBone = ik->endBone;
	if (ImGui::ObjectCombo("EndBone", (Object*&)endBone, skeletonMeshActor, "Bone")) {
		ik->endBone = endBone;
		resetup = true;
	}

	Transform* poleTran = ik->poleTransform;
	Transform* effectTran = ik->effectTransform;
	if (ImGui::ObjectCombo("Pole", (Object*&)poleTran, skeletonMeshActor, "Transform")) {
		ik->poleTransform = poleTran;
		resetup = true;
	}
	if (ImGui::ObjectCombo("Effect", (Object*&)effectTran, skeletonMeshActor, "Transform")) {
		ik->effectTransform = effectTran;
		resetup = true;
	}

	const char* spaceNames = "World\0Local";

	resetup |= ImGui::Combo("EffectSpace", (int*)&ik->effectSpace, spaceNames);
	ImGui::DragFloat3("EffectPosition", ik->effectPosition.data());

	resetup |= ImGui::Combo("PoleSpace", (int*)&ik->poleSpace, spaceNames);
	ImGui::DragFloat3("PolePosition", ik->polePosition.data());

	ImGui::DragFloat3("RefAxis", ik->refAxis.data());

	if (resetup)
		ik->setup();
}

void TwoBoneIKConstraintEditor::onGizmo(GizmoInfo& info)
{
	if (ik == NULL || !ik->isValid())
		return;
	Vector3f polePosition = ik->getPoleWorldPosition();
	Vector3f effectPosition = ik->getEffectWorldPosition();
	Matrix4f eT = Matrix4f::Identity();
	eT.block(0, 3, 3, 1) = effectPosition;
	Matrix4f pT = Matrix4f::Identity();
	pT.block(0, 3, 3, 1) = polePosition;

	Vector3f rootPostion = ik->rootBone->getPosition(WORLD);
	Vector3f midPostion = ik->midBone->getPosition(WORLD);
	Vector3f endPostion = ik->endBone->getPosition(WORLD);

	info.gizmo->drawLine(rootPostion, midPostion, { 0.0f, 0.0f, 1.0f });
	info.gizmo->drawLine(midPostion, endPostion, { 0.0f, 0.0f, 1.0f });

	info.gizmo->drawLine(rootPostion, polePosition, { 1.0f, 1.0f, 0.0f });

	Vector3f* snapVector = TransformEditor::getSnapVector();

	if (info.gizmo->drawHandle(effectPosition.data(),
		Gizmo::HandleType::Transition, WORLD, eT,
		snapVector == NULL ? Vector3f::Zero() : *snapVector)) {
		eT.decompose(effectPosition, Quaternionf(), Vector3f::Zero());
		ik->setEffectWorldPosition(effectPosition);
	}
	if (info.gizmo->drawHandle(polePosition.data(),
		Gizmo::HandleType::Transition, WORLD, pT,
		snapVector == NULL ? Vector3f::Zero() : *snapVector)) {
		pT.decompose(polePosition, Quaternionf(), Vector3f::Zero());
		ik->setPoleWorldPosition(polePosition);
	}
}
