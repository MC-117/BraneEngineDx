#include "MatBranchModifier.h"
#include "../Asset.h"
#include "../Engine.h"
#include "GUIUtility.h"
#include "../../ThirdParty/ImGui/imgui_stdlib.h"
#include "../Importer/MaterialImporter.h"

MatBranchModifier::MatBranchModifier(string name, bool defaultShow)
	: UIWindow(*Engine::getCurrentWorld(), name, defaultShow)
{
}

void MatBranchModifier::onWindowGUI(GUIRenderInfo& info)
{
	Object* obj = targetObject;
	ImGui::ObjectCombo("Object", obj, Engine::getCurrentWorld(), objectFilterName);
	targetObject = obj;

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
				MaterialLoader::saveMaterialInstance(path, *mat);
			}
		}
	}
}
