#include "MeshActorEditor.h"
#include "../Camera.h"
#include "../Engine.h"

RegistEditor(MeshActor);

void MeshActorEditor::setInspectedObject(void* object)
{
	meshActor = dynamic_cast<MeshActor*>((Object*)object);
	ActorEditor::setInspectedObject(meshActor);
}

void MeshActorEditor::onPersistentGizmo(GizmoInfo& info)
{
	Camera* camera = dynamic_cast<Camera*>(EditorManager::getSelectedObject());
	Mesh* mesh = meshActor->meshRender.getMesh();
	MeshMaterialCollection& collection = meshActor->meshRender.collection;
	if (camera) {
		for (int i = 0; i < collection.getMaterialCount(); i++) {
			Material* material = collection.getMaterial(i).second;
			if (material == NULL || !collection.getPartEnable(i))
				continue;
			MeshPart* part = &mesh->meshParts[i];

			if (!camera->culling(part->bound, meshActor->meshRender.transformMat)) {
				info.gizmo->drawAABB(part->bound, meshActor->meshRender.transformMat, { 255, 0, 0 });
			}
		}
	}
	if (ImGui::IsWindowFocused() && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
		for (auto& meshPart : mesh->meshParts) {
			info.gizmo->doScreenHit(meshPart, meshActor->meshRender.getInstancedTransformRenderDataHandle());
		}
	}
}

void MeshActorEditor::onHandleGizmo(GizmoInfo& info)
{
	ActorEditor::onHandleGizmo(info);
	Mesh* mesh = meshActor->meshRender.getMesh();
	if (mesh == NULL)
		return;
	info.gizmo->drawAABB(mesh->bound,
		meshActor->meshRender.transformMat, Color(0.0f, 0.0f, 1.0f));
}

void MeshActorEditor::onRenderersGUI(EditorInfo& info)
{
	Mesh* mesh = meshActor->meshRender.getMesh();
	if (mesh) {
		ImGui::Text("No mesh");
	}
	else {
		string meshPath = MeshAssetInfo::getPath(mesh);
		ImGui::Text(meshPath.c_str());
	}

	Asset* assignAsset = EditorManager::getSelectedAsset();
	if (assignAsset != NULL && assignAsset->assetInfo.type == "Mesh") {
		if (ImGui::Button("Set Selected Mesh", { -1, 40 })) {
			meshActor->setMesh((Mesh*)assignAsset->load());
		}
	}
	ImGui::Separator();
	Editor* editor = EditorManager::getEditor("MeshRender", &meshActor->meshRender);
	editor->onGUI(info);
}
