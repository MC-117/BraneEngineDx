#include "BoneEditor.h"

RegistEditor(Bone);

float BoneEditor::boneRadius = 0.15f;
BoneEditor::ShowBoneFlag BoneEditor::showBoneFlag = BoneEditor::Selected;

void BoneEditor::setInspectedObject(void* object)
{
	bone = dynamic_cast<Bone*>((Object*)object);
	ActorEditor::setInspectedObject(bone);
}

void BoneEditor::onActorGUI(EditorInfo& info)
{
	ActorEditor::onActorGUI(info);
	if (ImGui::CollapsingHeader("Bone"))
		onBoneGUI(info);
}

void BoneEditor::onBoneGUI(EditorInfo& info)
{
	if (ImGui::BeginCombo("Show Bone Flag", "Selected")) {
		if (ImGui::Selectable("Hide", showBoneFlag == Hide))
			showBoneFlag = Hide;
		if (ImGui::Selectable("Selected", showBoneFlag == Selected))
			showBoneFlag = Selected;
		if (ImGui::Selectable("Always", showBoneFlag == Always))
			showBoneFlag = Always;
		ImGui::EndCombo();
	}
	ImGui::DragFloat("Bone Radius", &boneRadius, 0.01f);
}

void BoneEditor::onPersistentGizmo(GizmoInfo& info)
{
	if (showBoneFlag == Hide ||
		(showBoneFlag == Selected &&
			EditorManager::getSelectedObject() != object))
		return;
	Color color = Color(0.0f, 1.0f, 0.0f);
	Matrix4f transformMat = bone->getTransformMat();
	info.gizmo->drawSphere(Vector3f::Zero(), boneRadius,
		transformMat, color);
	for (int i = 0; i < bone->children.size(); i++) {
		Bone* cb = dynamic_cast<Bone*>(bone->children[i]);
		if (cb != NULL) {
			Matrix4f R = Matrix4f::Identity();
			Vector3f vec = cb->position;
			R.block(0, 0, 3, 3) = Quaternionf::FromTwoVectors(Vector3f(1, 0, 0),
				vec.normalized()).toRotationMatrix();
			info.gizmo->drawPyramidX(Vector3f(boneRadius, 0, 0), boneRadius,
				vec.norm() - boneRadius * 2, 4, transformMat * R, color);
		}
	}
	if (showBoneFlag == Always)
		if (info.gizmo->pickSphere(Vector3f::Zero(), boneRadius, Vector2f::Zero(), bone->getTransformMat()))
			EditorManager::selectObject(bone);
}
