#include "PythonObjectBehaviorEditor.h"

RegistEditor(PythonObjectBehavior);

void PythonObjectBehaviorEditor::setInspectedObject(void* object)
{
	pythonObjectBehavior = dynamic_cast<PythonObjectBehavior*>((Base*)object);
	ObjectBehaviorEditor::setInspectedObject(pythonObjectBehavior);
}

void PythonObjectBehaviorEditor::onBehaviorGUI(EditorInfo& info)
{
	PythonScript* script = pythonObjectBehavior->getScript();
	if (script == NULL)
		ImGui::Text("No Script");
	else
		ImGui::Text("Script Class: %s", script->getName().c_str());
	if (ImGui::Button("Attach Selected Script", { -1, 36 })) {
		Asset* assignAsset = EditorManager::getSelectedAsset();
		if (assignAsset != NULL && assignAsset->assetInfo.type == "PythonScript") {
			pythonObjectBehavior->setScript(*(PythonScript*)assignAsset->load());
			pythonObjectBehavior->begin();
		}
	}
	if (script != NULL) {
		if (ImGui::Button("Remove Script", { -1, 36 })) {
			pythonObjectBehavior->removeScript();
		}
	}
}
