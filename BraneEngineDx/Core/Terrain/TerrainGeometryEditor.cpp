#include "TerrainGeometryEditor.h"
#include "../GUI/TextureViewer.h"

RegistEditor(TerrainGeometry);

void TerrainGeometryEditor::setInspectedObject(void* object)
{
	geometry = dynamic_cast<TerrainGeometry*>((Serializable*)object);
}

void TerrainGeometryEditor::onGeometryGUI(EditorInfo& info)
{
	unsigned long long tid = geometry->meshData.heightMap.getTextureID();
	ImGui::Columns(2, "TextureColumn", false);
	if (tid == 0) {
		ImGui::Button("##NoTextureBT", { 64, 64 });
	}
	else {
		if (ImGui::ImageButton((ImTextureID)tid, { 64, 64 }, { 0, 1 }, { 1, 0 })) {
			TextureViewer::showTexture(*info.gui, geometry->meshData.heightMap);
		}
	}
	ImGui::NextColumn();
	ImGui::Text("heightMap");
	if (ImGui::ArrowButton("AssignTexBT", ImGuiDir_Left)) {
		Asset* assignAsset = EditorManager::getSelectedAsset();
		if (assignAsset != NULL && assignAsset->assetInfo.type == "Texture2D") {
			Texture2D* srcTex = (Texture2D*)assignAsset->load();
			if (srcTex)
				geometry->meshData.updateHeightMap(*srcTex);
		}
	}
	ImGui::Columns();
	
	TerrainData data = geometry->getTerrainData();
	ImGui::InputFloat("GridUnit", &data.unit, 0.01f);
	ImGui::InputFloat("Height", &data.height, 0.01f);
	ImGui::InputFloat("TriSize", &data.triSize, 0.01f);
	ImGui::DragInt2("Grid", (int*)data.grid.data(), 1.0f, 0, 128);
	geometry->setTerrainData(data);
}

void TerrainGeometryEditor::onGUI(EditorInfo& info)
{
	if (geometry == NULL)
		return;
	onGeometryGUI(info);
}
