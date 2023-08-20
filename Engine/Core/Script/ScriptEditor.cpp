#include "ScriptEditor.h"
#include "../Engine.h"

ScriptEditor::ScriptEditor(string name, bool defaultShow)
	: UIWindow(*Engine::getCurrentWorld(), name, defaultShow)
{
	setStyle(ImGuiWindowFlags_MenuBar);
}

ScriptEditor::~ScriptEditor()
{
	for (auto b = tempScripts.begin(), e = tempScripts.end(); b != e; b++)
		b->second.release();
}

void ScriptEditor::OpenScript(GUI& gui, PythonScript& script)
{
	ScriptEditor* win = dynamic_cast<ScriptEditor*>(gui.getUIControl("ScriptEditor"));
	if (win == NULL) {
		win = new ScriptEditor();
		gui.addUIControl(*win);
	}
	win->show = true;
	win->setScript(script);
}

void ScriptEditor::setScript(PythonScript & script)
{
	auto iter = tempScripts.find(&script);
	if (iter == tempScripts.end()) {
		currentData = &tempScripts.insert(make_pair(&script, CodeTempData(script))).first->second;
	}
	else
		currentData = &iter->second;
	currentData->applyCode();
}

PythonScript * ScriptEditor::getScript()
{
	return currentData == NULL ? NULL : currentData->script;
}

void ScriptEditor::onRenderWindow(GUIRenderInfo & info)
{
	if (currentData == NULL)
		return;
	TextEditor& textEditor = *currentData->textEditor;
	PythonScript& script = *currentData->script;
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save"))
			{
				currentData->saveCode();
			}
			if (ImGui::MenuItem("Apply")) {
				currentData->applyCode();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			bool ro = textEditor.IsReadOnly();
			if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
				textEditor.SetReadOnly(ro);
			ImGui::Separator();

			if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && textEditor.CanUndo()))
				textEditor.Undo();
			if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && textEditor.CanRedo()))
				textEditor.Redo();

			ImGui::Separator();

			if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, textEditor.HasSelection()))
				textEditor.Copy();
			if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && textEditor.HasSelection()))
				textEditor.Cut();
			if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && textEditor.HasSelection()))
				textEditor.Delete();
			if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
				textEditor.Paste();

			ImGui::Separator();

			if (ImGui::MenuItem("Select all", nullptr, nullptr))
				textEditor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(textEditor.GetTotalLines(), 0));

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Dark palette"))
				textEditor.SetPalette(TextEditor::GetDarkPalette());
			if (ImGui::MenuItem("Light palette"))
				textEditor.SetPalette(TextEditor::GetLightPalette());
			if (ImGui::MenuItem("Retro blue palette"))
				textEditor.SetPalette(TextEditor::GetRetroBluePalette());
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Functions")) {
			for (int i = 0; i < currentData->functions.size(); i++) {
				PythonFunctionObject& function = currentData->functions[i];
				ImGui::PushID(i);
				if (ImGui::MenuItem(function.name.c_str())) {
					function.call();
				}
				ImGui::PopID();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	auto cpos = textEditor.GetCursorPosition();
	string codePath = script.getCodePath();
	ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, textEditor.GetTotalLines(),
		textEditor.IsOverwrite() ? "Ovr" : "Ins",
		textEditor.CanUndo() ? "*" : " ",
		textEditor.GetLanguageDefinition().mName.c_str(), codePath.empty() ? "None" : codePath.c_str());
	switch (script.getType())
	{
	case PythonScript::Wrapper:
		textEditor.Render("##ScriptEditorCode", { -1, -1 });
		break;
	case PythonScript::Behavior:
		textEditor.Render("##ScriptEditorCode", { -122, -1 });
		ImGui::SameLine();
		ImGui::BeginChild("##ScriptEditorFunctions", { 120,-1 });
		for (int i = 0; i < currentData->functions.size(); i++) {
			PythonFunctionObject& function = currentData->functions[i];
			ImGui::PushID(i);
			if (ImGui::Button(function.name.c_str(), { -1, 36 })) {
				function.call();
			}
			ImGui::PopID();
		}
		ImGui::EndChild();
		break;
	default:
		break;
	}
}

ScriptEditor::CodeTempData::CodeTempData(PythonScript & script) : script(&script)
{
	textEditor = new TextEditor();
	textEditor->SetText(script.getSourceCode());
	if (script.getType() == PythonScript::Behavior) {
		if (runtimeObject == NULL) {
			runtimeObject = new PythonRuntimeObject();
			runtimeObject->setScript(script);
		}
		functions.clear();
		runtimeObject->getCallables(functions);
	}
}

bool ScriptEditor::CodeTempData::saveCode()
{
	if (script != NULL) {
		if (textEditor->CanUndo()) {
			script->setSourceCode(textEditor->GetText());
			if (script->saveSourceCode()) {
				textEditor->ClearUndo();
				if (!script->refresh())
					return false;
				if (script->getType() == PythonScript::Behavior) {
					if (runtimeObject == NULL) {
						runtimeObject = new PythonRuntimeObject();
						runtimeObject->setScript(*script);
					}
					functions.clear();
					runtimeObject->getCallables(functions);
				}
			}
			else
				return false;
		}
		return true;
	}
	return false;
}

bool ScriptEditor::CodeTempData::applyCode()
{
	if (script != NULL) {
		if (textEditor->CanUndo()) {
			script->setSourceCode(textEditor->GetText());
			if (!script->refresh())
				return false;
			if (script->getType() == PythonScript::Behavior) {
				if (runtimeObject == NULL) {
					runtimeObject = new PythonRuntimeObject();
					runtimeObject->setScript(*script);
				}
				functions.clear();
				runtimeObject->getCallables(functions);
			}
		}
		return true;
	}
	return false;
}

void ScriptEditor::CodeTempData::release()
{
	if (runtimeObject != NULL) {
		Py_DECREF(runtimeObject);
		runtimeObject = NULL;
	}
	functions.clear();
	script = NULL;
	delete textEditor;
}
