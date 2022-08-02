#include "ValueNodeEditor.h"
#include "../GUI/GUIUtility.h"

RegistEditor(CastToNode);

void CastToNodeEditor::setInspectedObject(void* object)
{
	castToNode = dynamic_cast<CastToNode*>((Base*)object);
	GraphNodeEditor::setInspectedObject(castToNode);
}

void CastToNodeEditor::onGraphGUI(EditorInfo& info, GraphInfo& graphInfo)
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

	ValuePin* fromPin = castToNode->getFromPin();
	if (fromPin) {
		GraphPinEditor* pinEditor = dynamic_cast<GraphPinEditor*>(
			EditorManager::getEditor(*fromPin));
		if (pinEditor)
			pinEditor->onGraphGUI(info, graphInfo);
	}

	ValuePin* toPin = castToNode->getToPin();
	if (toPin) {
		GraphPinEditor* pinEditor = dynamic_cast<GraphPinEditor*>(
			EditorManager::getEditor(*toPin));
		if (pinEditor)
			pinEditor->onGraphGUI(info, graphInfo);
	}

	graphInfo.builder->End();
}

void CastToNodeEditor::onContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	GraphNodeEditor::onContextMenuGUI(info, graphInfo);
	ValuePin* fromPin = castToNode->getFromPin();
	Serialization* fromType = fromPin == NULL ? NULL : &fromPin->getSerialization();
	ImGui::SetNextItemWidth(60);
	if (ImGui::TypeCombo("FromType", fromType, ValuePin::ValuePinSerialization::serialization)) {
		ValuePin* newPin = NULL;
		if (fromType) {
			Serializable* serialiable = fromType->instantiate(SerializationInfo());
			if (serialiable) {
				newPin = dynamic_cast<ValuePin*>(serialiable);
				if (newPin == NULL)
					delete serialiable;
			}
		}
		castToNode->setFromPin(newPin);
	}

	ValuePin* toPin = castToNode->getToPin();
	Serialization* toType = toPin == NULL ? NULL : &toPin->getSerialization();
	ImGui::SetNextItemWidth(60);
	if (ImGui::TypeCombo("##ToType", toType, ValuePin::ValuePinSerialization::serialization)) {
		ValuePin* newPin = NULL;
		if (toType) {
			Serializable* serialiable = toType->instantiate(SerializationInfo());
			if (serialiable) {
				newPin = dynamic_cast<ValuePin*>(serialiable);
				if (newPin == NULL)
					delete serialiable;
			}
		}
		castToNode->setToPin(newPin);
	}
}