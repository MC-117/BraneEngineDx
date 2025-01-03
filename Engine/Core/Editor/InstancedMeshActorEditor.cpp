#include "InstancedMeshActorEditor.h"
#include "../Camera.h"
#include "../Engine.h"
#include "../GUI/GUIUtility.h"
#include "../../Core/ObjectUltility.h"

RegistEditor(InstancedMeshActor);

void InstancedMeshActorEditor::setInspectedObject(void* object)
{
	instancedMeshActor = dynamic_cast<InstancedMeshActor*>((Object*)object);
	ActorEditor::setInspectedObject(instancedMeshActor);
}

void InstancedMeshActorEditor::onPersistentGizmo(GizmoInfo& info)
{
	if (ImGui::IsWindowFocused()) {
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
			for (int meshIndex = 0; meshIndex < instancedMeshActor->instancedMeshRender.getMeshCount(); meshIndex++) {
				MeshMaterialCollection* collection = instancedMeshActor->instancedMeshRender.getMesh(meshIndex);
				Mesh* mesh = collection->getMesh();
				for (auto& meshPart : mesh->meshParts) {
					info.gizmo->doScreenHit(meshPart, instancedMeshActor->instancedMeshRender.getInstancedTransformRenderDataHandle(meshIndex));
				}
			}
		}

		if (enableInstanceBrushMode) {
			if (World* world = getRootWorld(*instancedMeshActor)) {
				ImVec2 rawPos = ImGui::GetMousePos();
				ImVec2 winPos = ImGui::GetWindowPos();
				ImVec2 winSize = ImGui::GetWindowSize();
				Vector2f mouseUV = { (rawPos.x - winPos.x) / winSize.x, (rawPos.y - winPos.y) / winSize.y };

				if (mouseUV.x() > 0 && mouseUV.x() < 1 && mouseUV.y() > 0 && mouseUV.y() < 1) {
					Vector4f mouseNDC { mouseUV.x() * 2.0f - 1.0f, 1.0f - mouseUV.y() * 2.0f, 1.0f, 1.0f };
					Vector4f mouseView = info.camera->getProjectionMatrix().inverse() * mouseNDC;
					mouseView /= mouseView.w();
					mouseView.w() = 0;
					Vector3f mouseRay = info.camera->getViewMatrix().inverse() * mouseView;
					Vector3f cameraPos = info.camera->getFinalWorldPosition();
					ContactInfo hit;
					if (world->physicalWorld.rayTest(cameraPos, cameraPos + mouseRay * 1500, hit)) {
						info.gizmo->drawSphere(hit.location, brushRadius, Matrix4f::Identity(), Color(0.0f, 0.0f, 1.0f));
					}
				}
			}
		}
	}
}

void InstancedMeshActorEditor::onHandleGizmo(GizmoInfo& info)
{
	ActorEditor::onHandleGizmo(info);
}

void InstancedMeshActorEditor::onActorGUI(EditorInfo& info)
{
	ActorEditor::onActorGUI(info);

	ImGui::BeginGroupPanel("InstanceBrush");

	ImGui::Checkbox("Enable Brush", &enableInstanceBrushMode);
	ImGui::DragFloat("Brush Radius", &brushRadius, 0.1, 0, 2000);
	
	ImGui::EndGroupPanel();
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
