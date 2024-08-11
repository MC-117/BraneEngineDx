#include "StateMachine.h"

SerializeInstance(StateMachine);

StateMachine::StateMachine()
{
    flag = Flag::Expression;
    setEntryNode();
}

bool StateMachine::addState(StateNode* state)
{
    if (state == NULL || state->getName().empty() || getState(state->getName()))
        return false;
    stateNameMap[state->getName()] = state;
    nodes.push_back(state);
    return true;
}

bool StateMachine::addNode(GraphNode* node)
{
    EntryNode* entryNode = dynamic_cast<EntryNode*>(node);
    if (entryNode)
        return setEntryNode();
    StateNode* state = dynamic_cast<StateNode*>(node);
    if (state == NULL)
        return false;
    return addState(state);
}

bool StateMachine::removeNode(GraphNode* node)
{
    StateNode* state = dynamic_cast<StateNode*>(node);
    if (state)
        stateNameMap.erase(node->getName());
    return Graph::removeNode(node);
}

bool StateMachine::setEntryNode()
{
    if (entryNode)
        return false;
    entryNode = new StateEntryNode();
    nodes.push_back(entryNode);
    return true;
}

int StateMachine::getStateCount() const
{
    return getNodeCount();
}

StateNode* StateMachine::getState(int index) const
{
    return dynamic_cast<StateNode*>(getNode(index));
}

StateNode* StateMachine::getState(const Name& name) const
{
    auto iter = stateNameMap.find(name);
    if (iter == stateNameMap.end())
        return NULL;
    return iter->second;
}

StateNode* StateMachine::getCurrentState() const
{
    return currentState;
}

StateTransition* StateMachine::getActiveTransition() const
{
    return activeTransition;
}

bool StateMachine::checkStateName(const string& name) const
{
    return getState(name) == NULL;
}

bool StateMachine::process(GraphContext& context)
{
    if (currentState) {
        currentState->process(context);
        StateNode* nextState = currentState->getNextState();
        StateTransition* transition = currentState->getActiveTransition();
        if (nextState && transition) {
            if (transition->completeTransition()) {
                transition->onEndTransition(context);
                activeTransition = NULL;
                currentState->onLeaveState(context);
                currentState = nextState;
                currentState->onEnterState(context);
                currentState->onUpdateState(context);
            }
            else {
                if (activeTransition == NULL) {
                    activeTransition = transition;
                    activeTransition->onBeginTransition(context);
                }
                currentState->onUpdateState(context);
                nextState->onUpdateState(context);
                transition->onTransition(context);
                currentState->onTransitState(context);
            }
        }
        else {
            currentState->onUpdateState(context);
        }
    }
    else {
        StateEntryNode* entryState = dynamic_cast<StateEntryNode*>(entryNode);
        if (entryState) {
            entryState->process(context);
            StateNode* nextState = entryState->getNextState();
            if (nextState) {
                currentState = nextState;
                currentState->onEnterState(context);
                currentState->onUpdateState(context);
            }
        }
    }
    return true;
}

Serializable* StateMachine::instantiate(const SerializationInfo& from)
{
    return new StateMachine();
}

void StateMachine::addInput(GraphPin& input)
{
    GraphNode::addInput(input);
}

void StateMachine::addOutput(GraphPin& output)
{
    GraphNode::addOutput(output);
}
