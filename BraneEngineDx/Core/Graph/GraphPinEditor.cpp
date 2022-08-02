#include "GraphPinEditor.h"
#include "../Utility/hash.h"

RegistEditor(GraphPin);

void GraphPinEditor::setInspectedObject(void* object)
{
	pin = dynamic_cast<GraphPin*>((Base*)object);
	GraphBaseEditor::setInspectedObject(pin);
}

void GraphPinEditor::onInspectGUI(EditorInfo& info)
{
}

void GraphPinEditor::onGraphGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	namespace ne = ax::NodeEditor;
	namespace wg = ax::Widgets;
	if (pin == NULL)
		return;
	ne::PinId pid = pin->getInstanceID();
	if (pin->isOutputPin()) {
		graphInfo.builder->Output(pid);
		ImGui::TextUnformatted(pin->getDisplayName().c_str());
	}
	else
		graphInfo.builder->Input(pid);
	GraphPin* newLinkPin = getGraphPin(graphInfo.newLinkPinID);
	GraphPin* linkPin = pin->getConnectedPin();
	float alpha = ImGui::GetStyle().Alpha * ((newLinkPin != NULL && !pin->isConnectable(newLinkPin)) ? 0.19 : 1);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
	Color pinColor = pin->getPinColor();
	ImColor color = { pinColor.r, pinColor.g, pinColor.b, alpha * pinColor.a };
	wg::Icon(ImVec2(24, 24), (ax::Drawing::IconType)pin->getPinShape(), linkPin, color, ImColor(0.12f, 0.12f, 0.12f, alpha));
	if (!pin->isOutputPin()) {
		onPinGUI(info, graphInfo);
	}
	if (pin->isOutputPin())
		graphInfo.builder->EndOutput();
	else {
		graphInfo.builder->EndInput();
		if (linkPin) {
			ne::PinId outputPid = linkPin->getInstanceID();
			graphInfo.links.push_back({ outputPid, pid });
		}
	}
	ImGui::PopStyleVar();
}

void GraphPinEditor::onPinGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	ImGui::TextUnformatted(pin->getDisplayName().c_str());
}

void GraphPinEditor::onLinkGUI(EditorInfo& info, GraphInfo& graphInfo, LinkInfo& linkInfo)
{
}

void GraphPinEditor::onContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	ImGui::TextUnformatted("Pin Context Menu");
	ImGui::Separator();
	ImGui::Text("Name: %p", pin->getName().c_str());
	ImGui::Text("GUID: %s", pin->getAssetHandle().guid.toString().c_str());
}

GraphPin* getGraphPin(ax::NodeEditor::PinId id)
{
	return (GraphPin*)Brane::getPtrByInsID(id.Get());
}

ax::NodeEditor::LinkId getLinkID(ax::NodeEditor::PinId fromID, ax::NodeEditor::PinId toID)
{
	const int halfway = ((sizeof(ax::NodeEditor::PinId) * CHAR_BIT) / 2);
	return ((fromID.Get() << halfway)) | (toID.Get());
}

void getLinkedPinID(ax::NodeEditor::LinkId linkID, ax::NodeEditor::PinId& fromID, ax::NodeEditor::PinId& toID)
{
	const int halfway = ((sizeof(ax::NodeEditor::PinId) * CHAR_BIT) / 2);
	fromID = linkID.Get() >> halfway;
	toID = (linkID.Get() << halfway) >> halfway;
}
