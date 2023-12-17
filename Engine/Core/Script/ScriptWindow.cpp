#include "ScriptWindow.h"
#include <fstream>
#include "../Engine.h"

SerializeInstance(TempScript);

TempScript::TempScript(const string& name) : name(name)
{
}

bool TempScript::isValid() const
{
	return true;
}

bool TempScript::load(const std::string& file)
{
	ifstream f(file);
	if (f.fail())
		return false;
	stringstream buffer;
	buffer << f.rdbuf();
	f.close();
	code = buffer.str();
	return true;
}

const std::string& TempScript::getName() const
{
	return name;
}

const std::string& TempScript::getCodePath() const
{
	return "none";
}

const std::string& TempScript::getSourceCode() const
{
	return code;
}

bool TempScript::setSourceCode(const std::string& code)
{
	this->code = code;
	return true;
}

bool TempScript::refresh()
{
	return true;
}

bool TempScript::saveSourceCode()
{
	return false;
}

Serializable* TempScript::instantiate(const SerializationInfo& from)
{
	return new TempScript(from.name);
}

ScriptWindow::ScriptWindow(const string& name, bool defaultShow)
	: UIWindow(*Engine::getCurrentWorld(), name, defaultShow)
{
	setStyle(ImGuiWindowFlags_MenuBar);
}

void ScriptWindow::OpenScript(ScriptBase& script)
{
	ScriptWindow* win = dynamic_cast<ScriptWindow*>(GUI::get().getUIControl("ScriptWindow"));
	if (win == NULL) {
		win = new ScriptWindow();
		GUI::get().addUIControl(*win);
	}
	win->show = true;
	win->AddOrSelectScript(script);
}

TempScript& ScriptWindow::OpenTempScript(const string& name, string& text)
{
	TempScript& script = *new TempScript(name);
	script.setSourceCode(text);
	OpenScript(script);
	return script;
}

void ScriptWindow::AddScript(ScriptBase& script)
{
	bool found = false;
	for (auto& scriptRef : scripts) {
		if (scriptRef == &script) {
			found = true;
			break;
		}
	}
	if (!found)
		scripts.push_back(&script);
	if (ScriptBaseEditor* editor = dynamic_cast<ScriptBaseEditor*>(EditorManager::getEditor(script))) {
		editor->syncCode();
	}
}

void ScriptWindow::AddOrSelectScript(ScriptBase& script)
{
	AddScript(script);
	activeScript = &script;
}

void ScriptWindow::onWindowGUI(GUIRenderInfo & info)
{
	if (scripts.empty()) {
		return;
	}

	if (ImGui::BeginTabBar("ScriptTabBar")) {
		int closedIndex = -1;
		for (int i = 0; i < scripts.size(); i++) {
			ScriptBase* pScript = scripts[i];
			if (pScript == NULL)
				continue;
			ImGui::PushID(pScript);
			bool open = true;
			if (ImGui::BeginTabItem(pScript->getName().c_str(), &open),
				activeScript == pScript ? ImGuiTabItemFlags_SetSelected : 0) {
				activeScript = pScript;
				ImGui::EndTabItem();
			}
			if (!open) {
				closedIndex = i;
				if (activeScript == pScript) {
					if (scripts.size() <= 1)
						activeScript = NULL;
					else
						activeScript = scripts[clamp(i + 1, 0, (int)scripts.size() - 1)];
				}
			}
			ImGui::PopID();
		}
		ImGui::EndTabBar();
		if (closedIndex >= 0) {
			scripts.erase(scripts.begin()+=closedIndex);
		}
	}

	if (activeScript) {
		ScriptBaseEditor* editor = dynamic_cast<ScriptBaseEditor*>(EditorManager::getEditor(*activeScript));
		if (editor) {
			EditorInfo editorInfo;
			editorInfo.gui = &info.gui;
			editorInfo.gizmo = info.gizmo;
			editorInfo.camera = info.camera;
			editorInfo.world = Engine::getCurrentWorld();
			editorInfo.destroy = false;
			
			TextEditor& textEditor = editor->getTextEditor();
			if (ImGui::BeginMenuBar()) {
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Save"))
					{
						editor->saveCode();
					}
					if (ImGui::MenuItem("Apply")) {
						editor->applyCode();
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

				if (ImGui::BeginMenu("Lang")) {
					if (ImGui::MenuItem("General"))
						textEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::General());
					if (ImGui::MenuItem("C"))
						textEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::C());
					if (ImGui::MenuItem("Cpp"))
						textEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
					if (ImGui::MenuItem("Python"))
						textEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::Python());
					if (ImGui::MenuItem("HLSL"))
						textEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::HLSL());
					if (ImGui::MenuItem("GLSL"))
						textEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::GLSL());
					ImGui::EndMenu();
				}

				editor->onMenu(editorInfo);
				
				ImGui::EndMenuBar();
			}

			editor->onStatusGUI(editorInfo);
			editor->onTextEditor(editorInfo);
		}
	}
}
