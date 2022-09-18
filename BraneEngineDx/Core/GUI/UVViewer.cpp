#include "UVViewer.h"
#include "../Engine.h"
#include "../Asset.h"
#include "../Editor/Editor.h"
#include "../GUI/GUIUtility.h"

UVViewer::UVViewer(string name, bool defaultShow)
	: UIWindow(*Engine::getCurrentWorld(), name, defaultShow)
	, camera(renderTarget)
{
	camera.clearColor = { 0, 0, 0, 0 };
	Material* baseMaterial = getAssetByPath<Material>("Engine/Shaders/UVViewer.mat");
	if (baseMaterial) {
		uvMaterial = &baseMaterial->instantiate();
	}
	renderTarget.addTexture("uvTex", uvTexture);
}

void UVViewer::setTargetMeshPart(Mesh* mesh)
{
	selectMesh = mesh;
	partIndex = 0;
	selectMeshPart = NULL;
	if (mesh)
		selectMeshPart = &mesh->meshParts[0];
}

void UVViewer::onRenderWindow(GUIRenderInfo& info)
{
	if (ImGui::AssetCombo("Mesh", meshAsset, "Mesh;SkeletonMesh")) {
		setTargetMeshPart((Mesh*)meshAsset->load());
	}
	ImGui::SameLine();
	if (ImGui::ArrowButton("MeshAssignBT", ImGuiDir_Left)) {
		meshAsset = EditorManager::getSelectedAsset();
		if (meshAsset->assetInfo.type == "Mesh" || meshAsset->assetInfo.type == "SkeletonMesh")
			setTargetMeshPart((Mesh*)meshAsset->load());
	}

	if (ImGui::AssetCombo("Texture", textureAsset, "Texture2D")) {
		selectTexture = (Texture2D*)textureAsset->load();
	}
	ImGui::SameLine();
	if (ImGui::ArrowButton("TexAssignBT", ImGuiDir_Left)) {
		textureAsset = EditorManager::getSelectedAsset();
		if (textureAsset->assetInfo.type == "Texture2D")
			selectTexture = (Texture2D*)textureAsset->load();
	}

	if (selectMesh == NULL)
		return;
	
	string meshName = to_string(partIndex) + ": " + selectMesh->partNames[partIndex];
	if (ImGui::BeginCombo("MeshPart", meshName.c_str())) {
		for (int i = 0; i < selectMesh->meshParts.size(); i++) {
			meshName = to_string(i) + ": " + selectMesh->partNames[i];
			if (ImGui::Selectable(meshName.c_str(), partIndex == i)) {
				selectMeshPart = &selectMesh->meshParts[i];
				partIndex = i;
			}
		}
		ImGui::EndCombo();
	}

	if (ImGui::Button("SaveUVToDisk")) {
		FileDlgDesc desc;
		desc.title = "Capture";
		desc.filter = "png(*.png)|*.png|jpg(*.jpg)|*.jpg|tga(*.tga)|*.tga|bmp(*.bmp)|*.bmp";
		desc.initDir = "";
		desc.defFileExt = "png";
		desc.save = true;

		if (openFileDlg(desc)) {
			uvTexture.save(desc.filePath);
		}
	}

	if (selectMesh == NULL || selectMeshPart == NULL || uvMaterial == NULL)
		return;

	float height = ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y - ImGui::GetCursorPosY();
	float width = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;

	float tw = uvTexture.getWidth();
	float th = uvTexture.getHeight();
	float aspect = tw / th;
	if (aspect < width / height) {
		tw = height * aspect;
		th = height;
	}
	else {
		tw = width;
		th = width / aspect;
	}
	float hpw = (width - tw) / 2.0f;
	float hph = (height - th) / 2.0f;

	ImGui::SameLine();
	ImGui::Text("%d, %d", (int)tw, (int)th);

	if (tw * th == 0)
		return;

	camera.setSize({ (int)tw, (int)th });

	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0, 0 });
	auto list = ImGui::GetWindowDrawList();
	float padding = 0;
	ImVec2 pos = ImGui::GetCursorScreenPos();

	if (selectTexture) {
		selectTexture->bind();
		unsigned long long id = selectTexture->getTextureID();
		list->AddImage((ImTextureID)id, { pos.x + padding + hpw, pos.y + padding + hph },
			{ pos.x + padding + hpw + tw, pos.y + padding + hph + th });
	}

	uvTexture.bind();
	unsigned long long id = uvTexture.getTextureID();
	if (id != 0)
		list->AddImage((ImTextureID)id, { pos.x + padding + hpw, pos.y + padding + hph },
			{ pos.x + padding + hpw + tw, pos.y + padding + hph + th });
	ImGui::PopStyleVar(2);

	MeshRenderCommand command;
	command.camera = &camera;
	command.material = uvMaterial;
	command.mesh = selectMeshPart;
	command.isNonTransformIndex = true;

	info.renderCommandList->setRenderCommand(command);
}