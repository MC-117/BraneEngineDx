#include <fstream>
#include "EditorWindow.h"
#include "../Engine.h"
#include "AssetLoadWindow.h"
#include "ConsoleWindow.h"
#include "AssetBrowser.h"
#include "../../ThirdParty/ImGui/imgui_stdlib.h"
#include "../GrassMeshActor.h"
#include "../PostProcess/PostProcessingCamera.h"

EditorWindow::EditorWindow(Object & object, Material& baseMat, string name, bool defaultShow) : UIWindow(object, name, defaultShow), baseMat(baseMat)
{
}

string nextName(const string& name)
{
	int num = 0, i = 0;
	for (; i < name.size(); i++) {
		char c = name[name.size() - 1 - i];
		if (isdigit(c))
			num += (c - '0') * pow(10, i);
		else
			break;
	}
	if (num == 0 && i == 0)
		return name + '1';
	return name.substr(0, name.size() - i) + to_string(num + 1);
}

string getNextName(const string& name)
{
	string newName = nextName(name);
	while(Brane::find(typeid(Object).hash_code(), newName) != NULL)
		newName = nextName(newName);
	return newName;
}

void EditorWindow::onRenderWindow(GUIRenderInfo& info)
{
	World* w = dynamic_cast<World*>(&object);
	if (w == NULL)
		return;
	World& world = *w;
	GUI& gui = info.gui;
	ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
	ImGui::Separator();
	ImVec2 size = { ImGui::GetWindowContentRegionWidth() / 2 - 4, 40 };
	if (ImGui::Button("Load", size)) {
		UIControl* uc = gui.getUIControl("LoadAsset");
		bool b = false;
		if (uc != NULL) {
			AssetLoadWindow* alw = dynamic_cast<AssetLoadWindow*>(uc);
			if (alw != NULL) {
				alw->show = true;
			}
			else
				b = true;
		}
		else
			b = true;
		if (b) {
			AssetLoadWindow* win = new AssetLoadWindow(world, "LoadAsset", true);
			world += win;
		}
		//world.loadTransform("Save.world");
		/*CFileDialog dialog(true, _T("*.world"), NULL, 6UL, _T("World(*.world)|*.world|All Files(*.*)|*.*"));
		if (dialog.DoModal() == IDOK) {
		if (!world.loadTransform(string(CT2A(dialog.GetPathName().GetString()))))
		MessageBox(NULL, _T("Load failed"), _T("Alert"), MB_OK);
		}*/
	}
	ImGui::SameLine();
	if (ImGui::Button("Console", size)) {
		UIControl* uc = gui.getUIControl("Console");
		bool b = false;
		if (uc != NULL) {
			ConsoleWindow* win = dynamic_cast<ConsoleWindow*>(uc);
			if (win != NULL) {
				win->show = true;
			}
			else
				b = true;
		}
		else
			b = true;
		if (b) {
			ConsoleWindow* win = new ConsoleWindow(world, "Console", true);
			world += win;
		}
		//world.saveTransform("Save.world");
		/*CFileDialog dialog(false, _T("*.world"), NULL, 6UL, _T("World(*.world)|*.world|All Files(*.*)|*.*"));
		if (dialog.DoModal() == IDOK) {
		if (!world.saveTransform(string(CT2A(dialog.GetPathName().GetString()))))
		MessageBox(NULL, _T("Load failed"), _T("Alert"), MB_OK);
		}*/
	}
	if (ImGui::Button("Engine Browser", size)) {
		UIControl* uc = gui.getUIControl("Engine Browser");
		bool b = false;
		if (uc != NULL) {
			AssetBrowser* win = dynamic_cast<AssetBrowser*>(uc);
			if (win != NULL) {
				win->show = true;
			}
			else
				b = true;
		}
		else
			b = true;
		if (b) {
			AssetBrowser* win = new AssetBrowser(world, "Engine Browser", true);
			win->setCurrentPath("Engine");
			world += win;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Content Browser", size)) {
		UIControl* uc = gui.getUIControl("Content Browser");
		bool b = false;
		if (uc != NULL) {
			AssetBrowser* win = dynamic_cast<AssetBrowser*>(uc);
			if (win != NULL) {
				win->show = true;
			}
			else
				b = true;
		}
		else
			b = true;
		if (b) {
			AssetBrowser* win = new AssetBrowser(world, "Content Browser", true);
			win->setCurrentPath("Content");
			world += win;
		}
	}
	if (!alwaysShow) {
		if (ImGui::Button("Hide UI", size)) {
			world.input.setCursorHidden(true);
			gui.hideAllUIControl();
		}
		ImGui::SameLine();
	}
	if (!show)
		return;
	if (ImGui::Button("Quit Editor", size))
		world.quit();

#ifdef AUDIO_USE_OPENAL
	if (ImGui::SliderFloat("Volume", &mainVolume, 0, 1)) {
		world.setMainVolume(mainVolume);
	}
#endif // AUDIO_USE_OPENAL
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Objects");
	ImGui::BeginChild("ObjectsView", { -0.1f, -0.1f });

	// ObjectContext
	if (ImGui::BeginPopupContextWindow("WorldContextMenu")) {
		objectContextMenu(NULL);
		ImGui::EndPopup();
	}

	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow;
	if (selectedObj == &world)
		node_flags |= ImGuiTreeNodeFlags_Selected;

	// ObjectTree
	if (ImGui::TreeNodeEx("world", node_flags)) {
		Object *dragObj = NULL, *targetObj = NULL;

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EDITOR_DRAG"))
			{
				if (payload->DataSize == sizeof(Object*)) {
					dragObj = *(Object**)payload->Data;
					targetObj = &world;
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::IsItemClicked()) {
			selectedObj = &world;
			UIControl* uc = gui.getUIControl("Inspector");
			bool b = false;
			if (uc != NULL) {
				InspectorWindow* ipw = dynamic_cast<InspectorWindow*>(uc);
				if (ipw != NULL) {
					ipw->setTargetObject(world);
					ipw->show = true;
				}
				else
					b = true;
			}
			else
				b = true;
			if (b) {
				InspectorWindow* win = new InspectorWindow(world, "Inspector", true);
				world += win;
			}
		}
		traverse(world, gui, dragObj, targetObj);

		if (dragObj != NULL && targetObj != NULL &&
			targetObj != dragObj && targetObj != dragObj->parent) {
			dragObj->setParent(*targetObj);
		}
		ImGui::TreePop();
	}

	ImGui::EndChild();
}

void EditorWindow::onPostAction(GUIPostInfo & info)
{
	World* w = dynamic_cast<World*>(&object);
	if (w == NULL)
		return;
	World& world = *w;
	if (isFocus() || info.focusControl == NULL) {
		if (world.input.getKeyDown(VK_CONTROL)) {
			if (selectedObj != NULL &&
				world.input.getKeyPress('C')) {
				copyInfo.clear();
				if (selectedObj->serialize(copyInfo)) {
					parentObj = selectedObj->parent;
					copyInfo.serialization = SerializationManager::getSerialization(copyInfo.type);
				}
				else {
					parentObj = NULL;
					copyInfo.clear();
				}
			}
			else if (copyInfo.serialization != NULL &&
				world.input.getKeyPress('V')) {
				string oldName = copyInfo.name;
				string newName = getNextName(copyInfo.name);
				copyInfo.name = newName;
				Serializable* ser = copyInfo.serialization->instantiate(copyInfo);
				if (ser != NULL) {
					Object* obj = dynamic_cast<Object*>(ser);
					if (obj == NULL)
						delete ser;
					else {
						Object* pobj = parentObj == NULL ? &world : parentObj;
						obj->deserialize(copyInfo);
						pobj->addChild(*obj);
						select(obj, info.gui);
					}
				}
				copyInfo.name = oldName;
			}
		}
	}
}

void EditorWindow::traverse(Object & obj, GUI& gui, Object*& dragObj, Object*& targetObj)
{
	int i = 0;
	for (auto b = obj.children.begin(), e = obj.children.end(); b != e; b++, i++) {
		Object& obj = **b;
		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow;
		if (obj.children.empty())
			node_flags |= ImGuiTreeNodeFlags_Leaf;
		if (selectedObj == &obj)
			node_flags |= ImGuiTreeNodeFlags_Selected;
		ImGui::PushID(i);
		bool isOpen = ImGui::TreeNodeEx(obj.name.c_str(), node_flags);
		if (ImGui::BeginPopupContextItem("ObjectContextMenu")) {
			objectContextMenu(&obj);
			ImGui::EndPopup();
		}
		if (ImGui::BeginDragDropSource())
		{
			Object* p = *b;
			ImGui::SetDragDropPayload("EDITOR_DRAG", &p, sizeof(Object*));
			ImGui::Text("Reparent %s", obj.name.c_str());
			ImGui::EndDragDropSource();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EDITOR_DRAG"))
			{
				if (payload->DataSize == sizeof(Object*)) {
					dragObj = *(Object**)payload->Data;
					targetObj = *b;
				}
			}
			ImGui::EndDragDropTarget();
		}
		if (ImGui::IsItemClicked()) {
			select(&obj, gui);
		}
		if (isOpen) {
			traverse(obj, gui, dragObj, targetObj);
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
}

void EditorWindow::select(Object * obj, GUI& gui)
{
	selectedObj = obj;
	InspectorWindow* ipw = dynamic_cast<InspectorWindow*>(gui.getUIControl("Inspector"));
	if (ipw == NULL) {
		ipw = new InspectorWindow(world, "Inspector", true);
		world += ipw;

	}
	ipw->setTargetObject(*obj);
	ipw->show = true;
}

void EditorWindow::meshCombo()
{
	vector<Mesh*> meshes;
	string meshstr;
	for (auto b = MeshAssetInfo::assetInfo.assets.begin(), e = MeshAssetInfo::assetInfo.assets.end(); b != e; b++) {
		meshes.push_back((Mesh*)b->second->load());
		meshstr += b->first + '\0';
	}
	if (ImGui::Combo("Mesh", &selectedMeshID, meshstr.c_str())) {
		selectedMesh = meshes[selectedMeshID];
	}
}

void EditorWindow::materialCombo()
{
	vector<Material*> materials;
	string materialstr;
	for (auto b = MaterialAssetInfo::assetInfo.assets.begin(), e = MaterialAssetInfo::assetInfo.assets.end(); b != e; b++) {
		materials.push_back((Material*)b->second->load());
		materialstr += b->first + '\0';
	}
	if (ImGui::Combo("Material", &selectedMaterialID, materialstr.c_str())) {
		selectedMaterial = materials[selectedMaterialID];
	}
}

void EditorWindow::objectContextMenu(Object * obj)
{
	Object& target = obj == NULL ? object : *obj;
	ImGui::Text("%s(%s)", target.getSerialization().type.c_str(), target.name.c_str());
	ImGui::Separator();
	ImGui::PushID("ContextMenu");
	if (ImGui::BeginMenu("New Object")) {
		
		// Transform
		if (ImGui::BeginMenu("Transform")) {
			ImGui::InputText("Name", &newObjectName);
			if (Brane::find(typeid(Object).hash_code(), newObjectName) == NULL) {
				if (ImGui::Button("Create", { -1, 36 })) {
					::Transform* t = new ::Transform(newObjectName);
					target.addChild(*t);
				}
			}
			else {
				ImGui::Text("Name exists");
			}
			ImGui::EndMenu();
		}

		// Camera
		if (ImGui::BeginMenu("Camera")) {
			if (ImGui::BeginMenu("Camera")) {
				ImGui::InputText("Name", &newObjectName);
				if (Brane::find(typeid(Object).hash_code(), newObjectName) == NULL) {
					if (ImGui::Button("Create", { -1, 36 })) {
						::Transform* t = new ::Transform(newObjectName);
						target.addChild(*t);
					}
				}
				else {
					ImGui::Text("Name exists");
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("PostProcessingCamera##PostProcessingCameraMenu")) {
				ImGui::InputText("Name", &newObjectName);
				if (Brane::find(typeid(Object).hash_code(), newObjectName) == NULL) {
					if (ImGui::Button("Create", { -1, 36 })) {
						PostProcessingCamera* t = new PostProcessingCamera(newObjectName);
						target.addChild(*t);
					}
				}
				else {
					ImGui::Text("Name exists");
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		// SpringArm
		/*if (ImGui::BeginMenu("SpringArm")) {
			ImGui::InputText("Name", &newObjectName);
			ImGui::DragFloat("Radius", &springArmRadius);
			ImGui::DragFloat("SpringLength", &springArmLength);
			if (Brane::find(typeid(Object).hash_code(), newObjectName) == NULL) {
				if (ImGui::Button("Create", { -1, 36 })) {
					SpringArm* t = new SpringArm(newObjectName);
					t->setRadius(springArmRadius);
					t->setSpringLength(springArmLength);
					target.addChild(*t);
				}
			}
			else {
				ImGui::Text("Name exists");
			}
			ImGui::EndMenu();
		}*/

		// Character
		/*if (ImGui::BeginMenu("Character")) {
			ImGui::InputText("Name", &newObjectName);
			ImGui::DragFloat("Radius", &capsuleRadius);
			ImGui::DragFloat("HalfLength", &capsuleHalfLength);
			if (Brane::find(typeid(Object).hash_code(), newObjectName) == NULL) {
				if (ImGui::Button("Create", { -1, 36 })) {
					Character* t = new Character(Capsule(capsuleRadius, capsuleHalfLength), newObjectName);
					target.addChild(*t);
				}
			}
			else {
				ImGui::Text("Name exists");
			}
			ImGui::EndMenu();
		}*/

		// GrassActor
		if (ImGui::BeginMenu("GrassActor")) {
			ImGui::InputText("Name", &newObjectName);
			if (Brane::find(typeid(Object).hash_code(), newObjectName) == NULL) {
				meshCombo();
				materialCombo();
				if (ImGui::DragFloat("Density", &grassDensity)) {
					grassDensity = max(0, grassDensity);
				}
				if (ImGui::DragFloat2("Boundry", grassBound.data())) {
					grassBound.x() = max(0, grassBound.x());
					grassBound.y() = max(0, grassBound.y());
				}
				if (selectedMesh != NULL && selectedMaterial != NULL && ImGui::Button("Create", { -1, 36 })) {
					GrassMeshActor* t = new GrassMeshActor(*selectedMesh, *selectedMaterial, newObjectName);
					t->set(grassDensity, grassBound);
					target.addChild(*t);
				}
			}
			else {
				ImGui::Text("Name exists");
			}
			ImGui::EndMenu();
		}

		ImGui::EndMenu();
	}
	ImGui::PopID();
	if (obj != NULL) {
		if (ImGui::MenuItem("Delete"))
			target.destroy();
		if (ImGui::MenuItem("DeleteAll"))
			target.destroy(true);
	}
	const char* serStr = "Save Recursively";
	if (obj == NULL || isClassOf<World>(obj))
		serStr = "Save World";
	else if (ImGui::MenuItem("Save")) {
		thread td = thread([](Object* tar) {
			FileDlgDesc desc;
			desc.title = "asset";
			desc.filter = "asset(*.asset)\0*.asset\0";
			desc.initDir = "Content";
			desc.save = true;
			desc.defFileExt = "asset";
			if (openFileDlg(desc)) {
				SerializationInfo info;
				info.add("children");
				if (!tar->serialize(info)) {
					MessageBox(NULL, "Serialize failed", "Error", MB_OK);
					return;
				}
				ofstream f = ofstream(desc.filePath);
				SerializationInfoWriter writer = SerializationInfoWriter(f);
				writer.write(info);
				f.close();
				MessageBox(NULL, "Complete", "Info", MB_OK);
			}
		}, &target);
		td.detach();
	}
	if (ImGui::MenuItem(serStr)) {
		thread td = thread([](Object* tar) {
			FileDlgDesc desc;
			desc.title = "asset";
			desc.filter = "asset(*.asset)\0*.asset\0";
			desc.initDir = "Content";
			desc.save = true;
			desc.defFileExt = "asset";
			if (openFileDlg(desc)) {
				SerializationInfo info;
				if (!tar->serialize(info)) {
					MessageBox(NULL, "Serialize failed", "Error", MB_OK);
					return;
				}
				ofstream f = ofstream(desc.filePath);
				SerializationInfoWriter writer = SerializationInfoWriter(f);
				writer.write(info);
				f.close();
				MessageBox(NULL, "Complete", "Info", MB_OK);
			}
		}, &target);
		td.detach();
	}
}
