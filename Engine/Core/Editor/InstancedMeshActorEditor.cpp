#include "InstancedMeshActorEditor.h"
#include "../Camera.h"
#include "../Engine.h"
#include "../GUI/GUIUtility.h"

RegistEditor(InstancedMeshActor);

void InstancedMeshActorEditor::setInspectedObject(void* object)
{
	instancedMeshActor = dynamic_cast<InstancedMeshActor*>((Object*)object);
	ActorEditor::setInspectedObject(instancedMeshActor);
}

void InstancedMeshActorEditor::onPersistentGizmo(GizmoInfo& info)
{
	if (ImGui::IsWindowFocused() && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
		for (int meshIndex = 0; meshIndex < instancedMeshActor->instancedMeshRender.getMeshCount(); meshIndex++) {
			MeshMaterialCollection* collection = instancedMeshActor->instancedMeshRender.getMesh(meshIndex);
			Mesh* mesh = collection->getMesh();
			for (auto& meshPart : mesh->meshParts) {
				info.gizmo->doScreenHit(meshPart, instancedMeshActor->instancedMeshRender.getInstancedTransformRenderDataHandle(meshIndex));
			}
		}
	}
}

void InstancedMeshActorEditor::onHandleGizmo(GizmoInfo& info)
{
	ActorEditor::onHandleGizmo(info);
}

void InstancedMeshActorEditor::onRenderersGUI(EditorInfo& info)
{
	ImGui::DragFloat("Density", &instancedMeshActor->density);
	ImGui::DragFloat2("Bound", instancedMeshActor->bound.data());
	if (ImGui::Button("Update Instances", { -1, 36 })) {
		instancedMeshActor->updateData();
	}
	if (ImGui::AssetCombo("Mesh", selectedAsset, selectMeshFilter, "Mesh")) {
		if (selectedAsset && selectedAsset->assetInfo.type == "Mesh") {
			selectedMesh = (Mesh*)selectedAsset->load();
		}
	}
	ImGui::BeginDisabled(selectedMesh == NULL);
	if (ImGui::Button("Add Mesh", { -1, 36 })) {
		instancedMeshActor->addMesh(*selectedMesh);
	}
	ImGui::EndDisabled();
	
	ImGui::Indent(10);
	for (int meshIndex = 0; meshIndex < instancedMeshActor->instancedMeshRender.getMeshCount(); meshIndex++) {
		MeshMaterialCollection* collection = instancedMeshActor->instancedMeshRender.getMesh(meshIndex);

		if (ImGui::CollapsingHeader(("InstancedMesh " + std::to_string(meshIndex)).c_str())) {
			Mesh* mesh = collection->getMesh();
			if (mesh) {
				string meshPath = MeshAssetInfo::getPath(mesh);
				ImGui::Text(meshPath.c_str());
			}
			else {
				ImGui::Text("No mesh");
			}

			Asset* assignAsset = EditorManager::getSelectedAsset();
			if (assignAsset != NULL && assignAsset->assetInfo.type == "Mesh") {
				if (ImGui::Button("Set Selected Mesh", { -1, 40 })) {
					collection->setMesh((Mesh*)assignAsset->load());
				}
			}
			ImGui::MeshMaterialGUI(info, *collection);
		}
	}
	ImGui::Unindent(10);
}
