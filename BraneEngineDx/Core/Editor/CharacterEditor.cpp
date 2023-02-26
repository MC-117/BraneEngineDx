#include "CharacterEditor.h"

RegistEditor(Character);

void CharacterEditor::setInspectedObject(void* object)
{
	character = dynamic_cast<Character*>((Base*)object);
	ActorEditor::setInspectedObject(character);
}

void CharacterEditor::onPersistentGizmo(GizmoInfo& info)
{
	ActorEditor::onPersistentGizmo(info);

	Color color = Color::HSV({ 12.0f, 0.64f, 0.94f });
	BoundBox bound = character->physicalController.capsule.bound;
	float radius = abs(bound.maxPoint.x() - bound.minPoint.x()) * 0.5f;
	float halfLength = (abs(bound.maxPoint.z() - bound.minPoint.z()) - abs(bound.maxPoint.x() - bound.minPoint.x())) * 0.5f;
	Matrix4f mat = character->getTransformMat();
	info.gizmo->drawCapsuleZ(Vector3f::Zero(), radius, halfLength, mat, color);
	if (info.gizmo->pickCapsuleZ(Vector3f::Zero(), radius, halfLength, Vector2f::Zero(), mat))
		EditorManager::selectObject(character);
}

void CharacterEditor::onActorGUI(EditorInfo& info)
{
	ActorEditor::onActorGUI(info);
	if (ImGui::CollapsingHeader("Character")) {
		Vector3f gv = character->physicalController.gravityVelocity;
		Vector3f mv = character->physicalController.moveVelocity;
		ImGui::Text("GravityVelocity: %f, %f, %f", gv.x(), gv.y(), gv.z());
		ImGui::Text("MoveVelocity: %f, %f, %f", mv.x(), mv.y(), mv.z());
		ImGui::Text("IsFly: %s", character->isFly() ? "true" : "false");
	}
}
