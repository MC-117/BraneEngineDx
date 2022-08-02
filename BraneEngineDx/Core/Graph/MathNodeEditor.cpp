#include "MathNodeEditor.h"
#include "../GUI/GUIUtility.h"

RegistEditor(MathOperationNode);

void MathOperationNodeEditor::setInspectedObject(void* object)
{
	mathOperationNode = dynamic_cast<MathOperationNode*>((Base*)object);
	GraphNodeEditor::setInspectedObject(mathOperationNode);
}

void MathOperationNodeEditor::onGraphGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	if (node == NULL)
		return;

	namespace ne = ax::NodeEditor;
	namespace wg = ax::Widgets;

	ne::NodeId nid = node->getInstanceID();

	graphInfo.builder->Begin(nid);

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
	ImGui::TextUnformatted(node->getDisplayName().c_str());
	ImGui::Spring(1, 0);
	if (ImGui::Button(ICON_FA_PLUS, { 20, 20 })) {
		mathOperationNode->addInputPin();
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

RegistEditor(ComparisonNode);

void ComparisonNodeEditor::setInspectedObject(void* object)
{
	comparisonNode = dynamic_cast<ComparisonNode*>((Base*)object);
	GraphNodeEditor::setInspectedObject(comparisonNode);
}

void ComparisonNodeEditor::onGraphGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	if (node == NULL)
		return;

	namespace ne = ax::NodeEditor;
	namespace wg = ax::Widgets;

	ne::NodeId nid = node->getInstanceID();

	graphInfo.builder->Begin(nid);

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
	ImGui::TextUnformatted(node->getDisplayName().c_str());
	ImGui::Spring(1, 0);

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

RegistEditor(NotNode);

void NotNodeEditor::setInspectedObject(void* object)
{
	notNode = dynamic_cast<NotNode*>((Base*)object);
	GraphNodeEditor::setInspectedObject(notNode);
}

void NotNodeEditor::onGraphGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	if (node == NULL)
		return;

	namespace ne = ax::NodeEditor;
	namespace wg = ax::Widgets;

	ne::NodeId nid = node->getInstanceID();

	graphInfo.builder->Begin(nid);

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
	ImGui::TextUnformatted(node->getDisplayName().c_str());
	ImGui::Spring(1, 0);

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
