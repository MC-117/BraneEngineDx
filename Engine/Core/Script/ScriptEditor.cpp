#include "ScriptEditor.h"
#include "../Engine.h"
#include "Core/Utility/EngineUtility.h"

RegistEditor(ScriptBase);

void setLanguageDefinitionByExtension(TextEditor& textEditor, string ext)
{
	transform(ext.begin(), ext.end(), ext.begin(),
	[](unsigned char c){ return tolower(c); });
	if (ext == "py") {
		textEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::Python());
	}
	else if (ext == "c") {
		textEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::C());
	}
	else if (ext == "mat") {
		textEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::HLSL());
	}
	else if (ext == "cpp" || ext == "h" || ext == "hpp" || ext == "mm" || ext == "cxx" || ext == "hxx") {
		textEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
	}
	else {
		textEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::General());
	}
}

void ScriptBaseEditor::setInspectedObject(void* object)
{
	scriptBase = dynamic_cast<ScriptBase*>((Base*)object);
	BaseEditor::setInspectedObject(scriptBase);

	if (scriptBase && textEditor.GetLanguageDefinition().IsNone()) {
		string ext = getExtension(scriptBase->getName());
		setLanguageDefinitionByExtension(textEditor, ext);
	}
}

TextEditor& ScriptBaseEditor::getTextEditor()
{
	return textEditor;
}

void ScriptBaseEditor::syncCode()
{
	if (scriptBase)
		textEditor.SetText(scriptBase->getSourceCode());
}

bool ScriptBaseEditor::saveCode()
{
	if (scriptBase != NULL) {
		if (textEditor.CanUndo() && savedUndoIndex != textEditor.GetUndoIndex()) {
			scriptBase->setSourceCode(textEditor.GetText());
			if (scriptBase->saveSourceCode()) {
				savedUndoIndex = textEditor.GetUndoIndex();
				if (!scriptBase->refresh())
					return false;
				onApplyCode();
			}
			else
				return false;
		}
		return true;
	}
	return false;
}

bool ScriptBaseEditor::applyCode()
{
	if (scriptBase != NULL) {
		if (textEditor.CanUndo()) {
			scriptBase->setSourceCode(textEditor.GetText());
			if (!scriptBase->refresh())
				return false;
			onApplyCode();
		}
		return true;
	}
	return false;
}

void ScriptBaseEditor::onApplyCode()
{
	
}

void ScriptBaseEditor::onMenu(EditorInfo& info)
{
}

void ScriptBaseEditor::onStatusGUI(EditorInfo& info)
{
	auto cpos = textEditor.GetCursorPosition();
	string codePath = scriptBase->getCodePath();
	ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, textEditor.GetTotalLines(),
		textEditor.IsOverwrite() ? "Ovr" : "Ins",
		textEditor.CanUndo() ? "*" : " ",
		textEditor.GetLanguageDefinition().mName.c_str(), codePath.empty() ? "None" : codePath.c_str());
}

void ScriptBaseEditor::onTextEditor(EditorInfo& info)
{
	textEditor.Render("##ScriptEditorCode", { -1, -1 });
}

void ScriptBaseEditor::onGUI(EditorInfo& info)
{
	if (scriptBase == NULL)
		return;

	onTextEditor(info);
}
