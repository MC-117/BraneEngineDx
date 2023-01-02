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
	if (camera) {
		for (int i = 0; i < meshActor->meshRender.materials.size(); i++) {
			Material* material = meshActor->meshRender.materials[i];
			if (material == NULL || !meshActor->meshRender.meshPartsEnable[i])
				continue;
			MeshPart* part = &meshActor->meshRender.mesh->meshParts[i];

			if (!camera->culling(part->bound, meshActor->meshRender.transformMat)) {
				info.gizmo->drawAABB(part->bound, meshActor->meshRender.transformMat, { 255, 0, 0 });
			}
		}
	}
	if (Engine::input.getMouseButtonRelease(MouseButtonEnum::Left)) {
		for (auto& meshPart : meshActor->meshRender.mesh->meshParts) {
			info.gizmo->doScreenHit(meshActor->getInstanceID(), meshPart, meshActor->meshRender.instanceID, meshActor->meshRender.instanceCount);
		}
	}
}

void MeshActorEditor::onHandleGizmo(GizmoInfo& info)
{
	ActorEditor::onHandleGizmo(info);
	if (meshActor->meshRender.mesh == NULL)
		return;
	info.gizmo->drawAABB(meshActor->meshRender.mesh->bound,
		meshActor->meshRender.transformMat, Color(0.0f, 0.0f, 1.0f));
}

void MeshActorEditor::onRenderersGUI(EditorInfo& info)
{
	if (meshActor->meshRender.mesh == NULL) {
		ImGui::Text("No mesh");
	}
	else {
		string meshPath = MeshAssetInfo::getPath(meshActor->meshRender.mesh);
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
