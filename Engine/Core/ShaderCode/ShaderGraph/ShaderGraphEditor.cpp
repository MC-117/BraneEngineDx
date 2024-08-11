#include "ShaderGraphEditor.h"

#include "HLSLGeneration.h"
#include "../../Attributes/TagAttribute.h"
#include "../../CodeGeneration/ClangGeneration.h"
#include "../../Importer/MaterialImporter.h"
#include "Core/ShaderCode/ShaderGraphCompiler.h"

RegistEditor(ShaderGraph);

ShaderGraphEditor::ShaderGraphEditor()
{
	showFlags = Show_Details | Show_CreateVariables | Show_CreateSubgraphs | Show_CreateSubgraphs;
}

void ShaderGraphEditor::setInspectedObject(void* object)
{
    shaderGraph = dynamic_cast<ShaderGraph*>((Base*)object);
    GraphEditor::setInspectedObject(shaderGraph);
}

void ShaderGraphEditor::onDetailsGUI(EditorInfo& info)
{
	if (info.previewer) {
		info.previewer->setMaterial(shaderGraph->getBaseMaterial());
		
		float width = ImGui::GetWindowContentRegionWidth();
		info.previewer->onGUI("ShaderGraphScene", width, width);
	}
	GraphEditor::onDetailsGUI(info);
}

void ShaderGraphEditor::getPinMenuTags(vector<Name>& tags)
{
	getNodeMenuTags(tags);
}

void ShaderGraphEditor::getVariableMenuTags(vector<Name>& tags)
{
	getNodeMenuTags(tags);
	tags.emplace_back("Graph"_N);
}

void ShaderGraphEditor::getNodeMenuTags(vector<Name>& tags)
{
	TagManager::get().fuzzyFindTag("Graph.Shader", tags);
	TagManager::get().fuzzyFindTag("Graph.Math", tags);
}

void ShaderGraphEditor::onGenerateCode(ScriptBase& script)
{
	string code;
	switch (genCodeType) {
	case Clang:
		{
			ClangWriter writer;
			ShaderGraph::generateCode(writer, *shaderGraph);
			writer.getString(code);
			break;
		}
	case HLSL:
		{
			HLSLWriter writer;
			ShaderGraph::generateCode(writer, *shaderGraph);
			writer.getString(code);
			break;
		}
	}

	tempScript->setSourceCode(code);
}

void ShaderGraphEditor::onCompileGUI(EditorInfo& info)
{
	if (ImGui::Button("Compile", { -1, 36 })) {
		const Name shaderName = shaderGraph->getName();
		if (shaderName.isNone()) {
			MessageBoxA(NULL, "Name is none", "Error", MB_OK);
		}
		else {
			Name path = shaderGraph->getPath();
			if (path.isNone()) {
				path = "Memory/ShaderGraph/"s + shaderName.c_str() + ".gmat";
				shaderGraph->setPath(path);
			}
			if (ShaderGraphCompiler::compile(*shaderGraph)) {
				bool newMaterial = shaderGraph->getBaseMaterial();
				Material* material = MaterialLoader::loadMaterialGraph(*shaderGraph);
				if (newMaterial) {
					Asset* asset = new Asset(&MaterialAssetInfo::assetInfo, shaderName.c_str(), path.c_str());
					asset->asset[0] = material;
					AssetManager::registAsset(*asset);
				}
			}
		}
	}
}

void ShaderGraphEditor::onCustomGUI(EditorInfo& info)
{
	GraphEditor::onCustomGUI(info);
	onCompileGUI(info);
}
