#include "StateNode.h"

SerializeInstance(StateTransition);

StateTransition::StateTransition() : Base()
{
}

void StateTransition::init(StateNode* fromNode, StateNode* toNode)
{
	state = fromNode;
	nextState = toNode;
}

void StateTransition::onBeginTransition(GraphContext& context)
{
}

void StateTransition::onTransition(GraphContext& context)
{
}

void StateTransition::onEndTransition(GraphContext& context)
{
}

StateNode* StateTransition::getState() const
{
	return state;
}

StateNode* StateTransition::getNextState() const
{
	return nextState;
}

bool StateTransition::canTransition() const
{
	return true;
}

bool StateTransition::completeTransition() const
{
	return true;
}

bool StateTransition::process(GraphContext& context)
{
	if (!getNextState())
		return false;
	return true;
}

Serializable* StateTransition::instantiate(const SerializationInfo& from)
{
	return new StateTransition();
}

bool StateTransition::deserialize(const SerializationInfo& from)
{
	Base::deserialize(from);
	const SerializationInfo* nextStateInfo = from.get("nextState");
	if (nextStateInfo)
		nextState.deserialize(*nextStateInfo);
	return true;
}

bool StateTransition::serialize(SerializationInfo& to)
{
	Base::serialize(to);
	SerializationInfo* nextStateInfo = to.add("nextState");
	if (nextStateInfo)
		nextState.serialize(*nextStateInfo);
	return true;
}

SerializeInstance(StateGraphTransition);

StateGraphTransition::StateGraphTransition() : StateTransition()
{
	transitionPin = new BoolPin("canTransit");
	transitionGraph.addReturn(transitionPin);
	transitionGraph.addReturnNode();
}

void StateGraphTransition::init(StateNode* fromNode, StateNode* toNode)
{
	StateTransition::init(fromNode, toNode);
	if (fromNode && toNode)
		transitionGraph.setName(fromNode->getName() + "To" + toNode->getName());
}

Graph* StateGraphTransition::getGraph()
{
	return &transitionGraph;
}

const Graph* StateGraphTransition::getGraph() const
{
	return &transitionGraph;
}

bool StateGraphTransition::canTransition() const
{
	if (transitionPin == NULL)
		return false;
	return transitionPin->getValue();
}

bool StateGraphTransition::completeTransition() const
{
	if (transitionPin == NULL)
		return false;
	return transitionPin->getValue();
}

bool StateGraphTransition::process(GraphContext& context)
{
	if (!getNextState())
		return false;
	GraphContext localContext;
	localContext.deltaTime = context.deltaTime;
	return transitionGraph.solveState(localContext);
}

Serializable* StateGraphTransition::instantiate(const SerializationInfo& from)
{
	return new StateGraphTransition();
}

bool StateGraphTransition::deserialize(const SerializationInfo& from)
{
	if (transitionGraph.getOutputCount() != 1) {
		return false;
	}
	if (!StateTransition::deserialize(from))
		return false;
	const SerializationInfo* transitionGraphInfo = from.get("transitionGraph");
	if (transitionGraphInfo)
		transitionGraph.deserialize(*transitionGraphInfo);
	transitionPin = dynamic_cast<BoolPin*>(transitionGraph.getOutput(0));
	return true;
}

bool StateGraphTransition::serialize(SerializationInfo& to)
{
	if (transitionGraph.getOutputCount() != 1) {
		return false;
	}
	if (!StateTransition::serialize(to))
		return false;
	SerializationInfo* transitionGraphInfo = to.add("transitionGraph");
	if (transitionGraphInfo)
		transitionGraph.serialize(*transitionGraphInfo);
	return true;
}

SerializeInstance(StateTransitionPin);

StateTransitionPin::StateTransitionPin() : FlowPin("Transition")
{
}

bool StateTransitionPin::switchTransitionType(Serialization& type)
{
	if (&type != &StateTransition::StateTransitionSerialization::serialization &&
		!type.isChildOf(StateTransition::StateTransitionSerialization::serialization))
		return false;
	transitionType = &type;
	return true;
}

bool StateTransitionPin::isConnectable(GraphPin* pin) const
{
	if (pin == this || transitionType == NULL)
		return false;
	StateTransitionPin* transitionPin = dynamic_cast<StateTransitionPin*>(pin);
	if (transitionPin == NULL)
		return false;
	StateNode* pinState = dynamic_cast<StateNode*>(pin->getNode());
	if (pinState == NULL)
		return false;
	for (int i = 0; i < getTransitionCount(); i++) {
		if (pinState == getTransition(i)->getNextState())
			return false;
	}
	return true;
}

bool StateTransitionPin::connect(GraphPin* pin)
{
	if (!isConnectable(pin))
		return false;
	Serializable* serializable = transitionType->instantiate(SerializationInfo());
	if (serializable == NULL)
		return false;
	StateTransition* transition = dynamic_cast<StateTransition*>(serializable);
	if (transition == NULL) {
		delete serializable;
		return false;
	}
	transition->init((StateNode*)node, (StateNode*)pin->getNode());
	transitions.push_back(transition);
	return true;
}

bool StateTransitionPin::disconnect(GraphPin* pin)
{
	StateTransitionPin* transitionPin = dynamic_cast<StateTransitionPin*>(pin);
	if (transitionPin == NULL)
		return false;
	for (int i = 0; i < transitionPin->getTransitionCount(); i++) {
		StateTransition* transition = transitionPin->getTransition(i);
		if (getNode() == transition->getNextState()) {
			transitionPin->transitions.erase(transitionPin->transitions.begin()+=i);
			delete transition;
			return true;
		}
	}
	return false;
}

bool StateTransitionPin::isConnectedWith(const StateNode* state) const
{
	if (state == NULL)
		return false;
	for (int i = 0; i < getTransitionCount(); i++) {
		StateTransition* transition = getTransition(i);
		if (transition->getNextState() == state)
			return true;
	}
	return false;
}

void StateTransitionPin::clearTransition()
{
	for (int i = 0; i < getTransitionCount(); i++) {
		StateTransition* transition = getTransition(i);
		delete transition;
	}
	transitions.clear();
}

int StateTransitionPin::getTransitionCount() const
{
	return transitions.size();
}

StateTransition* StateTransitionPin::getTransition(int index) const
{
	return transitions[index];
}

StateTransition* StateTransitionPin::getTransition(const StateTransitionPin* targetPin) const
{
	for (int i = 0; i < getTransitionCount(); i++) {
		StateTransition* transition = getTransition(i);
		if (transition->getNextState() == targetPin->getNode())
			return transition;
	}
	return NULL;
}

Serializable* StateTransitionPin::instantiate(const SerializationInfo& from)
{
	return new StateTransitionPin();
}

bool StateTransitionPin::deserialize(const SerializationInfo& from)
{
	if (!FlowPin::deserialize(from))
		return false;
	const SerializationInfo* transitionInfos = from.get("transitions");
	if (transitionInfos) {
		for (int i = 0; i < transitionInfos->sublists.size(); i++) {
			const SerializationInfo& info = transitionInfos->sublists[i];
			if (info.serialization == NULL)
				continue;
			Serializable* serializable = info.serialization->instantiate(info);
			if (serializable == NULL)
				continue;
			StateTransition* transition = dynamic_cast<StateTransition*>(serializable);
			if (transition == NULL) {
				delete serializable;
				continue;
			}
			transition->init(dynamic_cast<StateNode*>(node), NULL);
			transition->deserialize(info);
			transitions.push_back(transition);
		}
	}
	return true;
}

bool StateTransitionPin::serialize(SerializationInfo& to)
{
	if (!FlowPin::serialize(to))
		return false;
	SerializationInfo* transitionInfos = to.add("transitions");
	if (transitionInfos) {
		for (int i = 0; i < transitions.size(); i++) {
			StateTransition* transition = transitions[i];
			SerializationInfo* info = transitionInfos->add(to_string(i));
			transition->serialize(*info);
		}
	}
	return true;
}

SerializeInstance(StateNode);

StateNode::StateNode()
{
	setTransitionPin(new StateTransitionPin());
}

void StateNode::setTransitionPin(StateTransitionPin* pin)
{
	if (pin == NULL)
		return;
	clearOutputs();
	transitionPin = pin;
	addOutput(*transitionPin);
}

StateTransitionPin* StateNode::getTransitionPin() const
{
	return transitionPin;
}

int StateNode::getTransitionCount() const
{
	return transitionPin->getTransitionCount();
}

StateTransition* StateNode::getTransition(int index) const
{
	return transitionPin->getTransition(index);
}

bool StateNode::isConnectedWith(const StateNode* state) const
{
	return transitionPin->isConnectedWith(state);
}

StateNode* StateNode::getNextState() const
{
	return nextState;
}

StateTransition* StateNode::getActiveTransition() const
{
	return activeTransition;
}

void StateNode::onEnterState(GraphContext& context)
{
	nextState = NULL;
	activeTransition = NULL;
}

void StateNode::onUpdateState(GraphContext& context)
{
}

void StateNode::onTransitState(GraphContext& context)
{
}

void StateNode::onLeaveState(GraphContext& context)
{
}

bool StateNode::process(GraphContext& context)
{
	if (activeTransition)
		return activeTransition->process(context);
	for (int i = 0; i < getTransitionCount(); i++) {
		StateTransition* transition = getTransition(i);
		transition->process(context);
		if (transition->canTransition()) {
			nextState = transition->getNextState();
			activeTransition = transition;
			break;
		}
	}
	return true;
}

Serializable* StateNode::instantiate(const SerializationInfo& from)
{
	return new StateNode();
}

void StateNode::addInput(GraphPin& input)
{
}

void StateNode::addOutput(GraphPin& output)
{
	StateTransitionPin* pin = dynamic_cast<StateTransitionPin*>(&output);
	if (pin)
		GraphNode::addOutput(*pin);
}

SerializeInstance(StateEntryTransitionPin);

bool StateEntryTransitionPin::connect(GraphPin* pin)
{
	if (!isConnectable(pin))
		return false;
	clearTransition();
	return StateTransitionPin::connect(pin);
}

Serializable* StateEntryTransitionPin::instantiate(const SerializationInfo& from)
{
	return new StateEntryTransitionPin();
}

SerializeInstance(StateEntryNode);

StateEntryNode::StateEntryNode()
{
	name = "Entry";
	transitionPin = new StateEntryTransitionPin();
	addOutput(*transitionPin);
}

StateTransitionPin* StateEntryNode::getTransitionPin() const
{
	return transitionPin;
}

StateNode* StateEntryNode::getNextState() const
{
	return nextState;
}

bool StateEntryNode::process(GraphContext& context)
{
	if (nextState)
		return true;
	for (int i = 0; i < transitionPin->getTransitionCount(); i++) {
		StateTransition* transition = transitionPin->getTransition(i);
		transition->process(context);
		if (transition->canTransition()) {
			nextState = transition->getNextState();
			break;
		}
	}
	return true;
}

Serializable* StateEntryNode::instantiate(const SerializationInfo& from)
{
	return new StateEntryNode();
}
