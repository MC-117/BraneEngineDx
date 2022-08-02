#include "GraphNodeEditor.h"

RegistEditor(GraphNode);

void GraphNodeEditor::setInspectedObject(void* object)
{
	node = dynamic_cast<GraphNode*>((Base*)object);
	GraphBaseEditor::setInspectedObject(node);
}

void GraphNodeEditor::setNodePosition(const Vector2f& position)
{
	nodePosition = position;
}

Vector2f GraphNodeEditor::getNodePosition() const
{
	return nodePosition;
}

void GraphNodeEditor::onDoubleClicked(EditorInfo& info, GraphInfo& graphInfo)
{
}

void GraphNodeEditor::onInspectGUI(EditorInfo& info)
{
	for (int i = 0; i < node->getInputCount(); i++) {
		GraphPin* pin = node->getInput(i);
		if (!pin)
			continue;
		GraphPinEditor* pinEditor = dynamic_cast<GraphPinEditor*>(
			EditorManager::getEditor(*pin));
		if (pinEditor)
			pinEditor->onInspectGUI(info);
	}

	for (int i = 0; i < node->getOutputCount(); i++) {
		GraphPin* pin = node->getOutput(i);
		if (!pin)
			continue;
		GraphPinEditor* pinEditor = dynamic_cast<GraphPinEditor*>(
			EditorManager::getEditor(*pin));
		if (pinEditor)
			pinEditor->onInspectGUI(info);
	}
}

void GraphNodeEditor::onGraphGUI(EditorInfo& info, GraphInfo& graphInfo)
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

	if (ne::GetDoubleClickedNode() == nid) {
		onDoubleClicked(info, graphInfo);
	}
}

void GraphNodeEditor::onContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	ImGui::TextUnformatted("Node Context Menu");
	ImGui::Separator();
	ImGui::Text("Name: %s", node->getDisplayName().c_str());
	ImGui::Text("GUID: %s", node->getAssetHandle().guid.toString().c_str());
	ImGui::Text("Inputs: %d", node->getInputCount());
	ImGui::Text("Outputs: %d", node->getOutputCount());
	ImGui::Separator();
	if (ImGui::MenuItem("Delete"))
		graphInfo.deleteNodeID = node->getInstanceID();
}

GraphNode* getGraphNode(ax::NodeEditor::NodeId id)
{
	return (GraphNode*)Brane::getPtrByInsID(id.Get());
}
