#pragma once

#include "../Graph.h"

class ENGINE_API StateTransition : public Base
{
	friend class StateNode;
	friend class StateTransitionPin;
public:
	Serialize(StateTransition, Base);

	StateTransition();

	virtual void init(StateNode* fromNode, StateNode* toNode);

	virtual void onBeginTransition(GraphContext& context);
	virtual void onTransition(GraphContext& context);
	virtual void onEndTransition(GraphContext& context);

	StateNode* getState() const;
	StateNode* getNextState() const;
	virtual bool canTransition() const;
	virtual bool completeTransition() const;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	StateNode* state;
	Ref<StateNode> nextState;
};

class ENGINE_API StateGraphTransition : public StateTransition
{
	friend class StateGraphTransitionPin;
public:
	Serialize(StateGraphTransition, StateTransition);

	StateGraphTransition();

	virtual void init(StateNode* fromNode, StateNode* toNode);

	Graph* getGraph();
	const Graph* getGraph() const;

	virtual bool canTransition() const;
	virtual bool completeTransition() const;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	Graph transitionGraph;
	BoolPin* transitionPin = NULL;
};

class ENGINE_API StateTransitionPin : public FlowPin
{
	friend class StateNode;
public:
	Serialize(StateTransitionPin, FlowPin);

	StateTransitionPin();

	bool switchTransitionType(Serialization& type);

	virtual bool isConnectable(GraphPin* pin) const;
	virtual bool connect(GraphPin* pin);
	virtual bool disconnect(GraphPin* pin);

	bool isConnectedWith(const StateNode* state) const;
	
	virtual void clearTransition();

	int getTransitionCount() const;
	StateTransition* getTransition(int index) const;
	StateTransition* getTransition(const StateTransitionPin* targetPin) const;

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	Serialization* transitionType = &StateTransition::StateTransitionSerialization::serialization;
	vector<StateTransition*> transitions;
};

class ENGINE_API StateNode : public GraphNode
{
public:
	Serialize(StateNode, GraphNode);

	StateNode();

	virtual void setTransitionPin(StateTransitionPin* pin);
	StateTransitionPin* getTransitionPin() const;

	int getTransitionCount() const;
	StateTransition* getTransition(int index) const;

	bool isConnectedWith(const StateNode* state) const;

	StateNode* getNextState() const;
	StateTransition* getActiveTransition() const;

	virtual void onEnterState(GraphContext& context);
	virtual void onUpdateState(GraphContext& context);
	virtual void onTransitState(GraphContext& context);
	virtual void onLeaveState(GraphContext& context);

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	StateTransitionPin* transitionPin;
	StateNode* nextState = NULL;
	StateTransition* activeTransition = NULL;

	virtual void addInput(GraphPin& input);
	virtual void addOutput(GraphPin& output);
};

class ENGINE_API StateEntryTransitionPin : public StateTransitionPin
{
public:
	Serialize(StateEntryTransitionPin, StateTransitionPin);

	StateEntryTransitionPin() = default;
	virtual ~StateEntryTransitionPin() = default;

	virtual bool connect(GraphPin* pin);

	static Serializable* instantiate(const SerializationInfo& from);
};

class StateEntryNode : public EntryNode
{
public:
	Serialize(StateEntryNode, EntryNode);

	StateEntryNode();
	virtual ~StateEntryNode() = default;

	StateTransitionPin* getTransitionPin() const;

	StateNode* getNextState() const;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	StateEntryTransitionPin* transitionPin = NULL;
	StateNode* nextState = NULL;
};