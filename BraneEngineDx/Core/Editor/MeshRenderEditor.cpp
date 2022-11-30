#include "MeshRenderEditor.h"
#include "../SkeletonMesh.h"
#include <fstream>

RegistEditor(MeshRender);

void MeshRenderEditor::setInspectedObject(void* object)
{
	meshRender = dynamic_cast<MeshRender*>((Render*)object);
	render = meshRender;
}

void MeshRenderEditor::onShapeGUI(EditorInfo& info)
{
	RenderEditor::onShapeGUI(info);

	ImGui::Separator();

	SkeletonMesh* skeletonMesh = dynamic_cast<SkeletonMesh*>(meshRender->mesh);

	ImGui::Text("Vertex Count: %d", meshRender->mesh->vertCount);
	ImGui::Text("Face Count: %d", meshRender->mesh->faceCount);
	if (ImGui::TreeNode("MeshParts")) {
		for (auto b = meshRender->mesh->meshPartNameMap.begin(), e = meshRender->mesh->meshPartNameMap.end(); b != e; b++) {
			if (ImGui::TreeNode(b._Ptr, b->first.c_str())) {
				bool enable = meshRender->getPartEnable(b->second);
				if (ImGui::Checkbox("Enable", &enable))
					meshRender->setPartEnable(b->second, enable);
				if (ImGui::Button("Export OBJ")) {
					FileDlgDesc desc;
					desc.title = "obj";
					desc.filter = "obj(*.obj)|*.obj";
					desc.save = true;
					desc.defFileExt = "obj";
					desc.addToRecent = false;

					if (openFileDlg(desc)) {
						ofstream f(desc.filePath);
						if (!f.fail()) {
							vector<int> index = { (int)b->second };
							meshRender->mesh->writeObjStream(f, index);
							f.close();
						}
					}
				}

				ImGui::Text("Vertex Count: %d", meshRender->mesh->meshParts[b->second].vertexCount);
				ImGui::Text("Face Count: %d", meshRender->mesh->meshParts[b->second].elementCount);
				if (skeletonMesh != NULL && skeletonMesh->partHasMorph[b->second])
					ImGui::TextColored({ 1.f, 1.f, 0.f, 1.f }, "Has MorphTarget");
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
}

void MeshRenderEditor::onOutlineGUI(EditorInfo& info)
{
	Mesh* mesh = meshRender->mesh;
	if (mesh == NULL)
		return;
	if (ImGui::Button("Enable All", { -1, 36 })) {
		for (int i = 0; i < meshRender->outlineEnable.size(); i++) {
			meshRender->outlineEnable[i] = true;
			Material*& outlineMaterial = meshRender->outlineMaterials[i];
			if (outlineMaterial == NULL) {
				outlineMaterial = getAssetByPath<Material>("Engine/Shaders/Outline.mat");
				if (outlineMaterial != NULL)
					outlineMaterial = &outlineMaterial->instantiate();
			}
		}
	}
	if (ImGui::Button("Disable All", { -1, 36 })) {
		for (int i = 0; i < meshRender->outlineEnable.size(); i++) {
			meshRender->outlineEnable[i] = false;
		}
	}
	int i = 0;
	for (auto mb = mesh->meshPartNameMap.begin(), me = mesh->meshPartNameMap.end(); mb != me; mb++, i++) {
		ImGui::PushID(i);
		Material*& outlineMaterial = meshRender->outlineMaterials[mb->second];
		bool enable = meshRender->outlineEnable[mb->second];
		if (ImGui::Checkbox("##EnableOutlineMaterial", &enable)) {
			meshRender->outlineEnable[mb->second] = enable;
		}
		ImGui::SameLine();
		if (ImGui::TreeNode((mb->first + " Outline").c_str())) {
			if (outlineMaterial == NULL && ImGui::Button("Set Outline Material")) {
				outlineMaterial = getAssetByPath<Material>("Engine/Shaders/Outline.mat");
				if (outlineMaterial != NULL)
					outlineMaterial = &outlineMaterial->instantiate();
			}
			if (outlineMaterial != NULL) {
				for (auto b = outlineMaterial->getColorField().begin(), e = outlineMaterial->getColorField().end(); b != e; b++) {
					Color color = b->second.val;
					if (ImGui::ColorEdit4(b->first.c_str(), (float*)&color))
						b->second.val = color;
				}
				for (auto b = outlineMaterial->getScalarField().begin(), e = outlineMaterial->getScalarField().end(); b != e; b++) {
					float val = b->second.val;
					if (ImGui::DragFloat(b->first.c_str(), &val, 0.01))
						b->second.val = val;
				}
				for (auto b = outlineMaterial->getTextureField().begin(), e = outlineMaterial->getTextureField().end(); b != e; b++) {
					unsigned long long id = b->second.val->getTextureID();
					if (id == 0)
						id = Texture2D::blackRGBDefaultTex.getTextureID();
					ImGui::Image((ImTextureID)id, { 64, 64 }, { 0, 1 }, { 1, 0 });
					ImGui::SameLine();
					ImGui::Text(b->first.c_str());
				}
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
}

void MeshRenderEditor::onMaterialsGUI(EditorInfo& info)
{
	Mesh* mesh = meshRender->mesh;
	if (mesh == NULL)
		return;
	int i = 0;
	for (auto mb = mesh->meshPartNameMap.begin(), me = mesh->meshPartNameMap.end(); mb != me; mb++, i++) {
		ImGui::PushID(i);
		bool enable = meshRender->getPartEnable(mb->second);
		if (ImGui::Checkbox("##EnableMaterial", &enable)) {
			meshRender->setPartEnable(mb->second, enable);
		}
		ImGui::SameLine();
		if (ImGui::TreeNode(mb->first.c_str())) {

			if (ImGui::Button("Assign Selected Material", { -1, 36 })) {
				Asset* assignAsset = EditorManager::getSelectedAsset();
				if (assignAsset != NULL && assignAsset->assetInfo.type == "Material") {
					meshRender->materials[mb->second] = (Material*)assignAsset->load();
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
							meshRender->materials[mb->second] = &((Material*)_b->second->load())->instantiate();
						else
							meshRender->materials[mb->second] = (Material*)_b->second->load();
					}
				}
				ImGui::EndPopup();
			}

			Material* material = meshRender->materials[mb->second];
			if (material == NULL) {
				ImGui::Text("No Material");
			}
			else {
				Editor* editor = EditorManager::getEditor("Material", material);
				editor->onGUI(info);
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
}
