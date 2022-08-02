#include "ObjectGraphEditor.h"
#include "GraphWindow.h"
#include "../GUI/GUIUtility.h"

RegistEditor(ObjectGraph);

void ObjectGraphEditor::setInspectedObject(void* object)
{
	objectGraph = dynamic_cast<ObjectGraph*>((Base*)object);
	ObjectBehaviorEditor::setInspectedObject(objectGraph);
}

void ObjectGraphEditor::onBehaviorGUI(EditorInfo& info)
{
	bool tickable = objectGraph->isTickable();
	if (ImGui::Checkbox("Tick", &tickable)) {
		objectGraph->setTickable(tickable);
	}
	Graph* graph = &objectGraph->getGraph();
	if (ImGui::Button("OpenGraph", { -1, 36 })) {
		GraphWindow::showGraph(*info.gui, graph);
	}

	if (ImGui::CollapsingHeader("Variables")) {
		for (int i = 0; i < graph->getVariableCount(); i++)
		{
			GraphVariable* variable = graph->getVariable(i);
			GraphBaseEditor* editor = dynamic_cast<GraphBaseEditor*>(EditorManager::getEditor(*variable));
			if (editor) {
				ImGui::BeginGroupPanel(variable->getName().c_str());
				editor->onInspectGUI(info);
				ImGui::EndGroupPanel();
			}
		}
	}
}
