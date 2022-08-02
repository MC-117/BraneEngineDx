#include "ShaderManagerWindow.h"
#include "../Engine.h"

ShaderManagerWindow::ShaderManagerWindow(string name, bool defaultShow)
	: UIWindow(*Engine::getCurrentWorld(), name, defaultShow)
{
}

void ShaderManagerWindow::onRenderWindow(GUIRenderInfo& info)
{
	ShaderManager& manager = ShaderManager::getInstance();

	if (ImGui::CollapsingHeader("ShaderFiles")) {
		int i = 0;
		for (auto b = manager.shaderFiles.begin(), e = manager.shaderFiles.end(); b != e; b++, i++) {
			ImGui::PushID(i);
			if (ImGui::TreeNode(b->first.c_str())) {
				onShaderFileGUI(*b->second);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}

	if (ImGui::CollapsingHeader("ShaderAdapters")) {
		int i = 0;
		for (auto b = manager.shaderAdapters.begin(), e = manager.shaderAdapters.end(); b != e; b++, i++) {
			if (ImGui::TreeNode(to_string(i).c_str())) {
				onShaderAdapterGUI(**b);
				ImGui::TreePop();
			}
		}
	}
}

string ShaderManagerWindow::getShaderFeatureName(Enum<ShaderFeature> feature)
{
	if ((int)feature == 0) {
		return "Default";
	}
	else if ((int)feature == 1) {
		return "Custom";
	}
	string featureName;
	const int numDefault = 6;
	static const char* featureDefaultName[numDefault] = {
		"Deferred", "Postprocess", "Skeleton", "Morph", "Particle", "Modifier"
	};
	const int numCustom = 7;
	static const char* featureCustomName[numCustom] = {
		"Custom_1", "Custom_2", "Custom_3", "Custom_4", "Custom_5", "Custom_6", "Custom_7"
	};
	bool custom = feature.has(ShaderFeature::Shader_Custom);
	if (custom) {
		for (int i = 0; i < numCustom; i++) {
			if (feature.has(1 << (i + 1)))
				featureName += featureCustomName[i] + string(" | ");
		}
	}
	else {
		for (int i = 0; i < numDefault; i++) {
			if (feature.has(1 << (i + 1)))
				featureName += featureDefaultName[i] + string(" | ");
		}
	}
	if (!featureName.empty())
		featureName = featureName.substr(0, featureName.size() - 3);
	return featureName;
}

void ShaderManagerWindow::onShaderFileGUI(const ShaderFile& file)
{
	ImGui::Text("Path: %s", file.path.c_str());

	int i = 0;
	if (ImGui::TreeNode("Adapters")) {
		for (auto b = file.adapters.begin(), e = file.adapters.end(); b != e; b++, i++) {
			if (ImGui::TreeNode(to_string(i).c_str())) {
				onShaderAdapterGUI(**b);
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}

	i = 0;
	if (ImGui::TreeNode("IncludeFiles")) {
		for (auto b = file.includeFiles.begin(), e = file.includeFiles.end(); b != e; b++, i++) {
			ImGui::PushID(i);
			if (ImGui::TreeNode((*b)->path.c_str())) {
				onShaderFileGUI(**b);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	}
}

void ShaderManagerWindow::onShaderAdapterGUI(const ShaderAdapter& adapter)
{
	ImGui::Text("Stage: %s", ShaderStage::enumShaderStageType(adapter.stageType));
	ImGui::Text("Name: %s", adapter.name.c_str());
	ImGui::Text("Path: %s", adapter.path.c_str());
	string featureStr;
	int i = 0;

	if (ImGui::TreeNode("Stages")) {
		for (auto b = adapter.shaderStageVersions.begin(),
			e = adapter.shaderStageVersions.end(); b != e; b++, i++) {
			ImGui::PushID(i);
			string featureName = getShaderFeatureName(b->first);
			if (ImGui::TreeNode(featureName.c_str())) {
				onShaderStageGUI(*b->second);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	}
}

void ShaderManagerWindow::onShaderStageGUI(const ShaderStage& stage)
{
	string featureName = getShaderFeatureName(stage.getShaderFeature());
	ImGui::Text("Stage: %s", ShaderStage::enumShaderStageType(stage.getShaderStageType()));
	ImGui::Text("Feature: %s", featureName.c_str());
	ImGui::Text("ShaderID: %lld", stage.getShaderID());
}
