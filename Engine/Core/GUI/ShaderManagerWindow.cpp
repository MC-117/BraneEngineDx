#include "ShaderManagerWindow.h"
#include "../Engine.h"
#include "../Utility/RenderUtility.h"

ShaderManagerWindow::ShaderManagerWindow(string name, bool defaultShow)
	: UIWindow(*Engine::getCurrentWorld(), name, defaultShow)
{
}

void ShaderManagerWindow::onWindowGUI(GUIRenderInfo& info)
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
			string featureName = getShaderFeatureNames(b->first);
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
	string featureName = getShaderFeatureNames(stage.getShaderFeature());
	ImGui::Text("Stage: %s", ShaderStage::enumShaderStageType(stage.getShaderStageType()));
	ImGui::Text("Feature: %s", featureName.c_str());
	ImGui::Text("ShaderID: %lld", stage.getShaderID());
	const unordered_map<ShaderPropertyName, ShaderProperty>& properties = stage.getProperties();
	if (properties.empty())
		return;
	ImGui::Columns(5, "Properties");
	ImGui::Separator();
	ImGui::Text("Name"); ImGui::NextColumn();
	ImGui::Text("Type"); ImGui::NextColumn();
	ImGui::Text("Offset"); ImGui::NextColumn();
	ImGui::Text("Size"); ImGui::NextColumn();
	ImGui::Text("Meta"); ImGui::NextColumn();
	for (auto& item : properties) {
		onShaderPropertyGUI(item.second);
	}
	ImGui::Columns(1);
	ImGui::Separator();
}

void ShaderManagerWindow::onShaderPropertyGUI(const ShaderProperty& prop)
{
	ImGui::Separator();
	ImGui::Text(prop.name.c_str()); ImGui::NextColumn();
	ImGui::Text(getShaderPropertyTypeName(prop.type)); ImGui::NextColumn();
	ImGui::Text("%d", prop.offset); ImGui::NextColumn();
	ImGui::Text("%d", prop.size); ImGui::NextColumn();
	ImGui::Text("%d", prop.meta); ImGui::NextColumn();
}
