#include "TerrainActorEditor.h"
#include "TerrainGeometryEditor.h"
#include "../Physics/RigidBody.h"

RegistEditor(TerrainActor);

void TerrainActorEditor::setInspectedObject(void* object)
{
	terrainActor = dynamic_cast<TerrainActor*>((Object*)object);
	ActorEditor::setInspectedObject(terrainActor);
}

void TerrainActorEditor::onPersistentGizmo(GizmoInfo& info)
{
}

void TerrainActorEditor::onHandleGizmo(GizmoInfo& info)
{
	ActorEditor::onHandleGizmo(info);
	TerrainGeometry* geometry = terrainActor->getGeometry();
	if (geometry == NULL)
		return;
	info.gizmo->drawAABB(geometry->bound,
		terrainActor->terrainRender.transformMat,
		Color(0.0f, 0.0f, 1.0f));
}

void TerrainActorEditor::onActorGUI(EditorInfo& info)
{
	ActorEditor::onActorGUI(info);
	if (ImGui::CollapsingHeader("TerrainActor"))
		onTerrainActorGUI(info);
}

void TerrainActorEditor::onTerrainActorGUI(EditorInfo& info)
{
	TerrainGeometry* geometry = terrainActor->getGeometry();
	if (geometry == NULL) {
		if (ImGui::Button("Create Terrain", { -1, 36 })) {
			geometry = new TerrainGeometry();
			terrainActor->setGeometry(geometry);
		}
	}
	else {
		if (ImGui::Button("Update Collision", { -1, 36 })) {
			terrainActor->updateCollision();
		}
		Editor* editor = EditorManager::getEditor(*geometry);
		if (editor)
			editor->onGUI(info);
	}
}

void TerrainActorEditor::onRenderersGUI(EditorInfo& info)
{
	Material*& material = terrainActor->terrainRender.material;
	if (ImGui::TreeNode("Material")) {
		bool create = false;
		if (ImGui::Button("Assign Selected Material", { -1, 36 })) {
			Asset* assignAsset = EditorManager::getSelectedAsset();
			if (assignAsset != NULL && assignAsset->assetInfo.type == "Material") {
				material = (Material*)assignAsset->load();
			}
		}
		if (ImGui::Button("Assign Material", { -1, 36 })) {
			ImGui::OpenPopup("MatSelectPopup");
			create = false;
		}
		if (ImGui::Button("Create Material", { -1, 36 })) {
			ImGui::OpenPopup("MatSelectPopup");
			create = true;
		}

		if (ImGui::BeginPopup("MatSelectPopup")) {
			for (auto _b = MaterialAssetInfo::assetInfo.assets.begin(), _e = MaterialAssetInfo::assetInfo.assets.end();
				_b != _e; _b++) {
				if (ImGui::Selectable(_b->first.c_str(), false, 0, { 0, 20 })) {
					if (create)
						material = &((Material*)_b->second->load())->instantiate();
					else
						material = (Material*)_b->second->load();
				}
			}
			ImGui::EndPopup();
		}

		if (material == NULL) {
			ImGui::Text("No Material");
		}
		else {
			Editor* editor = EditorManager::getEditor("Material", material);
			editor->onGUI(info);
		}
		ImGui::TreePop();
	}
}
