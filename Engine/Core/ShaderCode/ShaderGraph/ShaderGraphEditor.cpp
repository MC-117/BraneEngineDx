#include "ShaderGraphEditor.h"
#include "../../Attributes/TagAttribute.h"

RegistEditor(ShaderGraph);

void ShaderGraphEditor::setInspectedObject(void* object)
{
    shaderGraph = dynamic_cast<ShaderGraph*>((Base*)object);
    GraphEditor::setInspectedObject(shaderGraph);
}

void ShaderGraphEditor::getPinMenuTags(vector<Name>& tags)
{
	getNodeMenuTags(tags);
}

void ShaderGraphEditor::getVariableMenuTags(vector<Name>& tags)
{
	getNodeMenuTags(tags);
}

void ShaderGraphEditor::getNodeMenuTags(vector<Name>& tags)
{
	TagManager::get().fuzzyFindTag("Graph.Shader", tags);
	TagManager::get().fuzzyFindTag("Graph.Math", tags);
}
