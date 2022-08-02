#include "StateMachineEditor.h"
#include "../../GUI/GUIUtility.h"

RegistEditor(StateMachine);

void StateMachineEditor::setInspectedObject(void* object)
{
	stateMachine = dynamic_cast<StateMachine*>((Base*)object);
	GraphEditor::setInspectedObject(stateMachine);
}

void StateMachineEditor::onExtraContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	if (ImGui::BeginPopup("CreateLinkCtx")) {
		vector<Serialization*> nodeTypes;
		nodeTypes.push_back(&StateTransition::StateTransitionSerialization::serialization);
		StateTransition::StateTransitionSerialization::serialization.getChildren(nodeTypes);

		StateTransition* transition = NULL;

		for each (auto type in nodeTypes) {
			if (ImGui::MenuItem(type->type.c_str())) {
				StateTransitionPin* fromPin = dynamic_cast<StateTransitionPin*>(linkFromPin);
				StateTransitionPin* toPin = dynamic_cast<StateTransitionPin*>(linkToPin);
				if (fromPin && toPin) {
					fromPin->switchTransitionType(*type);
					fromPin->connect(toPin);
					linkFromPin = NULL;
					linkToPin = NULL;
				}
			}
		}
		ImGui::EndPopup();
	}
}

void StateMachineEditor::onCreateLink(GraphPin* fromPin, GraphPin* toPin)
{
	namespace ne = ax::NodeEditor;
	linkFromPin = fromPin;
	linkToPin = toPin;
	ne::Suspend();
	ImGui::OpenPopup("CreateLinkCtx");
	ne::Resume();
}

void StateMachineEditor::onGraphCanvasGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	namespace ne = ax::NodeEditor;
	StateNode* currentState = stateMachine->getCurrentState();
	StateTransition* activeTransition = stateMachine->getActiveTransition();
	if (currentState) {
		ne::PushStyleVar(ne::StyleVar_FlowDuration, 0.2f);
		ne::NodeId nodeID = currentState->getInstanceID();
		ne::Flow(nodeID);
		if (activeTransition) {
			StateNode* nextNode = activeTransition->getNextState();
			if (nextNode) {
				ne::LinkId linkID = getLinkID(activeTransition->getState()->getTransitionPin()->getInstanceID(),
					nextNode->getTransitionPin()->getInstanceID());
				ne::Flow(linkID);
			}
		}
		ne::PopStyleVar();
	}
	GraphEditor::onGraphCanvasGUI(info, graphInfo);
}

void StateMachineEditor::onCanvasContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	if (graph == NULL)
		return;
	ImGui::InputText("Filter", &createNodeFilter);
	ImGui::InputText("Name", &stateName);

	if (stateMachine->checkStateName(stateName)) {
		ImGui::BeginChild("NodeTypes", { -1, 300 });

		vector<Serialization*> nodeTypes;
		StateNode::StateNodeSerialization::serialization.getChildren(nodeTypes);

		StateNode* state = NULL;
		for each (auto type in nodeTypes) {
			if (!createNodeFilter.empty() && type->type.find(createNodeFilter) == string::npos)
				continue;
			if (ImGui::MenuItem(type->type.c_str())) {
				if (type != &EntryNode::EntryNodeSerialization::serialization) {
					SerializationInfo info;
					Serializable* serializable = type->instantiate(info);
					if (serializable) {
						state = dynamic_cast<StateNode*>(serializable);
						if (!state)
							delete serializable;
					}
				}
			}
		}

		if (state) {
			state->setName(stateName);
			if (stateMachine->addState(state)) {
				createNewNode = false;
				ax::NodeEditor::SetNodePosition(state->getInstanceID(), popupCurserPos);
			}
			else {
				delete state;
				state = NULL;
			}
		}

		ImGui::EndChild();
	}
	else {
		ImGui::TextUnformatted("Invalid state name");
	}
}
