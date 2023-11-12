#include "MaterialEditor.h"
#include "../Engine.h"
#include "../Utility/EngineUtility.h"
#include "../Utility/RenderUtility.h"
#include "../GUI/TextureViewer.h"
#include "../GUI/GUIUtility.h"
#include <thread>

RegistEditor(Material);

void MaterialEditor::setInspectedObject(void* object)
{
	material = (Material*)object;
}

void MaterialEditor::onMaterialGUI(EditorInfo& info)
{
	if (material == NULL)
		return;
	if (ImGui::Button("Save Material", { -1, 36 })) {
		string s = AssetInfo::getPath(material);
		string ext = filesystem::u8path(s).extension().generic_u8string();
		if (!_stricmp(ext.c_str(), ".mat")) {
			MessageBoxA(NULL, "can not save to mat file", "Error", MB_OK);
		}
		else {
			if (s.empty()) {
				thread td = thread([](Material* mat) {
					FileDlgDesc desc;
					desc.title = "imat";
					desc.filter = "imat(*.imat)|*.imat";
					desc.initDir = "./Content";
					desc.save = true;
					desc.defFileExt = "imat";
					desc.addToRecent = false;
					if (openFileDlg(desc)) {
						if (!Material::MaterialLoader::saveMaterialInstance(desc.filePath, *mat))
							MessageBox(NULL, "Save failed", "Error", MB_OK);
						string goodPath = getGoodRelativePath(desc.filePath);
						if (goodPath.empty()) {
							MessageBox(NULL, "File not in engine workfolder", "Warning", MB_OK);
							return;
						}
						string filename = getFileName(goodPath);
						Asset* asset = new Asset(&MaterialAssetInfo::assetInfo, filename, goodPath);
						asset->asset[0] = mat;
						MaterialAssetInfo::assetInfo.regist(*asset);
					}
				}, material);
				td.detach();
			}
			else {
				if (!Material::MaterialLoader::saveMaterialInstance(s, *material))
					MessageBox(NULL, "Save failed", "Error", MB_OK);
			}
		}
	}
	Asset* shaderAsset = NULL;
	vector<string> shaderName = split(material->getShaderName(), '.');
	if (!shaderName.empty()) {
		shaderAsset = AssetManager::getAsset("Material", shaderName[0]);
	}
	if (ImGui::AssetCombo("BaseMaterial", shaderAsset, "Material")) {
		Material* baseMaterial = (Material*)shaderAsset->load();
		if (baseMaterial) {
			material->instantiateFrom(*baseMaterial);
		}
	}
	RenderStage renderStage = enumRenderStage(material->renderOrder);
	static map<RenderStage, const char*> renderStageName = {
		{ RenderStage::RS_PreBackground, "PreBackground" },
		{ RenderStage::RS_Opaque, "Opaque" },
		{ RenderStage::RS_Aplha, "Aplha" },
		{ RenderStage::RS_Transparent, "Transparent" },
		{ RenderStage::RS_Post, "Post" }
	};
	if (ImGui::BeginCombo("RenderOrder", getRenderStageName(renderStage))) {
		for (const auto& item : renderStageName) {
			bool selected = renderStage == item.first;
			if (ImGui::Selectable(item.second, &selected)) {
				material->renderOrder = item.first;
			}
		}
		ImGui::EndCombo();
	}
	ImGui::Checkbox("TwoSide", &material->isTwoSide);
	ImGui::Checkbox("CastShadow", &material->canCastShadow);
	int passNum = material->getPassNum();
	if (ImGui::DragInt("PassNum", &passNum)) {
		material->setPassNum(passNum);
	}
	for (auto b = material->getColorField().begin(), e = material->getColorField().end(); b != e; b++) {
		Color color = b->second.val;
		if (ImGui::ColorEdit4(b->first.c_str(), (float*)&color))
			b->second.val = color;
	}
	for (auto b = material->getScalarField().begin(), e = material->getScalarField().end(); b != e; b++) {
		float val = b->second.val;
		if (ImGui::DragFloat(b->first.c_str(), &val, 0.01))
			b->second.val = val;
	}
	for (auto b = material->getCountField().begin(), e = material->getCountField().end(); b != e; b++) {
		int val = b->second.val;
		if (ImGui::DragInt(b->first.c_str(), &val, 0.01))
			b->second.val = val;
	}
	static string choice;
	ImGui::Columns(2, "TextureColumn", false);
	for (auto b = material->getTextureField().begin(), e = material->getTextureField().end(); b != e; b++) {
		if (b->first == "depthMap")
			continue;
		ImGui::PushID(b._Ptr);
		bool trigger = false;
		if (b->second.val == NULL) {
			trigger = ImGui::Button("##NoTextureBT", { 64, 64 });
		}
		else {
			unsigned long long id = b->second.val->getTextureID();
			trigger = ImGui::ImageButton((ImTextureID)id, { 64, 64 }, { 0, 1 }, { 1, 0 });
		}
		if (trigger) {
			if (b->second.val != NULL)
				TextureViewer::showTexture(*info.gui, *b->second.val);
			//ImGui::OpenPopup("TexSelectPopup");
			choice = b->first;
		}
		ImGui::NextColumn();
		ImGui::Text(b->first.c_str());
		if (ImGui::ArrowButton("AssignTexBT", ImGuiDir_Left)) {
			Asset* assignAsset = EditorManager::getSelectedAsset();
			if (assignAsset != NULL && assignAsset->assetInfo.type == "Texture2D") {
				material->setTexture(b->first, *(Texture2D*)assignAsset->load());
			}
		}
		ImGui::NextColumn();
		ImGui::PopID();
	}
	ImGui::Columns();
	/*if (ImGui::BeginPopup("TexSelectPopup")) {
		for (auto _b = Texture2DAssetInfo::assetInfo.assets.begin(), _e = Texture2DAssetInfo::assetInfo.assets.end();
			_b != _e; _b++) {
			if (_b->second->asset[0] == NULL)
				continue;
			unsigned long long id = ((Texture2D*)_b->second->asset[0])->bind();
			if (id == 0)
				id = Texture2D::blackRGBDefaultTex.bind();
			if (ImGui::Selectable(_b->first.c_str(), false, 0, { 0, 60 })) {
				material->setTexture(choice, *((Texture2D*)_b->second->asset[0]));
			}
			ImGui::SameLine(100);
			ImGui::Image((ImTextureID)id, { 56, 56 });
		}
		ImGui::EndPopup();
	}*/
}

void MaterialEditor::onGUI(EditorInfo& info)
{
	onMaterialGUI(info);
}
