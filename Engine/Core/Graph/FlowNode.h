#pragma once

#include "Variable.h"

class ENGINE_API EntryNode : public GraphNode
{
public:
	Serialize(EntryNode, GraphNode);

	EntryNode() = default;
	static Serializable* instantiate(const SerializationInfo& from);
};

class ENGINE_API ReturnNode : public GraphNode
{
public:
	Serialize(ReturnNode, GraphNode);

	ReturnNode() = default;
	static Serializable* instantiate(const SerializationInfo& from);
};

class ENGINE_API InOutFlowNode : public GraphNode
{
public:
	Serialize(InOutFlowNode, GraphNode);

	InOutFlowNode();

	FlowPin* getInPin();
	FlowPin* getOutPin();

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	FlowPin* inPin = NULL;
	FlowPin* outPin = NULL;
};

class ENGINE_API SetVariableNode : public InOutFlowNode
{
public:
	Serialize(SetVariableNode, InOutFlowNode);

	SetVariableNode();

	ValuePin* getValuePin();
	GraphVariable* getVariable();

	void init(GraphVariable* variable);

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	ValuePin* valuePin = NULL;
	Ref<GraphVariable> variable;
};

class ENGINE_API SequenceNode : public GraphNode
{
public:
	Serialize(SequenceNode, GraphNode);

	SequenceNode();

	void addFlowPin();

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	FlowPin* inFlowPin = NULL;
};

class ENGINE_API BranchNode : public GraphNode
{
public:
	Serialize(BranchNode, GraphNode);

	BranchNode();

	virtual bool flowControl(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	FlowPin* inPin = NULL;
	BoolPin* conditionPin = NULL;
	FlowPin* truePin = NULL;
	FlowPin* falsePin = NULL;
};

class ENGINE_API LoopNode : public GraphNode
{
public:
	Serialize(LoopNode, GraphNode);

	LoopNode();

	virtual bool flowControl(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	FlowPin* inPin = NULL;
	FlowPin* loopPin = NULL;
	FlowPin* outPin = NULL;
};

class ENGINE_API BreakNode : public GraphNode
{
public:
	Serialize(BreakNode, GraphNode);

	BreakNode();

	virtual bool flowControl(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	FlowPin* breakPin = NULL;
};

class ENGINE_API ForLoopNode : public GraphNode
{
public:
	Serialize(ForLoopNode, GraphNode);

	ForLoopNode();

	virtual bool flowControl(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	FlowPin* inPin = NULL;
	IntPin* countPin = NULL;
	FlowPin* loopPin = NULL;
	IntPin* indexPin = NULL;
	FlowPin* outPin = NULL;
};