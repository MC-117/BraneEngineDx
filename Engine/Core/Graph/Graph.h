#pragma once

#include "FlowNode.h"

class ENGINE_API GraphProxy : public GraphNode
{
	friend class Graph;
public:
	Serialize(GraphProxy, GraphNode);

	GraphProxy();

	virtual void setEntryNode(GraphNode* node);
	virtual void setReturnNode(GraphNode* node);
	GraphNode* getEntryNode() const;
	GraphNode* getReturnNode() const;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	Ref<GraphNode> entryNode;
	Ref<GraphNode> returnNode;
};

class ENGINE_API Graph : public GraphNode
{
public:
	Serialize(Graph, GraphNode);

	Graph() = default;
	virtual ~Graph();

	virtual bool addNode(GraphNode* node);

	virtual bool process(GraphContext& context);

	Flag getFlag() const;
	void setFlag(Flag flag);

	int getNodeCount() const;
	GraphNode* getNode(int index) const;

	virtual bool addVariable(GraphVariable* variable);
	int getVariableCount() const;
	GraphVariable* getVariable(const string& name) const;
	GraphVariable* getVariable(int index) const;

	virtual bool addSubGraph(Graph* graph);
	int getSubGraphCount() const;
	Graph* getSubGraph(const string& name) const;
	Graph* getSubGraph(int index) const;

	bool setEntryNode();
	EntryNode* getEntryNode() const;

	int getReturnCount() const;
	ReturnNode* getReturnNode(int index) const;

	ReturnNode* addReturnNode(ReturnNode* returnNode = NULL);

	virtual bool removeNode(GraphNode* node);
	bool removeVariable(GraphVariable* variable);
	bool removeSubGraph(Graph* graph);

	bool checkParameterName(const string& name) const;
	bool checkReturnName(const string& name) const;
	bool checkVariableName(const string& name) const;
	bool checkSubGraphName(const string& name) const;

	virtual void addParameter(GraphPin* parameterPin);
	virtual void addReturn(GraphPin* returnPin);

	virtual void removeParameter(GraphPin* parameterPin);
	virtual void removeReturn(GraphPin* parameterPin);

	virtual void clear();

	static void addNodeInput(GraphNode* node, GraphPin* input);
	static void addNodeOutput(GraphNode* node, GraphPin* output);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	vector<GraphNode*> nodes;
	EntryNode* entryNode = NULL;
	vector<ReturnNode*> returnNodes;
	unordered_map<string, GraphVariable*> variableNameMap;
	vector<GraphVariable*> variables;
	unordered_map<string, Graph*> subgraphNameMap;
	vector<Graph*> subgraphes;

	virtual void addInput(GraphPin& input);
	virtual void addOutput(GraphPin& output);
};