#include "MeshRenderEditor.h"
#include "../SkeletonMesh.h"
#include <fstream>

#include "../GUI/GUIUtility.h"

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

	Mesh* mesh = meshRender->getMesh();
	SkeletonMesh* skeletonMesh = dynamic_cast<SkeletonMesh*>(mesh);

	ImGui::Text("Vertex Count: %d", mesh->vertCount);
	ImGui::Text("Face Count: %d", mesh->faceCount);
	if (ImGui::TreeNode("MeshParts")) {
		for (auto b = mesh->meshPartNameMap.begin(), e = mesh->meshPartNameMap.end(); b != e; b++) {
			if (ImGui::TreeNode(b._Ptr, b->first.c_str())) {
				bool enable = meshRender->collection.getPartEnable(b->second);
				if (ImGui::Checkbox("Enable", &enable))
					meshRender->collection.setPartEnable(b->second, enable);
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
							mesh->writeObjStream(f, index);
							f.close();
						}
					}
				}

				ImGui::Text("Vertex Count: %d", mesh->meshParts[b->second].vertexCount);
				ImGui::Text("Face Count: %d", mesh->meshParts[b->second].elementCount);
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
	ImGui::OutlineMeshMaterialGUI(meshRender->outlineCollection);
}

void MeshRenderEditor::onMaterialsGUI(EditorInfo& info)
{
	ImGui::MeshMaterialGUI(info, meshRender->collection);
}
