#include "AssetBrowser.h"
#include "../Asset.h"
#include "../Engine.h"
#include "../../ThirdParty/ImGui/imgui_internal.h"
#include "MaterialWindow.h"
#include "TextureViewer.h"
#include "SerializationEditor.h"

AssetBrowser::AssetBrowser(Object & object, string name, bool defaultShow)
	: UIWindow(object, name, defaultShow)
{
	folderTex = getAssetByPath<Texture2D>("Engine/Icons/Folder_Icon.png");
	folderTex = folderTex == NULL ? &Texture2D::blackRGBDefaultTex : folderTex;
	modelTex = getAssetByPath<Texture2D>("Engine/Icons/Model_Icon.png");
	modelTex = modelTex == NULL ? &Texture2D::blackRGBDefaultTex : modelTex;
	materialTex = getAssetByPath<Texture2D>("Engine/Icons/Material_Icon.png");
	materialTex = materialTex == NULL ? &Texture2D::blackRGBDefaultTex : materialTex;
	skeletonMeshTex = getAssetByPath<Texture2D>("Engine/Icons/SkeletonMesh_Icon.png");
	skeletonMeshTex = skeletonMeshTex == NULL ? &Texture2D::blackRGBDefaultTex : skeletonMeshTex;
	animationTex = getAssetByPath<Texture2D>("Engine/Icons/Animation_Icon.png");
	animationTex = animationTex == NULL ? &Texture2D::blackRGBDefaultTex : animationTex;
	audioTex = getAssetByPath<Texture2D>("Engine/Icons/Audio_Icon.png");
	audioTex = audioTex == NULL ? &Texture2D::blackRGBDefaultTex : audioTex;
	assetFileTex = getAssetByPath<Texture2D>("Engine/Icons/AssetFile_Icon.png");
	assetFileTex = assetFileTex == NULL ? &Texture2D::blackRGBDefaultTex : assetFileTex;
	pythonTex = getAssetByPath<Texture2D>("Engine/Icons/Python_Icon.png");
	pythonTex = pythonTex == NULL ? &Texture2D::blackRGBDefaultTex : pythonTex;
	events.registerFunc("getSelectAsset", [](void* browser, void** handle) {
		if (handle != NULL)
			*handle = ((AssetBrowser*)browser)->getSelectedAsset();
	});
	vector<AssetTypeInfo> assetTypeList = {
		{ "Mesh", modelTex }, { "SkeletonMesh", skeletonMeshTex },
		{ "AnimationClipData", animationTex }, { "Material", materialTex },
		{ "AudioData", audioTex }, { "AssetFile", assetFileTex },
		{ "PythonScript", pythonTex }, { "Texture2D", NULL }
	};
	for (int i = 0; i < assetTypeList.size(); i++) {
		AssetTypeInfo& info = assetTypeList[i];
		info.typeID = 1 << assetTypes.size();
		assetTypes.insert(info.name, info);
		assetTypeFilter |= info.typeID;
	}
}

void AssetBrowser::setCurrentPath(const string & path)
{
	updatePath(path);
}

Asset * AssetBrowser::getSelectedAsset()
{
	if (seletedIndex == -1 || seletedIndex < subFolders.size())
		return NULL;
	else if (seletedIndex < (subFolders.size() + assets.size())) {
		return assets[seletedIndex - subFolders.size()];
	}
	return NULL;
}

void AssetBrowser::onAttech(GUI & gui)
{
	//updatePath(curFolder, true);
}

void AssetBrowser::onRenderWindow(GUIRenderInfo & info)
{
	if (pathChain.size() > 8)
		ImGui::Text("Path: ... ");
	else
		ImGui::Text("Path: ");
	string topath;
	int s = max(0LLU, pathChain.size() - 8);
	for (int i = 0; i < pathChain.size(); i++) {
		topath += pathChain[i];
		if (i >= s) {
			ImGui::SameLine();
			if (ImGui::Button(pathChain[i].c_str())) {
				updatePath(topath);
				break;
			}
		}
		topath += '/';
	}
	ImGui::Text("Filter: ");
	ImGui::SameLine();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::BeginChild("AssetFilterView", { 0, 36 }, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoScrollbar);
	for (int i = 0; i < assetTypes.size(); i++) {
		ImGui::PushID(i);
		unsigned int enumID = assetTypes.at(i).typeID;
		bool has = assetTypeFilter & enumID;
		ImVec4 btcol = ImGui::GetStyleColorVec4(ImGuiCol_Button);
		if (has)
			ImGui::PushStyleColor(ImGuiCol_Button, { 0.00f, 0.68f, 0.84f, 1.00f });
		else
			ImGui::PushStyleColor(ImGuiCol_Button, btcol);
		if (ImGui::Button(assetTypes.at(i).name.c_str())) {
			if (assetTypeFilterBackup != 0) {
				assetTypeFilter = assetTypeFilterBackup | enumID;
				assetTypeFilterBackup = 0;
			}
			else {
				if (!has)
					assetTypeFilter |= enumID;
				else
					assetTypeFilter ^= enumID;
			}
		}
		if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
			if (assetTypeFilterBackup == 0) {
				assetTypeFilterBackup = assetTypeFilter;
				assetTypeFilter = enumID;
			}
			else {
				assetTypeFilter = assetTypeFilterBackup;
				assetTypeFilterBackup = 0;
			}
		}
		ImGui::PopStyleColor();
		ImGui::SameLine();
		ImGui::PopID();
	}
	ImGui::EndChild();
	ImGui::PopStyleVar(2);

	ImGui::BeginChild("AssetsView", { -1, -1 });
	ImGui::Indent(16);
	float width = ImGui::GetWindowWidth();
	ImGui::BeginColumns("AssetColumn", max(width / itemWidth, 1.f), ImGuiColumnsFlags_NoBorder);
	int index = 0;
	for (int i = 0; i < subFolders.size(); i++, index++) {
		ImGui::PushID(index);
		if (Item(subFolders[i], *folderTex, 18, seletedIndex == index)) {
			if (ImGui::IsMouseDoubleClicked(0)) {
				string p = curFolder;
				if (!updatePath(curFolder + '/' + subFolders[i]))
					updatePath(p);
				ImGui::PopID();
				ImGui::EndColumns();
				ImGui::EndChild();
				return;
			}
			else {
				seletedIndex = index;
				Asset* asset = getSelectedAsset();
				info.gui.setParameter("SelectedAsset", asset);
				Events* e = info.gui.getUIControlEvent("Inspector");
				if (e != NULL)
					e->call("assignAsset", asset);
			}
		}
		ImGui::PopID();
		ImGui::NextColumn();
	}
	for (int i = 0; i < assets.size(); i++, index++) {
		auto iter = assetTypes.find(assets[i]->assetInfo.type);
		if (iter == assetTypes.end())
			continue;
		if (!(assetTypeFilter & iter->typeID)) {
			if (seletedIndex == index)
				seletedIndex = -1;
			continue;
		}
		Texture2D* tex = iter->typeTex == NULL ? (Texture2D*)assets[i]->load() : iter->typeTex;
		if (tex == NULL)
			continue;
		ImGui::PushID(index);
		if (Item(assets[i]->name, *tex, 18, seletedIndex == index)) {
			seletedIndex = index;
			Asset* asset = getSelectedAsset();
			info.gui.setParameter("SelectedAsset", asset);
			Events* e = info.gui.getUIControlEvent("Inspector");
			if (e != NULL)
				e->call("assignAsset", asset);
		}
		bool canPreview = isFocus() && ImGui::IsMouseDoubleClicked(0) && seletedIndex == index;
		if (assets[i]->assetInfo.type == "Mesh" || assets[i]->assetInfo.type == "SkeletonMesh") {
			if (ImGui::BeginPopupContextItem("MeshContext")) {
				static int selectedId = 0;
				static char name[100];
				static Material* selectedMat = NULL;
				vector<string> items;
				string str;

				ImGui::Text("Mesh: %s", assets[i]->name.c_str());

				for (auto b = MaterialAssetInfo::assetInfo.assets.begin(), e = MaterialAssetInfo::assetInfo.assets.end(); b != e; b++) {
					if (selectedMat == NULL)
						selectedMat = (Material*)b->second->load();
					items.push_back(b->first);
					str += b->first + '\0';
				}
				if (ImGui::Combo("Base Material", &selectedId, str.c_str())) {
					selectedMat = (Material*)(MaterialAssetInfo::assetInfo.assets[items[selectedId]]->asset[0]);
				}
				static bool twoSides = false;
				static bool castShadow = false;
				ImGui::Checkbox("TwoSides", &twoSides);
				ImGui::Checkbox("CastShadow", &castShadow);
				ImGui::InputText("Display Name", name, 100);
				ImGui::BeginGroup();
				static int phyMatCT = 0;
				static float mass = 0;
				ImGui::Combo("Physical Type", &phyMatCT, "Static\0Dynamic\0NoCollision\0");
				ImGui::InputFloat("Mass", &mass, 0.01);
				static bool simple = true;
				if (assets[i]->assetInfo.type == "Mesh")
					ImGui::Checkbox("Use Simple Collision", &simple);
				static float pos[3] = { 0, 0, 0 };
				ImGui::DragFloat3("Postion", pos, 0.1);
				static float rot[3] = { 0, 0, 0 };
				ImGui::DragFloat3("Rotation", rot, 0.1);
				static float sca[3] = { 1, 1, 1 };
				ImGui::DragFloat3("Scale", sca, 0.1);
				ImGui::EndGroup();
				ImVec2 size = { -1, 40 };
				if (strlen(name) != 0 && Brane::find(typeid(Object).hash_code(), name) == NULL)
					if (ImGui::Button("Load", size)) {
						Material &mat = *selectedMat;
						mat.setTwoSide(twoSides);
						if (assets[i]->assetInfo.type == "Mesh") {
							Mesh* mesh = (Mesh*)assets[i]->load();
							MeshActor* a = new MeshActor(*mesh, mat, { mass, (PhysicalType)phyMatCT }, *mesh, name, simple ? SIMPLE : COMPLEX, { sca[0], sca[1], sca[2] });
							a->meshRender.canCastShadow = castShadow;
							a->setPosition(pos[0], pos[1], pos[2]);
							a->setRotation(rot[0], rot[1], rot[2]);
							object.addChild(*a);
						}
						else if (assets[i]->assetInfo.type == "SkeletonMesh") {
							SkeletonMesh* mesh = (SkeletonMesh*)assets[i]->load();
							SkeletonMeshActor* a = new SkeletonMeshActor(*mesh, mat, name);
							a->skeletonMeshRender.canCastShadow = castShadow;
							a->setPosition(pos[0], pos[1], pos[2]);
							a->setRotation(rot[0], rot[1], rot[2]);
							a->setScale(sca[0], sca[1], sca[2]);
							object.addChild(*a);
						}
					}
				ImGui::EndPopup();
			}
		}
		else if (assets[i]->assetInfo.type == "Material") {
			if (canPreview) {
				MaterialWindow *win = dynamic_cast<MaterialWindow*>(info.gui.getUIControl("MaterialWindow"));
				if (win == NULL) {
					win = new MaterialWindow();
					info.gui.addUIControl(*win);
				}
				win->show = true;
				win->setMaterial(*(Material*)assets[i]->load());
			}
		}
		else if (assets[i]->assetInfo.type == "AssetFile") {
			if (canPreview) {
				SerializationEditor::showSerializationInfo(info.gui, *(SerializationInfo*)assets[i]->load());
			}
			if (ImGui::BeginPopupContextItem("AssetContext")) {
				static char name[100];
				ImGui::InputText("Name", name, 100);
				if (strlen(name) != 0 && Brane::find(typeid(Object).hash_code(), name) == NULL)
					if (ImGui::Button("Import to world", { -1, 36 })) {
						SerializationInfo* info = (SerializationInfo*)assets[i]->load();
						string name_bk = info->name;
						info->name = name;
						Serializable* ser = info->serialization->instantiate(*info);
						info->name = name_bk;
						if (ser != NULL) {
							Object* obj = dynamic_cast<Object*>(ser);
							if (obj == NULL) {
								delete ser;
							}
							else {
								if (obj->deserialize(*info))
									object.addChild(*obj);
								else
									delete ser;
							}
						}
					}
				ImGui::EndPopup();
			}
		}
		else if (assets[i]->assetInfo.type == "Texture2D") {
			if (canPreview) {
				TextureViewer::showTexture(info.gui, *(Texture2D*)assets[i]->load());
			}
		}
		ImGui::PopID();
		ImGui::NextColumn();
	}
	ImGui::EndColumns();
	ImGui::EndChild();
}

bool AssetBrowser::updatePath(const string & path, bool force)
{
	if (path.empty())
		return false;
	if (!force && curFolder == path)
		return true;
	if (!filesystem::exists(path))
		return false;
	curFolder = path;
	seletedIndex = -1;
	pathChain = split(path, '/');
	subFolders.clear();
	assets.clear();
	for (auto& p : filesystem::directory_iterator(path)) {
		filesystem::file_type type = p.status().type();
		string _path = p.path().generic_string();
		string _ext = p.path().extension().generic_string();
		if (type == filesystem::file_type::directory) {
			subFolders.push_back(p.path().filename().generic_string());
		}
		else if (type == filesystem::file_type::regular) {
			Asset* a = AssetInfo::getAssetByPath(_path);
			if (a != NULL) {
				assets.push_back(a);
			}
			if (!_stricmp(_ext.c_str(), ".fbx")) {
				findSimilar(AssetInfo::assetsByPath, _path + ':', assets);
			}
		}
	}
	return true;
}

bool AssetBrowser::Item(const string & name, Texture2D & tex, float pad, bool isSelected)
{
	ImVec2 pos = ImGui::GetCurrentWindow()->DC.CursorPos;
	pos.x -= 8;
	ImVec2 ts = ImGui::CalcTextSize(name.c_str(), name.c_str() + name.size());
	ImGui::SetNextItemWidth(itemWidth);
	bool click = ImGui::Selectable(("##" + name).c_str(), isSelected,
		ImGuiSelectableFlags_AllowDoubleClick, { itemWidth, itemWidth - pad * 2 + ts.y });
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(name.c_str());
	ImDrawList* dl = ImGui::GetCurrentWindow()->DrawList;
	dl->AddImage((ImTextureID)tex.getTextureID(), ImVec2(pos.x + pad, pos.y), ImVec2(pos.x + itemWidth - pad, pos.y + itemWidth - pad * 2));
	dl->AddCircle(ImVec2(pos.x + pad / 2, pos.y + pad / 2), 5, ImColor(255, 255, 255));
	if (isSelected)
		dl->AddCircleFilled(ImVec2(pos.x + pad / 2, pos.y + pad / 2), 3.5, ImColor(255, 255, 255));
	ImVec2 spos;
	if (ts.x < itemWidth - 4) {
		spos.x = pos.x + 2 + (itemWidth - 4 - ts.x) / 2;
		spos.y = pos.y + itemWidth - pad * 2;
	}
	else {
		spos.x = pos.x + 2;
		spos.y = pos.y + itemWidth - pad * 2;
	}
	ImVec4 clip = ImVec4(pos.x + 2, pos.y + itemWidth - pad * 2, pos.x + itemWidth - 2, pos.y + itemWidth - pad * 2 + ts.y);
	dl->AddText(ImGui::GetFont(), ImGui::GetFontSize(), spos, ImColor(1.0f, 1.0f, 1.0f),
		name.c_str(), name.c_str() + name.size(), 0, &clip);
	return click;
}
