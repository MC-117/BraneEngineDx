#include "AssetBrowser.h"
#include "../Asset.h"
#include "../Engine.h"
#include "../Editor/Editor.h"
#include "../Script/ScriptWindow.h"
#include "../../ThirdParty/ImGui/imgui_internal.h"
#include "MaterialWindow.h"
#include "TextureViewer.h"
#include "SerializationEditor.h"
#include "ObjectPreviewWindow.h"
#include "../Timeline/TimelineWindow.h"
#include "../Graph/GraphWindow.h"
#include "AnimationDataWindow.h"
#include "GUIUtility.h"
#include "../Utility/EngineUtility.h"
#include "../MeshActor.h"
#include "../SkeletonMeshActor.h"
#include "../../Spine2D/Spine2DActor.h"
#include "Core/ShaderCode/ShaderGraph/ShaderGraph.h"

AssetBrowser::AssetBrowser(Object & object, string name, bool defaultShow)
	: UIWindow(object, name, defaultShow)
{
	folderTex = getAssetByPath<Texture2D>("Engine/Icons/Folder_Icon.png");
	folderTex = folderTex == NULL ? &Texture2D::blackRGBDefaultTex : folderTex;
	modelTex = getAssetByPath<Texture2D>("Engine/Icons/Model_Icon.png");
	modelTex = modelTex == NULL ? &Texture2D::blackRGBDefaultTex : modelTex;
	materialTex = getAssetByPath<Texture2D>("Engine/Icons/Material_Icon.png");
	materialTex = materialTex == NULL ? &Texture2D::blackRGBDefaultTex : materialTex;
	genericShaderTex = getAssetByPath<Texture2D>("Engine/Icons/GenericShader_Icon.png");
	genericShaderTex = genericShaderTex == NULL ? &Texture2D::blackRGBDefaultTex : genericShaderTex;
	shaderGraphTex = getAssetByPath<Texture2D>("Engine/Icons/ShaderGraph_Icon.png");
	shaderGraphTex = shaderGraphTex == NULL ? &Texture2D::blackRGBDefaultTex : shaderGraphTex;
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
	timelineTex = getAssetByPath<Texture2D>("Engine/Icons/Timeline_Icon.png");
	timelineTex = timelineTex == NULL ? &Texture2D::blackRGBDefaultTex : timelineTex;
	graphTex = getAssetByPath<Texture2D>("Engine/Icons/Graph_Icon.png");
	graphTex = graphTex == NULL ? &Texture2D::blackRGBDefaultTex : graphTex;
	events.registerFunc("getSelectAsset", [](void* browser, void** handle) {
		if (handle != NULL)
			*handle = ((AssetBrowser*)browser)->getSelectedAsset();
	});
	vector<AssetTypeInfo> assetTypeList = {
		{ MeshType, modelTex }, { SkeletonMeshType, skeletonMeshTex },
		{ AnimationClipDataType, animationTex }, { MaterialType, materialTex },
		{ GenericShaderType, genericShaderTex }, { ShaderGraphType, shaderGraphTex },
		{ AudioDataType, audioTex }, { AssetFileType, assetFileTex },
		{ PythonScriptType, pythonTex }, { Live2DModelType, modelTex }, { Spine2DModelType, modelTex },
		{ Texture2DType, NULL }, { TimelineType, timelineTex }, { GraphType, graphTex }
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

void AssetBrowser::onWindowGUI(GUIRenderInfo & info)
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
				if (asset != NULL)
					EditorManager::selectAsset(asset);
			}
		}
		ImGui::PopID();
		ImGui::NextColumn();
	}
	if (ImGui::BeginPopupContextWindow("FolderContext")) {
		if (ImGui::Button("RefreshFolder", { 120, 36 })) {
			refreshNewAsset(curFolder);
		}
		ImGui::EndPopup();
	}
	for (int i = 0; i < assets.size(); i++, index++) {
		Asset* asset = assets[i];
		Name typeName = getAssetTypeName(asset);
		auto iter = assetTypes.find(typeName);
		if (iter == assetTypes.end())
			continue;
		if (!(assetTypeFilter & iter->typeID)) {
			if (seletedIndex == index)
				seletedIndex = -1;
			continue;
		}
		void* assetPtr = asset->load();
		if (assetPtr == NULL)
			continue;
		Texture2D* tex = typeName == Texture2DType ? (Texture2D*)asset->load() : iter->typeTex;
		if (tex == NULL)
			continue;
		ImGui::PushID(index);
		if (Item(assets[i]->name, *tex, 18, seletedIndex == index)) {
			seletedIndex = index;
			Asset* selectedAsset = getSelectedAsset();
			if (selectedAsset != NULL)
				EditorManager::selectAsset(selectedAsset);
		}
		bool canPreview = isFocus() && ImGui::IsMouseDoubleClicked(0) && seletedIndex == index;
		if (typeName == MeshType || typeName == SkeletonMeshType) {
			if (ImGui::BeginPopupContextItem("MeshContext")) {
				static char name[100];
				static Material* selectedMat = NULL;

				ImGui::Text("Mesh: %s", assets[i]->name.c_str());
				Asset* selectedAsset = MaterialAssetInfo::assetInfo.getAsset(selectedMat);
				if (ImGui::AssetCombo("Base Material", selectedAsset, MaterialType.c_str())) {
					selectedMat = (Material*)(selectedAsset->load());
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
				if (typeName == MeshType)
					ImGui::Checkbox("Use Simple Collision", &simple);
				static float pos[3] = { 0, 0, 0 };
				ImGui::DragFloat3("Postion", pos, 0.1);
				static float rot[3] = { 0, 0, 0 };
				ImGui::DragFloat3("Rotation", rot, 0.1);
				static float sca[3] = { 1, 1, 1 };
				ImGui::DragFloat3("Scale", sca, 0.1);
				ImGui::EndGroup();
				ImVec2 size = { -1, 40 };
				if (strlen(name) != 0 && Engine::getCurrentWorld()->findChild(name) == NULL)
					if (ImGui::Button("Load", size)) {
						if (selectedMat)
							selectedMat->setTwoSide(twoSides);
						if (typeName == MeshType) {
							Mesh* mesh = (Mesh*)assets[i]->load();
							MeshActor* a = new MeshActor(name);
							a->meshRender.setMesh(mesh);
							if (selectedMat)
								a->meshRender.setMaterial(0, *selectedMat);
							a->setScale({ sca[0], sca[1], sca[2] });
							a->rigidBody = new RigidBody(*a, { mass, (PhysicalType)phyMatCT });
							a->rigidBody->addCollider(mesh, simple ? SIMPLE : COMPLEX);
							a->meshRender.canCastShadow = castShadow;
							a->setPosition(pos[0], pos[1], pos[2]);
							a->setRotation(rot[0], rot[1], rot[2]);
							object.addChild(*a);
						}
						else if (typeName == SkeletonMeshType) {
							SkeletonMesh* mesh = (SkeletonMesh*)assets[i]->load();
							SkeletonMeshActor* a = new SkeletonMeshActor(name);
							a->addSkeletonMesh(*mesh);
							if (selectedMat)
								a->skeletonMeshRenders[0]->setMaterial(0, *selectedMat);
							a->setCastShadow(castShadow);
							a->setPosition(pos[0], pos[1], pos[2]);
							a->setRotation(rot[0], rot[1], rot[2]);
							a->setScale(sca[0], sca[1], sca[2]);
							object.addChild(*a);
						}
					}
				ImGui::EndPopup();
			}
		}
		else if (typeName == MaterialType) {
			if (canPreview) {
				Material* material = (Material*)assetPtr;
				MaterialWindow *win = dynamic_cast<MaterialWindow*>(info.gui.getUIControl("MaterialWindow"));
				if (win == NULL) {
					win = new MaterialWindow();
					info.gui.addUIControl(*win);
				}
				win->show = true;
				win->setMaterial(*material);
			}
		}
		else if (typeName == GenericShaderType) {
			if (canPreview) {
				Material* material = (Material*)assetPtr;
				GenericShader* shader = dynamic_cast<GenericShader*>(material->getShader());
				if (shader) {
					ScriptWindow::OpenTempScript(asset->path);
				}
			}
		}
		else if (typeName == ShaderGraphType) {
			if (canPreview) {
				Material* material = (Material*)assetPtr;
				ShaderGraph* shaderGraph = dynamic_cast<ShaderGraph*>(material->getShader());
				if (shaderGraph)
					GraphWindow::showGraph(info.gui, shaderGraph);
			}
		}
		else if (typeName == PythonScriptType) {
			if (canPreview) {
				PythonScript* script = (PythonScript*)assets[i]->load();
				if (script != NULL)
					ScriptWindow::OpenScript(*script);
			}
		}
		else if (typeName == AssetFileType) {
			if (canPreview) {
				SerializationInfo* sinfo = (SerializationInfo*)assets[i]->load();
				if (sinfo != NULL && sinfo->serialization != NULL && sinfo->serialization->type != "World")
					ObjectPreviewWindow::showObject(info.gui, *sinfo);
			}
			if (ImGui::BeginPopupContextItem("AssetContext")) {
				if (ImGui::Button("View Raw", { -1, 36 })) {
					SerializationEditor::showSerializationInfo(info.gui, *(SerializationInfo*)assets[i]->load());
				}
				static char name[100];
				ImGui::InputText("Name", name, 100);
				if (strlen(name) != 0 && Engine::getCurrentWorld()->findChild(name) == NULL)
					if (ImGui::Button("Import to world", { -1, 36 })) {
						SerializationInfo* info = (SerializationInfo*)assets[i]->load();
						SerializationInfo cloneInfo = *info;
						cloneInfo.name = name;
						newSerializationInfoGuid(cloneInfo);
						Serializable* ser = cloneInfo.serialization->instantiate(cloneInfo);
						if (ser != NULL) {
							Object* obj = dynamic_cast<Object*>(ser);
							if (obj == NULL) {
								delete ser;
							}
							else {
								if (obj->deserialize(cloneInfo))
									object.addChild(*obj);
								else
									delete ser;
							}
						}
					}
				ImGui::EndPopup();
			}
		}
		else if (typeName == Live2DModelType) {
			if (canPreview) {
				ObjectPreviewWindow::showObject(info.gui, *assets[i]);
			}
		}
		else if (typeName == Spine2DModelType) {
			if (canPreview) {
				ObjectPreviewWindow::showObject(info.gui, *assets[i]);
			}
			if (ImGui::BeginPopupContextItem("Spine2DModelContext")) {
				static char name[100];
				static Material* selectedMat = NULL;

				ImGui::Text("Mesh: %s", assets[i]->name.c_str());
				ImGui::InputText("Display Name", name, 100);
				ImGui::BeginGroup();
				static float pos[3] = { 0, 0, 0 };
				ImGui::DragFloat3("Postion", pos, 0.1);
				static float rot[3] = { 0, 0, 0 };
				ImGui::DragFloat3("Rotation", rot, 0.1);
				static float sca[3] = { 1, 1, 1 };
				ImGui::DragFloat3("Scale", sca, 0.1);
				ImGui::EndGroup();
				ImVec2 size = { -1, 40 };
				if (strlen(name) != 0 && Engine::getCurrentWorld()->findChild(name) == NULL) {
					if (ImGui::Button("Load", size)) {
						Spine2DModel* model = (Spine2DModel*)assets[i]->load();
						Spine2DActor* a = new Spine2DActor(name);
						a->setModel(model);
						a->setScale({ sca[0], sca[1], sca[2] });
						a->setPosition(pos[0], pos[1], pos[2]);
						a->setRotation(rot[0], rot[1], rot[2]);
						object.addChild(*a);
					}
				}
				ImGui::EndPopup();
			}
		}
		else if (typeName == Texture2DType) {
			if (canPreview) {
				TextureViewer::showTexture(info.gui, *(Texture2D*)assets[i]->load());
			}
		}
		else if (typeName == TimelineType) {
			if (canPreview) {
				TimelineWindow::showTimeline(info.gui, (Timeline*)assets[i]->load());
			}
		}
		else if (typeName == AnimationClipDataType) {
			if (canPreview) {
				AnimationDataWindow::showAnimationData(info.gui, (AnimationClipData*)assets[i]->load());
			}
		}
		else if (typeName == GraphType) {
			Graph* graph = (Graph*)assets[i]->load();
			if (canPreview) {
				GraphWindow::showGraph(info.gui, graph);
			}
			if (ImGui::BeginPopupContextItem("GraphContext")) {
				if (ImGui::Button("Open", { -1, 36 })) {
					GraphWindow::showGraph(info.gui, graph);
				}
				ImGui::Separator();
				EditorInfo editorInfo = { &info.gui, info.gizmo, info.camera, Engine::getCurrentWorld() };
				for (int i = 0; i < graph->getInputCount(); i++) {
					GraphPin* pin = graph->getInput(i);
					GraphPinEditor* editor = dynamic_cast<GraphPinEditor*>(EditorManager::getEditor(*pin));
					if (editor)
						editor->onInspectGUI(editorInfo);
				}
				if (ImGui::Button("Run", { -1, 36 })) {
					GraphContext context;
					graph->solveState(context);
				}
				ImGui::EndPopup();
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
		string _path = p.path().generic_u8string();
		string _ext = p.path().extension().generic_u8string();
		if (type == filesystem::file_type::directory) {
			subFolders.push_back(p.path().filename().generic_u8string());
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

Name AssetBrowser::getAssetTypeName(Asset* asset)
{
	Name name = asset->assetInfo.type;
	if (name == MaterialType) {
		Material* material = (Material*)asset->load();
		if (isBaseMaterial(*material)) {
			Shader* shader = material->getShader();
			if (isClassOf<GenericShader>(shader))
				name = GenericShaderType;
			else if (isClassOf<ShaderGraph>(shader))
				name = ShaderGraphType;
			else
				name = Name::none;
		}
	}
	return name;
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

bool AssetBrowser::refreshNewAsset(const string& path)
{
	if (!filesystem::is_directory(path) || !filesystem::exists(path))
		return false;
	for (auto& p : filesystem::directory_iterator(path)) {
		filesystem::file_type type = p.status().type();
		string assetPath = p.path().generic_u8string();
		if (type == filesystem::file_type::regular) {
			Asset* asset = AssetInfo::getAssetByPath(assetPath);
			if (asset == NULL) {
				ImportInfo info(assetPath);
				ImportResult result;
				if (IImporter::load(info, result)) {
					for (Asset* asset : result.assets)
						assets.push_back(asset);
				}
			}
		}
	}
	return false;
}
