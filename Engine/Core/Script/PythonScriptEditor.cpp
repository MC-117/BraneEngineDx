#include "PythonScriptEditor.h"

RegistEditor(PythonScript);

PythonScriptEditor::~PythonScriptEditor()
{
    if (runtimeObject != NULL) {
        Py_DECREF(runtimeObject);
        runtimeObject = NULL;
    }
    functions.clear();
    pythonScript = NULL;
}

void PythonScriptEditor::setInspectedObject(void* object)
{
    pythonScript = dynamic_cast<PythonScript*>((ScriptBase*)object);
    ScriptBaseEditor::setInspectedObject(pythonScript);

    if (pythonScript == NULL)
        return;

    textEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::Python());
    textEditor.SetText(pythonScript->getSourceCode());
    if (pythonScript->getType() == PythonScript::Behavior) {
        if (runtimeObject == NULL) {
            runtimeObject = new PythonRuntimeObject();
            runtimeObject->setScript(*pythonScript);
        }
        functions.clear();
        runtimeObject->getCallables(functions);
    }
}

void PythonScriptEditor::onMenu(EditorInfo& info)
{
    ScriptBaseEditor::onMenu(info);
    if (ImGui::BeginMenu("Functions")) {
        for (int i = 0; i < functions.size(); i++) {
            PythonFunctionObject& function = functions[i];
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

void PythonScriptEditor::onTextEditor(EditorInfo& info)
{
    switch (pythonScript->getType())
    {
    case PythonScript::Wrapper:
        ScriptBaseEditor::onTextEditor(info);
        break;
    case PythonScript::Behavior:
        textEditor.Render("##ScriptEditorCode", { -122, -1 });
        ImGui::SameLine();
        ImGui::BeginChild("##ScriptEditorFunctions", { 120,-1 });
        for (int i = 0; i < functions.size(); i++) {
            PythonFunctionObject& function = functions[i];
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

void PythonScriptEditor::onApplyCode()
{
    if (pythonScript->getType() == PythonScript::Behavior) {
        if (runtimeObject == NULL) {
            runtimeObject = new PythonRuntimeObject();
            runtimeObject->setScript(*pythonScript);
        }
        functions.clear();
        runtimeObject->getCallables(functions);
    }
}
