#pragma once

#include "StateNode.h"

class ENGINE_API StateMachine : public Graph
{
public:
	Serialize(StateMachine, Graph);

	StateMachine();
	virtual ~StateMachine() = default;

	virtual bool addState(StateNode* state);

	virtual bool addNode(GraphNode* node);

	virtual bool removeNode(GraphNode* node);

	bool setEntryNode();

	virtual int getStateCount() const;
	virtual StateNode* getState(int index) const;
	virtual StateNode* getState(const string& name) const;

	StateNode* getCurrentState() const;
	StateTransition* getActiveTransition() const;

	bool checkStateName(const string& name) const;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	map<string, StateNode*> stateNameMap;
	StateNode* currentState = NULL;
	StateTransition* activeTransition = NULL;

	virtual void addInput(GraphPin& input);
	virtual void addOutput(GraphPin& output);
};