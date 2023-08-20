#include "StateNodeEditor.h"
#include "../../GUI/GUIUtility.h"

RegistEditor(StateTransition);

void StateTransitionEditor::setInspectedObject(void* object)
{
	transition = dynamic_cast<StateTransition*>((Base*)object);
	GraphBaseEditor::setInspectedObject(transition);
}

void StateTransitionEditor::onGraphGUI(EditorInfo& info, GraphInfo& graphInfo)
{
}

void StateTransitionEditor::onLinkGUI(EditorInfo& info, GraphInfo& graphInfo, LinkInfo& linkInfo)
{
	StateEntryTransitionPin* entryPin = dynamic_cast<StateEntryTransitionPin*>(linkInfo.fromPin);
	if (entryPin)
		return;

	namespace ne = ax::NodeEditor;

	ImVec2 startPos, endPos;
	if (ne::GetLinkEndPoints(linkInfo.linkID, startPos, endPos)) {
		ne::BeginLinkEx();
		ImVec2 midPos = (startPos + endPos) * 0.5f;
		ImVec2 vec = endPos - startPos;
		Quaternionf rot = Quaternionf::FromTwoVectors({ 1.0f }, { vec.x, vec.y });
		midPos += (ImVec2&)(rot * Vector3f{ 0, -15 });

		ImVec2 mousePos = ImGui::GetMousePos();

		ImU32 color = linkInfo.color;

		if (ne::IsLinkSelected(linkInfo.linkID))
			color = (ImColor&)ne::GetStyle().Colors[ne::StyleColor_SelLinkBorder];
		if (ImLengthSqr(mousePos - midPos) < 100) {
			color = (ImColor&)ne::GetStyle().Colors[ne::StyleColor_SelLinkBorder];
			if (ne::IsBackgroundClicked()) {
				ne::SelectLink(linkInfo.linkID);
			}
			if (ne::IsBackgroundDoubleClicked()) {
				onDoubleClicked(info, graphInfo, linkInfo);
			}
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
				ne::Suspend();
				ImGui::OpenPopup(("TransitionCtx_" + to_string(transition->getInstanceID())).c_str());
				ne::Resume();
			}
		}


		ne::Suspend();
		if (ImGui::BeginPopup(("TransitionCtx_" + to_string(transition->getInstanceID())).c_str())) {
			onContextMenuGUI(info, graphInfo);
			ImGui::EndPopup();
		}
		ne::Resume();

		auto  drawList = ImGui::GetWindowDrawList();

		Vector3f lineO = Vector3f{ midPos.x, midPos.y };
		Vector3f lineA = { -7.2, 0 };
		Vector3f lineB = { 7.2, 0 };
		Vector3f lineC = { 2.2, -5 };
		lineA = rot * lineA + lineO;
		lineB = rot * lineB + lineO;
		lineC = rot * lineC + lineO;

		drawList->AddCircleFilled(midPos, 10, color);
		drawList->AddLine((ImVec2&)lineA, (ImVec2&)lineB, ImColor(0, 0, 0), 2);
		drawList->AddLine((ImVec2&)lineB, (ImVec2&)lineC, ImColor(0, 0, 0), 2);

		ne::EndLinkEx();
	}
}

void StateTransitionEditor::onDoubleClicked(EditorInfo& info, GraphInfo& graphInfo, LinkInfo& linkInfo)
{
}

RegistEditor(StateGraphTransition);

void StateGraphTransitionEditor::setInspectedObject(void* object)
{
	graphTransition = dynamic_cast<StateGraphTransition*>((Base*)object);
	StateTransitionEditor::setInspectedObject(graphTransition);
}

void StateGraphTransitionEditor::onDoubleClicked(EditorInfo& info, GraphInfo& graphInfo, LinkInfo& linkInfo)
{
	if (graphTransition) {
		Graph* graph = graphTransition->getGraph();
		graphInfo.openGraph(graph);
	}
}

RegistEditor(StateTransitionPin);

void StateTransitionPinEditor::setInspectedObject(void* object)
{
	transitionPin = dynamic_cast<StateTransitionPin*>((Base*)object);
	GraphPinEditor::setInspectedObject(transitionPin);
}

void StateTransitionPinEditor::onInspectGUI(EditorInfo& info)
{
}

void StateTransitionPinEditor::onGraphGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	namespace ne = ax::NodeEditor;
	namespace wg = ax::Widgets;
	if (pin == NULL)
		return;

	const float padding = 12.0f;

	StateNode* state = (StateNode*)pin->getNode();
	ne::NodeId nid = state->getInstanceID();
	ne::PinId pid = pin->getInstanceID();
	ne::PinKind pinKind = ne::PinKind::Output;

	if (transitionPin->getTransitionCount() == 0)
		pinKind = ne::PinKind::Input;

	for (int i = 0; i < transitionPin->getTransitionCount(); i++) {
		StateTransition* transition = transitionPin->getTransition(i);
		StateNode* nextState = transition->getNextState();
		if (nextState == NULL)
			continue;
		StateTransitionPin* nextPin = nextState->getTransitionPin();
		if (nextPin->isConnectedWith(state))
			pinKind = ne::PinKind::Input;
		ne::PinId nextPid = nextPin->getInstanceID();
		graphInfo.links.push_back({ pid, nextPid });
	}

	if (pinKind == ne::PinKind::Input) {
		ne::PushStyleVar(ne::StyleVar_PinArrowSize, 10.0f);
		ne::PushStyleVar(ne::StyleVar_PinArrowWidth, 10.0f);
		ne::PushStyleVar(ne::StyleVar_PinCorners, 12);
	}

	ne::BeginPin(pid, ne::PinKind::Output);

	GraphPin* newLinkPin = getGraphPin(graphInfo.newLinkPinID);
	float alpha = ImGui::GetStyle().Alpha * ((newLinkPin != NULL && !pin->isConnectable(newLinkPin)) ? 0.19 : 1);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
	Color pinColor = pin->getPinColor();
	ImColor color = { pinColor.r, pinColor.g, pinColor.b, alpha * pinColor.a };

	ImVec2 nodeSize = ne::GetNodeSize(nid);
	ImVec2 nodePos = ne::GetNodePosition(nid);

	ImRect rect = { { nodePos.x + padding, nodePos.y + padding },
		{ nodePos.x + nodeSize.x - padding, nodePos.y + nodeSize.y - padding } };

	ne::PinPivotRect(rect.Min, rect.Max);
	ne::PinRect(rect.Min, rect.Max);

	/*wg::Icon(ImVec2(24, 24), (ax::Drawing::IconType)pin->getPinShape(),
		transitionPin->getTransitionCount() > 0,color, ImColor(0.12f, 0.12f, 0.12f, alpha));*/
	//onPinGUI(info, graphInfo);

	ImGui::PopStyleVar();

	ne::EndPin();

	if (pinKind == ne::PinKind::Input)
		ne::PopStyleVar(3);
}

void StateTransitionPinEditor::onLinkGUI(EditorInfo& info, GraphInfo& graphInfo, LinkInfo& linkInfo)
{
	StateTransitionPin* toPin = dynamic_cast<StateTransitionPin*>(linkInfo.toPin);
	if (toPin == NULL)
		return;
	StateTransition* transition = transitionPin->getTransition(toPin);
	if (transition == NULL)
		return;
	StateTransitionEditor* editor = dynamic_cast<StateTransitionEditor*>(
		EditorManager::getEditor(*transition));
	if (editor == NULL)
		return;
	editor->onLinkGUI(info, graphInfo, linkInfo);
}

void StateTransitionPinEditor::onContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo)
{
}

RegistEditor(StateNode);

void StateNodeEditor::setInspectedObject(void* object)
{
	state = dynamic_cast<StateNode*>((Base*)object);
	GraphNodeEditor::setInspectedObject(state);
}

void StateNodeEditor::onInspectGUI(EditorInfo& info)
{
}

void StateNodeEditor::onGraphGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	if (node == NULL)
		return;

	namespace ne = ax::NodeEditor;
	namespace wg = ax::Widgets;

	const float rounding = 5;
	const float padding = 12.0f;

	ne::PushStyleColor(ne::StyleColor_NodeBg, ImColor(128, 128, 128, 200));
	ne::PushStyleColor(ne::StyleColor_NodeBorder, ImColor(32, 32, 32, 200));
	ne::PushStyleColor(ne::StyleColor_PinRect, ImColor(60, 180, 255, 150));
	ne::PushStyleColor(ne::StyleColor_PinRectBorder, ImColor(60, 180, 255, 150));

	ne::PushStyleVar(ne::StyleVar_NodePadding, ImVec4(0, 0, 0, 0));
	ne::PushStyleVar(ne::StyleVar_NodeRounding, rounding);
	ne::PushStyleVar(ne::StyleVar_SourceDirection, ImVec2(0.0f, 1.0f));
	ne::PushStyleVar(ne::StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
	ne::PushStyleVar(ne::StyleVar_LinkStrength, 0.0f);
	ne::PushStyleVar(ne::StyleVar_PinBorderWidth, 1.0f);
	ne::PushStyleVar(ne::StyleVar_PinRadius, 5.0f);

	ne::NodeId nid = node->getInstanceID();

	ne::BeginNode(nid);

	ImGui::BeginVertical("TextV", ImVec2(0.0f, 0.0f));
	ImGui::Spring(1, padding);
	ImGui::BeginHorizontal("TextH");
	ImGui::Spring(1, padding);
	ImGui::TextUnformatted(node->getName().c_str());
	ImGui::Spring(1, padding);
	ImGui::EndHorizontal();
	ImGui::Spring(1, padding);
	ImGui::EndVertical();
	ImRect contentRect = { ImGui::GetItemRectMin(), ImGui::GetItemRectMax() };

	GraphPin* pin = state->getTransitionPin();
	GraphPinEditor* pinEditor = dynamic_cast<GraphPinEditor*>(
		EditorManager::getEditor(*pin));
	if (pinEditor)
		pinEditor->onGraphGUI(info, graphInfo);

	ne::EndNode();

	ne::PopStyleVar(7);
	ne::PopStyleColor(4);

	/*auto drawList = ne::GetNodeBackgroundDrawList(nid);
	if (drawList) {
		drawList->AddRectFilled(contentRect.GetTL(), contentRect.GetBR(), IM_COL32(24, 64, 128, 200), 0.0f);
		drawList->AddRect(
			contentRect.GetTL(),
			contentRect.GetBR(),
			IM_COL32(48, 128, 255, 100), 0.0f);
	}*/

	if (ne::GetDoubleClickedNode() == nid)
		onDoubleClicked(info, graphInfo);
}

void StateNodeEditor::onContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo)
{
}

RegistEditor(StateEntryNode);

void StateEntryNodeEditor::setInspectedObject(void* object)
{
	entryState = dynamic_cast<StateEntryNode*>((Base*)object);
	GraphNodeEditor::setInspectedObject(entryState);
}

void StateEntryNodeEditor::onInspectGUI(EditorInfo& info)
{
}

void StateEntryNodeEditor::onGraphGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	if (node == NULL)
		return;

	namespace ne = ax::NodeEditor;
	namespace wg = ax::Widgets;

	const float rounding = 5;
	const float padding = 12.0f;

	ne::PushStyleColor(ne::StyleColor_NodeBg, ImColor(128, 128, 128, 200));
	ne::PushStyleColor(ne::StyleColor_NodeBorder, ImColor(32, 32, 32, 200));
	ne::PushStyleColor(ne::StyleColor_PinRect, ImColor(60, 180, 255, 150));
	ne::PushStyleColor(ne::StyleColor_PinRectBorder, ImColor(60, 180, 255, 150));

	ne::PushStyleVar(ne::StyleVar_NodePadding, ImVec4(0, 0, 0, 0));
	ne::PushStyleVar(ne::StyleVar_NodeRounding, rounding);
	ne::PushStyleVar(ne::StyleVar_SourceDirection, ImVec2(0.0f, 0.0f));
	ne::PushStyleVar(ne::StyleVar_TargetDirection, ImVec2(0.0f, 0.0f));
	ne::PushStyleVar(ne::StyleVar_LinkStrength, 0.0f);
	ne::PushStyleVar(ne::StyleVar_PinBorderWidth, 1.0f);
	ne::PushStyleVar(ne::StyleVar_PinRadius, 5.0f);

	ne::NodeId nid = node->getInstanceID();

	ne::BeginNode(nid);

	ImGui::BeginVertical("TextV", ImVec2(0.0f, 0.0f));
	ImGui::Spring(1, padding);
	ImGui::BeginHorizontal("TextH");
	ImGui::Spring(1, padding);
	ImGui::TextUnformatted(node->getName().c_str());
	ImGui::Spring(1, padding);
	ImGui::EndHorizontal();
	ImGui::Spring(1, padding);
	ImGui::EndVertical();
	ImRect contentRect = { ImGui::GetItemRectMin(), ImGui::GetItemRectMax() };

	GraphPin* pin = entryState->getTransitionPin();
	GraphPinEditor* pinEditor = dynamic_cast<GraphPinEditor*>(
		EditorManager::getEditor(*pin));
	if (pinEditor)
		pinEditor->onGraphGUI(info, graphInfo);

	ne::EndNode();

	ne::PopStyleVar(7);
	ne::PopStyleColor(4);

	/*auto drawList = ne::GetNodeBackgroundDrawList(nid);
	if (drawList) {
		drawList->AddRectFilled(contentRect.GetTL(), contentRect.GetBR(), IM_COL32(24, 64, 128, 200), 0.0f);
		drawList->AddRect(
			contentRect.GetTL(),
			contentRect.GetBR(),
			IM_COL32(48, 128, 255, 100), 0.0f);
	}*/
}

void StateEntryNodeEditor::onContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo)
{
}
