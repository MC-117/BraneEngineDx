#include "TransformEditor.h"
#include "../Physics/RigidBody.h"
#include "../Physics/PhysicalWorld.h"

RegistEditor(Transform);

Gizmo::HandleType TransformEditor::handleOperation = Gizmo::HandleType::Transition;
TransformSpace TransformEditor::handleSpace = WORLD;
bool TransformEditor::snapped = false;
Vector3f TransformEditor::positionSnap = { 0, 0, 0 };
Vector3f TransformEditor::rotationSnap = 0;
Vector3f TransformEditor::scalingSnap = 0;
Vector3f* TransformEditor::snapVector = NULL;

void TransformEditor::setInspectedObject(void* object)
{
	transform = dynamic_cast<Transform*>((Object*)object);
	ObjectEditor::setInspectedObject(transform);
}

void TransformEditor::onTransformGUI(EditorInfo& info)
{
	float v[3] = { transform->position.x(), transform->position.y(), transform->position.z() };
	if (ImGui::DragFloat3("Position", v, 0.01)) {
		transform->setPosition(v[0], v[1], v[2]);
	}
	Vector3f rv = transform->getEulerAngle();
	float r[3] = { rv.x(), rv.y(), rv.z() };
	if (ImGui::DragFloat3("Rotation", r, 0.1)) {
		transform->setRotation(r[0], r[1], r[2]);
	}
	float s[3] = { transform->scale.x(), transform->scale.y(), transform->scale.z() };
	if (ImGui::DragFloat3("Scale", s, 0.01))
		transform->setScale(s[0], s[1], s[2]);
	ImGui::RadioButton("World", (int*)&handleSpace, WORLD);
	ImGui::SameLine();
	ImGui::RadioButton("Local", (int*)&handleSpace, LOCAL);
}

void TransformEditor::onPersistentGizmo(GizmoInfo& info)
{
}

void TransformEditor::onHandleGUI(EditorInfo& info)
{
	if (ImGui::IsKeyPressed('Z'))
		handleOperation = Gizmo::HandleType::Transition;
	if (ImGui::IsKeyPressed('X'))
		handleOperation = Gizmo::HandleType::Rotation;
	if (ImGui::IsKeyPressed('C'))
		handleOperation = Gizmo::HandleType::Scaling;

	snapVector = NULL;
	snapped = false;
	if (ImGui::IsKeyDown('V')) {
		snapped = true;
		switch (handleOperation)
		{
		case Gizmo::HandleType::Transition:
			ImGui::DragFloat("Snap", positionSnap.data(), 0.1);
			if (positionSnap.x() <= 0)
				positionSnap[0] = 1;
			positionSnap.z() = positionSnap.y() = positionSnap.x();
			snapVector = &positionSnap;
			break;
		case Gizmo::HandleType::Rotation:
			ImGui::DragFloat("Snap", rotationSnap.data(), 0.1);
			if (rotationSnap.x() <= 0)
				rotationSnap.x() = 1;
			snapVector = &rotationSnap;
			break;
		case Gizmo::HandleType::Scaling:
			ImGui::DragFloat("Snap", scalingSnap.data(), 0.1);
			if (scalingSnap.x() <= 0)
				scalingSnap.x() = 1;
			snapVector = &scalingSnap;
			break;
		default:
			break;
		}
	}
}

void TransformEditor::onHandleGizmo(GizmoInfo& info)
{
	Matrix4f transMat;
	transMat = transform->getMatrix(WORLD);

	if (info.gizmo->drawHandle(this, handleOperation, handleSpace, transMat,
		snapVector == NULL ? Vector3f::Zero() : *snapVector)) {
		transform->setMatrix(transMat, WORLD);
	}
}

void TransformEditor::onPhysicsGUI(EditorInfo& info)
{
	RigidBody* rigid = transform->rigidBody;
	if (rigid != NULL && ImGui::CollapsingHeader("Physics")) {
		static const char* btStr[] = { "NONE", "RIGID", "SOFT" };
		static const char* ctStr[] = { "NONE", "RIGID", "SOFT" };
		ImGui::Text("BodyType: %s", btStr[rigid->bodyType]);

		if (ImGui::TreeNode("Colliders")) {
			for (int i = 0; i < rigid->getColliderCount(); i++) {
				PhysicalCollider* collider = rigid->getCollider(i);
				bool selected = selectedCollider == collider;
				ImGui::Checkbox(("##Collider" + to_string(i)).c_str(), &selected);
				if (selected) {
					selectedCollider = collider;
				}
				else if (selectedCollider == collider) {
					selectedCollider = NULL;
				}
				if (ImGui::TreeNode(to_string(i).c_str())) {
					ImGui::Text("ComplexShape: %s", collider->shapeComplexType == SIMPLE ? "SIMPLE" : "COMPLEX");

					Vector3f pos = collider->getPositionOffset();
					if (ImGui::DragFloat3("Position##Collision_Position", pos.data(), 0.01)) {
						collider->setPositionOffset(pos);
					}

					Quaternionf rot = collider->getRotationOffset();
					Vector3f rot_eular = rot.toRotationMatrix().eulerAngles();
					rot_eular *= 180 / PI;
					if (ImGui::DragFloat3("Rotation##Collision_Rotation", rot_eular.data(), 0.01)) {
						rot_eular *= PI / 180;
						collider->setRotationOffset(rot.setFromEularAngles(rot_eular));
					}

					if (collider->shape != NULL) {
						if (ImGui::TreeNode((collider->shape->getSerialization().type + " Shape").c_str())) {
							ImGui::InputFloat3("MinPoint", collider->shape->bound.minVal.data(), "%.3f", ImGuiInputTextFlags_ReadOnly);
							ImGui::InputFloat3("MaxPoint", collider->shape->bound.maxVal.data(), "%.3f", ImGuiInputTextFlags_ReadOnly);
							ImGui::TreePop();
						}
					}
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Constraint")) {
			for (int i = 0; i < transform->constraints.size(); i++) {
				ImGui::Text("Connect with %s", transform->constraints[i]->rigidBody2->targetTransform.name.c_str());
			}
			ImGui::TreePop();
		}
	}
}

void TransformEditor::onDetailGUI(EditorInfo& info)
{
	if (ImGui::CollapsingHeader("Transform")) {
		onTransformGUI(info);
		onHandleGUI(info);
	}
	onPhysicsGUI(info);
}

void TransformEditor::onGizmo(GizmoInfo& info)
{
	onPersistentGizmo(info);
	if (EditorManager::getSelectedObject() == object)
		onHandleGizmo(info);
	if (selectedCollider) {
		Matrix4f mat = Matrix4f::Identity();
		mat.block(0, 3, 3, 1) = selectedCollider->getPositionOffset();
		mat.block(0, 0, 3, 3) = selectedCollider->getRotationOffset().toRotationMatrix();
		if (info.gizmo->drawHandle(selectedCollider, handleOperation, handleSpace, mat)) {
			Vector3f posOffset;
			Quaternionf rotOffset = Quaternionf::Identity();
			if (mat.decompose(posOffset, rotOffset, Vector3f())) {
				selectedCollider->setPositionOffset(posOffset);
				selectedCollider->setRotationOffset(rotOffset);
			}
		}
	}
}

Vector3f* TransformEditor::getSnapVector()
{
	return snapVector;
}
