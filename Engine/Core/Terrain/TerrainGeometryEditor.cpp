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
	TerrainConfig config = Terrain::convertTerrainConfigFromData(data);
	ImGui::InputFloat("WidthPerGrid", &config.widthPerGrid, 0.01f);
	ImGui::InputFloat("Height", &config.height, 0.01f);
	ImGui::InputInt("TriangleLevels", &config.triangleLevels);
	ImGui::InputInt("TileLevels", &config.tileLevels);
	ImGui::InputInt2("Grid", (int*)config.grid.data(), 1.0f);
	
	config.validate();
	data = Terrain::getTerrainDataFromConfig(config);
	
	geometry->setTerrainData(data);
}

void TerrainGeometryEditor::onGUI(EditorInfo& info)
{
	if (geometry == NULL)
		return;
	onGeometryGUI(info);
}
