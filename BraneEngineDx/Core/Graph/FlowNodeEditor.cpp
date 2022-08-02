#include "FlowNodeEditor.h"
#include "../GUI/GUIUtility.h"

RegistEditor(SequenceNode);

void SequenceNodeEditor::setInspectedObject(void* object)
{
	sequenceNode = dynamic_cast<SequenceNode*>((Base*)object);
	GraphNodeEditor::setInspectedObject(sequenceNode);
}

void SequenceNodeEditor::onGraphGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	if (node == NULL)
		return;

	namespace ne = ax::NodeEditor;
	namespace wg = ax::Widgets;

	ne::NodeId nid = node->getInstanceID();

	graphInfo.builder->Begin(nid);

	graphInfo.builder->Header((ImVec4&)node->getNodeColor());
	ImGui::Spring(0);
	ImGui::TextUnformatted(node->getDisplayName().c_str());
	ImGui::Spring(1);
	ImGui::Dummy(ImVec2(0, 28));
	graphInfo.builder->EndHeader();

	for (int i = 0; i < node->getInputCount(); i++) {
		GraphPin* pin = node->getInput(i);
		if (!pin)
			continue;
		GraphPinEditor* pinEditor = dynamic_cast<GraphPinEditor*>(
			EditorManager::getEditor(*pin));
		if (pinEditor)
			pinEditor->onGraphGUI(info, graphInfo);
	}

	graphInfo.builder->Middle();
	ImGui::Spring(1, 0);
	if (ImGui::Button(ICON_FA_PLUS, { 24, 24 })) {
		sequenceNode->addFlowPin();
	}

	for (int i = 0; i < node->getOutputCount(); i++) {
		GraphPin* pin = node->getOutput(i);
		if (!pin)
			continue;
		GraphPinEditor* pinEditor = dynamic_cast<GraphPinEditor*>(
			EditorManager::getEditor(*pin));
		if (pinEditor)
			pinEditor->onGraphGUI(info, graphInfo);
	}

	graphInfo.builder->End();
}
