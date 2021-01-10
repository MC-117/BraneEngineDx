#include "MatBranchModifier.h"
#include "../Asset.h"
#include "../Engine.h"
#include "../../ThirdParty/ImGui/imgui_stdlib.h"

MatBranchModifier::MatBranchModifier(string name, bool defaultShow)
	: UIWindow(*Engine::getCurrentWorld(), name, defaultShow)
{
}

void MatBranchModifier::onRenderWindow(GUIRenderInfo& info)
{
	ImGui::InputText("Name", &objectName);
	Object* obj = (Object*)Brane::find(typeid(Object).hash_code(), objectName);
	meshRender = NULL;
	if (obj != NULL) {
		meshRender = dynamic_cast<MeshRender*>(obj->getRender());
	}
	if (meshRender == NULL)
		return;

	bool colorE = false, scalarE = false;
	ImGui::InputText("Color Name", &colorName);
	if (!colorName.empty())
		colorE = ImGui::ColorEdit4(colorName.c_str(), (float*)&color, ImGuiColorEditFlags_HDR);
	ImGui::InputText("Scalar Name", &scalarName);
	if (!scalarName.empty())
		scalarE = ImGui::DragFloat(scalarName.c_str(), &scalar);
	for (int i = 0; i < meshRender->materials.size(); i++) {
		Material* mat = meshRender->materials[i];
		if (mat == NULL)
			continue;
		if (colorE)
			mat->setColor(colorName, color);
		if (scalarE)
			mat->setScalar(scalarName, scalar);
	}
	if (ImGui::Button("Save All")) {
		for (int i = 0; i < meshRender->materials.size(); i++) {
			Material* mat = meshRender->materials[i];
			if (mat == NULL)
				continue;
			string path = AssetInfo::getPath(mat);
			if (!path.empty()) {
				Material::MaterialLoader::saveMaterialInstance(path, *mat);
			}
		}
	}
}
