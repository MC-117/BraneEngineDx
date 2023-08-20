#include "ClothActorEditor.h"

RegistEditor(ClothActor);

void ClothActorEditor::setInspectedObject(void* object)
{
	clothActor = dynamic_cast<ClothActor*>((Object*)object);
	MeshActorEditor::setInspectedObject(clothActor);
}

void ClothActorEditor::onHandleGizmo(GizmoInfo& info)
{
	MeshActorEditor::onHandleGizmo(info);
	if (!editingCollider)
		return;

	vector<PhysicalCollider*>& colliders = clothActor->clothBody.colliders;
	Matrix4f transformMat = clothActor->getTransformMat();
	for (int i = 0; i < colliders.size(); i++) {
		PhysicalCollider* collider = colliders[i];
		Matrix4f T = Matrix4f::Identity();
		T.block(0, 3, 3, 1) = collider->getPositionOffset();
		Matrix4f R = Matrix4f::Identity();
		R.block(0, 0, 3, 3) = collider->getRotationOffset().toRotationMatrix();
		Matrix4f mat = transformMat * T * R;

		Color color = Color::HSV({ collider->layer.getLayer() * 24.0f, 0.64f, 0.94f });
		Sphere* sphere = dynamic_cast<Sphere*>(collider->shape);
		if (sphere) {
			float radius = sphere->getRadius();
			info.gizmo->drawSphere(Vector3f::Zero(), radius, mat, color);
			if (info.gizmo->pickSphere(Vector3f::Zero(), radius, Vector2f::Zero(), mat)) {
				selectedCollider = collider;
				selectedCluster = NULL;
			}
		}

		if (selectedCollider == collider) {
			if (info.gizmo->drawHandle(collider,
				handleOperation, handleSpace, mat,
				snapVector == NULL ? Vector3f::Zero() : *snapVector)) {
				Matrix4f dmat = transformMat.inverse() * mat;
				dmat.decompose(collider->positionOffset, collider->rotationOffset, Vector3f::Zero());
				collider->apply();
			}
		}
	}

	for (int i = 0; i < clothActor->clothBody.getVertexClusterCount(); i++) {
		ClothVertexCluster* cluster = clothActor->clothBody.getVertexCluster(i);
		Matrix4f mat = transformMat * cluster->transformMat;

		for (int s = 0; s < cluster->spheres.size(); s++) {
			Vector4f sphere = cluster->spheres[s];
			float radius = sphere.w();
			info.gizmo->drawSphere(sphere.block(0, 0, 3, 1), sphere.w(), mat, { 0.0f, 0.0f, 1.0f });
		}
		for (int v = 0; v < cluster->getVertexCount(); v++) {
			Vector3f p = cluster->getVertexPosition(v);
			p = (transformMat * Vector4f(p.x(), p.y(), p.z(), 1)).block(0, 0, 3, 1);
			info.gizmo->drawPoint(p, 2, { 0.0f, 1.0f, 0.0f });

			if (info.gizmo->pickSphere(Vector3f::Zero(), 3, Vector2f::Zero(), mat)) {
				selectedCollider = NULL;
				selectedCluster = cluster;
			}
		}

		if (selectedCluster == cluster) {
			if (info.gizmo->drawHandle(cluster,
				handleOperation, handleSpace, mat,
				snapVector == NULL ? Vector3f::Zero() : *snapVector)) {
				cluster->transformMat = transformMat.inverse() * mat;
				cluster->needUpdate = true;
			}
		}
	}
}

void ClothActorEditor::onActorGUI(EditorInfo& info)
{
	MeshActorEditor::onActorGUI(info);
	editingCollider = ImGui::CollapsingHeader("ClothActor");
	if (editingCollider) {
		/*vector<PhysicalCollider*>& colliders = clothActor->clothBody.colliders;
		Matrix4f transformMat = clothActor->getTransformMat();
		for (int i = 0; i < colliders.size(); i++) {
			PhysicalCollider* collider = colliders[i];
			Matrix4f T = Matrix4f::Identity();
			T.block(0, 3, 3, 1) = collider->positionOffset;
			Matrix4f R = Matrix4f::Identity();
			R.block(0, 0, 3, 3) = collider->rotationOffset.toRotationMatrix();
			Matrix4f mat = transformMat * T * R;
		}*/
	}
}
