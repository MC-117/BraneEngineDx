#include "Graph.h"

#include "GraphCodeGeneration.h"
#include "../Console.h"

SerializeInstance(GraphProxy);

GraphProxy::GraphProxy()
{
}

void GraphProxy::setEntryNode(GraphNode* node)
{
	if (node) {
		entryNode = node;
		clearInputs();
		Graph* graph = dynamic_cast<Graph*>(node);
		if (graph)
			flag = graph->getFlag();
		vector<GraphPin*>& pins = graph ? node->inputs : node->outputs;
		for each (auto output in pins)
		{
			Serializable* serialize = Serialization::clone(*output);
			if (serialize == NULL)
				throw runtime_error("clone failed");
			GraphPin* pin = dynamic_cast<GraphPin*>(serialize);
			if (pin == NULL) {
				delete serialize;
				throw runtime_error("clone failed");
			}
			addInput(*pin);
		}
	}
	else {
		clearInputs();
	}
}

void GraphProxy::setReturnNode(GraphNode* node)
{
	if (node) {
		returnNode = node;
		clearOutputs();
		Graph* graph = dynamic_cast<Graph*>(node);
		if (graph)
			flag = graph->getFlag();
		vector<GraphPin*>& pins = graph ? node->outputs : node->inputs;
		for each (auto input in pins)
		{
			Serializable* serialize = Serialization::clone(*input);
			if (serialize == NULL)
				throw runtime_error("clone failed");
			GraphPin* pin = dynamic_cast<GraphPin*>(serialize);
			if (pin == NULL) {
				delete serialize;
				throw runtime_error("clone failed");
			}
			addOutput(*pin);
		}
	}
	else {
		clearOutputs();
	}
}

GraphNode* GraphProxy::getEntryNode() const
{
	return entryNode;
}

GraphNode* GraphProxy::getReturnNode() const
{
	return returnNode;
}

bool GraphProxy::process(GraphContext& context)
{
	GraphContext localContext;
	localContext.deltaTime = context.deltaTime;

	GraphNode* pEntryNode = entryNode;
	GraphNode* pReturnNode = returnNode;

	bool success = true;

	if (pEntryNode) {
		Graph* graph = dynamic_cast<Graph*>(pEntryNode);
		vector<GraphPin*>& pins = graph ? pEntryNode->inputs : pEntryNode->outputs;
		for (int i = 0; i < inputs.size(); i++) {
			ValuePin* input = dynamic_cast<ValuePin*>(getInput(i));
			ValuePin* pin = dynamic_cast<ValuePin*>(pins[i]);
			if (input && pin) {
				pin->assign(input);
			}
		}
	}

	if (pEntryNode && flag == Flag::Statment) {
		localContext.executeNode(entryNode);
	}
	else if (pReturnNode) {
		localContext.executeNode(pReturnNode);
	}
	else {
		return false;
	}

	if (pReturnNode) {
		Graph* graph = dynamic_cast<Graph*>(pReturnNode);
		vector<GraphPin*>& pins = graph ? pReturnNode->outputs : pReturnNode->inputs;
		for (int i = 0; i < outputs.size(); i++) {
			ValuePin* pin = dynamic_cast<ValuePin*>(pins[i]);
			ValuePin* output = dynamic_cast<ValuePin*>(getOutput(i));
			if (pin && output) {
				output->assign(pin);
			}
		}
	}

	return success;
}

Serializable* GraphProxy::instantiate(const SerializationInfo& from)
{
	GraphProxy* node = new GraphProxy();
	const SerializationInfo* inputInfos = from.get("inputs");
	if (inputInfos) {
		for each (const auto & info in inputInfos->sublists)
		{
			Serializable* serializable = info.serialization->instantiate(info);
			if (serializable == NULL)
				continue;
			GraphPin* pin = dynamic_cast<GraphPin*>(serializable);
			if (!pin) {
				delete serializable;
				continue;
			}
			node->addInput(*pin);
		}
	}
	const SerializationInfo* outputInfos = from.get("outputs");
	if (outputInfos) {
		for each (const auto & info in outputInfos->sublists)
		{
			Serializable* serializable = info.serialization->instantiate(info);
			if (serializable == NULL)
				continue;
			GraphPin* pin = dynamic_cast<GraphPin*>(serializable);
			if (!pin) {
				delete serializable;
				continue;
			}
			node->addOutput(*pin);
		}
	}
	return node;
}

bool GraphProxy::deserialize(const SerializationInfo& from)
{
	if (!GraphNode::deserialize(from))
		return false;
	const SerializationInfo* entryInfo = from.get("entryNode");
	if (entryInfo)
		entryNode.deserialize(*entryInfo);
	const SerializationInfo* returnInfo = from.get("returnNode");
	if (returnInfo)
		returnNode.deserialize(*returnInfo);
	return true;
}

bool GraphProxy::serialize(SerializationInfo& to)
{
	if (!GraphNode::serialize(to))
		return false;
	SerializationInfo* entryInfo = to.add("entryNode");
	if (entryInfo)
		entryNode.serialize(*entryInfo);
	SerializationInfo* returnInfo = to.add("returnNode");
	if (returnInfo)
		returnNode.serialize(*returnInfo);
	return true;
}

SerializeInstance(Graph);

Graph::~Graph()
{
	clear();
}

bool Graph::addNode(GraphNode* node)
{
	if (!node)
		return false;
	EntryNode* entryNode = dynamic_cast<EntryNode*>(node);
	if (entryNode)
		return setEntryNode();
	ReturnNode* returnNode = dynamic_cast<ReturnNode*>(node);
	if (returnNode) {
		addReturnNode(returnNode);
		return true;
	}
	nodes.push_back(node);
	return true;
}

bool Graph::process(GraphContext& context)
{
	bool success = true;

	if (entryNode) {
		for (int i = 0; i < inputs.size(); i++) {
			ValuePin* input = dynamic_cast<ValuePin*>(getInput(i));
			ValuePin* output = dynamic_cast<ValuePin*>(entryNode->getOutput(i));
			if (input && output) {
				output->assign(input);
			}
		}
	}

	ReturnNode* returnedNode = NULL;
	if (entryNode && flag == Flag::Statment) {
		context.executeNode(entryNode);
	}
	else if (returnNodes.size() == 1) {
		returnedNode = returnNodes[0];
		context.executeNode(returnedNode);
	}
	else {
		success = false;
	}

	if (returnedNode) {
		for (int i = 0; i < outputs.size(); i++) {
			ValuePin* input = dynamic_cast<ValuePin*>(returnedNode->getInput(i));
			ValuePin* output = dynamic_cast<ValuePin*>(getOutput(i));
			if (input && output) {
				output->assign(input);
			}
		}
	}
	return success;
}

bool Graph::generateParameter(GraphCodeGenerationContext& context)
{
	return true;
}

bool Graph::solveAndGenerateOutput(GraphCodeGenerationContext& context)
{
	return true;
}

bool Graph::generateGlobalVariables(GraphCodeGenerationContext& context)
{
	bool success = true;
	for (auto variable : variables) {
		if (variable->isGlobalVariable())
			success &= variable->generate(context);
	}
	return success;
}

bool Graph::generate(GraphCodeGenerationContext& context)
{
	bool success = true;
	ICodeScopeBackend& backend = context.getBackend();
	CodeFunctionSignature signature(getFunctionName());
	generateSignature(context, signature);

	int valueOutputCount = 0;
	ValuePin* firstValuePin = NULL;
	for (int i = 0; i < getOutputCount(); i++) {
		if (ValuePin* pin = dynamic_cast<ValuePin*>(getOutput(i))) {
			if (firstValuePin == NULL)
				firstValuePin = pin;
			valueOutputCount++;
		}
	}
			
	for (auto pin : inputs) {
		if (const GraphCodeHeaderFileAttribute* headerFile = pin->getSerialization().getAttribute<GraphCodeHeaderFileAttribute>())
			context.addIncludeFile(headerFile->getPath());
	}
	for (auto pin : outputs) {
		if (const GraphCodeHeaderFileAttribute* headerFile = pin->getSerialization().getAttribute<GraphCodeHeaderFileAttribute>())
			context.addIncludeFile(headerFile->getPath());
	}

	if (valueOutputCount) {
		if (valueOutputCount == 1) {
			if (firstValuePin) {
				CodeSymbolDefinition& definition = signature.outputs.emplace_back();
				definition.type = firstValuePin->getVariableType();
				Name pinName = firstValuePin->getName();
				definition.name = pinName;
			}
		}
		else {
			for (int i = 0; i < getOutputCount(); i++) {
				if (ValuePin* pin = dynamic_cast<ValuePin*>(getOutput(i))) {
					CodeSymbolDefinition& definition = signature.outputs.emplace_back();
					definition.type = pin->getVariableType();
					Name pinName = pin->getName();
					context.assignParameter(pin, pinName);
					definition.name = pinName;
				}
			}
		}
	}

	generateGlobalVariables(context);

	ICodeScopeBackend* funcScope = backend.declareFunction(signature);

	context.pushSubscopeBackend(funcScope);
	
	for (auto variable : variables) {
		if (!variable->isGlobalVariable())
			success &= variable->generate(context);
	}

	for (auto subgraph : subgraphes) {
		subgraph->generateStatement(context);
	}
	
	if (entryNode && flag == Flag::Statment) {
		context.generateCodeFromNode(entryNode);
	}
	else if (returnNodes.size() == 1) {
		context.generateCodeFromNode(returnNodes[0]);
	}
	else {
		success = false;
	}

	context.popSubscopeBackend(funcScope);
	
	return success;
}

Graph::Flag Graph::getFlag() const
{
	return flag;
}

void Graph::setFlag(Flag flag)
{
	this->flag = flag;
}

int Graph::getNodeCount() const
{
	return nodes.size();
}

GraphNode* Graph::getNode(int index) const
{
	return nodes[index];
}

bool Graph::addVariable(GraphVariable* variable)
{
	if (getVariable(variable->name))
		return false;
	variables.push_back(variable);
	variableNameMap[variable->name] = variable;
	return true;
}

int Graph::getVariableCount() const
{
	return variables.size();
}

GraphVariable* Graph::getVariable(const Name& name) const
{
	auto iter = variableNameMap.find(name);
	if (iter == variableNameMap.end())
		return NULL;
	return iter->second;
}

GraphVariable* Graph::getVariable(int index) const
{
	return variables[index];
}

bool Graph::addSubGraph(Graph* graph)
{
	if (getSubGraph(graph->name))
		return false;
	subgraphes.push_back(graph);
	subgraphNameMap[graph->name] = graph;
	return true;
}

int Graph::getSubGraphCount() const
{
	return subgraphes.size();
}

Graph* Graph::getSubGraph(const Name& name) const
{
	auto iter = subgraphNameMap.find(name);
	if (iter == subgraphNameMap.end())
		return NULL;
	return iter->second;
}

Graph* Graph::getSubGraph(int index) const
{
	return subgraphes[index];
}

bool Graph::setEntryNode()
{
	if (entryNode)
		return false;
	entryNode = new EntryNode();
	entryNode->setName("Entry");
	for each (auto input in inputs)
	{
		Serializable* serialize = Serialization::clone(*input);
		if (serialize == NULL)
			throw runtime_error("clone failed");
		GraphPin* pin = dynamic_cast<GraphPin*>(serialize);
		if (pin == NULL) {
			delete serialize;
			throw runtime_error("clone failed");
		}
		entryNode->addOutput(*pin);
	}
	nodes.push_back(entryNode);
	return true;
}

EntryNode* Graph::getEntryNode() const
{
	return entryNode;
}

int Graph::getReturnCount() const
{
	return returnNodes.size();
}

ReturnNode* Graph::getReturnNode(int index) const
{
	return returnNodes[index];
}

ReturnNode* Graph::addReturnNode(ReturnNode* returnNode)
{
	if (returnNode == NULL)
		returnNode = new ReturnNode();
	returnNode->clearPins();
	returnNode->setName("Return");
	for each (auto output in outputs)
	{
		Serializable* serialize = Serialization::clone(*output);
		if (serialize == NULL)
			throw runtime_error("clone failed");
		GraphPin* pin = dynamic_cast<GraphPin*>(serialize);
		if (pin == NULL) {
			delete serialize;
			throw runtime_error("clone failed");
		}
		returnNode->addInput(*pin);
	}
	nodes.push_back(returnNode);
	returnNodes.push_back(returnNode);
	return returnNode;
}

bool Graph::removeNode(GraphNode* node)
{
	if (node == NULL || node == entryNode)
		return false;
	bool found = false;
	for (auto b = nodes.begin(), e = nodes.end(); b != e; b++)
	{
		if (*b == node) {
			nodes.erase(b);
			found = true;
			break;
		}
	}
	ReturnNode* retNode = dynamic_cast<ReturnNode*>(node);
	if (retNode) {
		for (auto b = returnNodes.begin(), e = returnNodes.end(); b != e; b++)
		{
			if (*b == node) {
				returnNodes.erase(b);
				found = true;
				break;
			}
		}
	}
	delete node;
	return found;
}

bool Graph::removeVariable(GraphVariable* variable)
{
	if (variable == NULL)
		return false;
	bool found = false;
	for (auto b = variables.begin(), e = variables.end(); b != e; b++)
	{
		if (*b == variable) {
			variables.erase(b);
			found = true;
			break;
		}
	}
	if (found) {
		variableNameMap.erase(variable->getName());
		delete variable;
	}
	return found;
}

bool Graph::removeSubGraph(Graph* graph)
{
	if (graph == NULL)
		return false;
	bool found = false;
	for (auto b = subgraphes.begin(), e = subgraphes.end(); b != e; b++)
	{
		if (*b == graph) {
			subgraphes.erase(b);
			found = true;
			break;
		}
	}
	if (found) {
		subgraphNameMap.erase(graph->getName());
		delete graph;
	}
	return found;
}

bool Graph::checkParameterName(const string& name) const
{
	for each (auto pin in inputs)
	{
		if (pin->getName() == name)
			return false;
	}
	return true;
}

bool Graph::checkReturnName(const string& name) const
{
	for each (auto pin in outputs)
	{
		if (pin->getName() == name)
			return false;
	}
	return true;
}

bool Graph::checkVariableName(const string& name) const
{
	return !name.empty() && variableNameMap.find(name) == variableNameMap.end();
}

bool Graph::checkSubGraphName(const string& name) const
{
	return !name.empty() && subgraphNameMap.find(name) == subgraphNameMap.end();
}

void Graph::addParameter(GraphPin* parameterPin)
{
	if (parameterPin != NULL) {
		setEntryNode();
		addInput(*parameterPin);
	}
}

void Graph::addReturn(GraphPin* returnPin)
{
	if (returnPin != NULL)
		addOutput(*returnPin);
}

void Graph::removeParameter(GraphPin* parameterPin)
{
}

void Graph::removeReturn(GraphPin* parameterPin)
{
}

void Graph::clear()
{
	for each (auto state in nodes)
	{
		delete state;
	}
	nodes.clear();
	for each (auto variable in variables)
	{
		delete variable;
	}
	variables.clear();
	variableNameMap.clear();
	returnNodes.clear();
	entryNode = NULL;
}

void Graph::addNodeInput(GraphNode* node, GraphPin* input)
{
	node->addInput(*input);
}

void Graph::addNodeOutput(GraphNode* node, GraphPin* output)
{
	node->addOutput(*output);
}

Serializable* Graph::instantiate(const SerializationInfo& from)
{
	Graph* graph = new Graph();
	const SerializationInfo* inputInfos = from.get("inputs");
	if (inputInfos) {
		for (const auto & info : inputInfos->sublists)
		{
			Serializable* serializable = info.serialization->instantiate(info);
			if (serializable == NULL)
				continue;
			GraphPin* pin = dynamic_cast<GraphPin*>(serializable);
			if (!pin) {
				delete serializable;
				continue;
			}
			graph->GraphNode::addInput(*pin);
		}
	}
	const SerializationInfo* outputInfos = from.get("outputs");
	if (outputInfos) {
		for (const auto & info : outputInfos->sublists)
		{
			Serializable* serializable = info.serialization->instantiate(info);
			if (serializable == NULL)
				continue;
			GraphPin* pin = dynamic_cast<GraphPin*>(serializable);
			if (!pin) {
				delete serializable;
				continue;
			}
			graph->GraphNode::addOutput(*pin);
		}
	}
	return graph;
}

bool Graph::deserialize(const SerializationInfo& from)
{
	if (!GraphNode::deserialize(from))
		return false;
	int flagInt = (unsigned char)flag;
	if (from.get("flag", flagInt))
		flag = (Flag)flagInt;
	const SerializationInfo* nodeInfos = from.get("nodes");
	if (nodeInfos) {
		int processedReturnNodeIndex = 0;
		bool processedEntryNode = false;
		for each (const auto & info in nodeInfos->sublists)
		{
			if (info.serialization) {
				if (!processedEntryNode && entryNode &&
					info.serialization == &EntryNode::EntryNodeSerialization::serialization ||
					info.serialization->isChildOf(EntryNode::EntryNodeSerialization::serialization)) {
					entryNode->deserialize(info);
					processedEntryNode = true;
					continue;
				}
				if (processedReturnNodeIndex < returnNodes.size() &&
					info.serialization == &ReturnNode::ReturnNodeSerialization::serialization ||
					info.serialization->isChildOf(ReturnNode::ReturnNodeSerialization::serialization)) {
					ReturnNode* returnNode = returnNodes[processedReturnNodeIndex];
					returnNode->deserialize(info);
					processedReturnNodeIndex++;
					continue;
				}
				Serializable* serializable = info.serialization->instantiate(info);
				if (serializable) {
					GraphNode* node = dynamic_cast<GraphNode*>(serializable);
					if (node) {
						node->deserialize(info);
						EntryNode* pEntryNode = dynamic_cast<EntryNode*>(node);
						ReturnNode* pReturnNode = dynamic_cast<ReturnNode*>(node);
						if (pEntryNode) {
							nodes.push_back(node);
							entryNode = pEntryNode;
						}
						else if (pReturnNode) {
							nodes.push_back(node);
							returnNodes.push_back(pReturnNode);
						}
						else {
							if (!addNode(node)) {
								Console::error("Graph: add node(%s) failed", node->getDisplayName().c_str());
								delete node;
							}
						}
					}
					else
						delete serializable;
				}
			}
		}
	}
	const SerializationInfo* variableInfos = from.get("variables");
	if (variableInfos) {
		for each (const auto & info in variableInfos->sublists)
		{
			GraphVariable* variable = getVariable(info.name);
			if (variable) {
				if (&variable->getSerialization() != info.serialization) {
					removeVariable(variable);
					variable = NULL;
				}
			}
			else if (info.serialization) {
				Serializable* serializable = info.serialization->instantiate(info);
				if (serializable) {
					variable = dynamic_cast<GraphVariable*>(serializable);
					if (variable) {
						if (!addVariable(variable)) {
							Console::error("Graph: add variable(%s) failed", variable->name.c_str());
							delete variable;
						}
					}
					else
						delete serializable;
				}
			}
			if (variable) {
				variable->deserialize(info);
			}
		}
	}
	const SerializationInfo* subgraphInfos = from.get("subgraphes");
	if (subgraphInfos) {
		for each (const auto & info in subgraphInfos->sublists)
		{
			Graph* subgraph = getSubGraph(info.name);
			if (subgraph) {
				if (&subgraph->getSerialization() != info.serialization) {
					removeSubGraph(subgraph);
					subgraph = NULL;
				}
			}
			if (subgraph) {
				subgraph->deserialize(info);
			}
			else if (info.serialization) {
				Serializable* serializable = info.serialization->instantiate(info);
				if (serializable) {
					subgraph = dynamic_cast<Graph*>(serializable);
					if (subgraph) {
						subgraph->deserialize(info);
						if (!addSubGraph(subgraph)) {
							Console::error("Graph: add subgraph(%s) failed", subgraph->name.c_str());
							delete subgraph;
						}
					}
					else
						delete serializable;
				}
			}
		}
	}
	return true;
}

bool Graph::serialize(SerializationInfo& to)
{
	if (!GraphNode::serialize(to))
		return false;

	int flagInt = (unsigned char)flag;
	to.set("flag", flagInt);

	SerializationInfo* nodeInfos = to.add("nodes");
	if (nodeInfos) {
		int i = 0;
		for each (auto node in nodes)
		{
			SerializationInfo* info = nodeInfos->add(to_string(i));
			if (info) {
				node->serialize(*info);
			}
			i++;
		}
	}
	SerializationInfo* variableInfos = to.add("variables");
	if (variableInfos) {
		for each (auto variable in variables)
		{
			SerializationInfo* info = variableInfos->add(variable->getName());
			if (info) {
				variable->serialize(*info);
			}
		}
	}
	SerializationInfo* subgraphInfos = to.add("subgraphes");
	if (subgraphInfos) {
		for each (auto subgraph in subgraphes)
		{
			SerializationInfo* info = subgraphInfos->add(subgraph->getName().c_str());
			if (info) {
				subgraph->serialize(*info);
			}
		}
	}
	return true;
}

void Graph::addInput(GraphPin& input)
{
	Serializable* serialize = Serialization::clone(input);
	if (serialize == NULL)
		return;
	GraphPin* pin = dynamic_cast<GraphPin*>(serialize);
	if (pin == NULL) {
		delete serialize;
		throw runtime_error("clone failed");
	}
	GraphNode::addInput(input);
	if (entryNode)
		entryNode->addOutput(*pin);
}

void Graph::addOutput(GraphPin& output)
{
	GraphNode::addOutput(output);
	for each (ReturnNode* node in returnNodes)
	{
		Serializable* serialize = Serialization::clone(output);
		if (serialize == NULL)
			throw runtime_error("clone failed");
		GraphPin* pin = dynamic_cast<GraphPin*>(serialize);
		if (pin == NULL) {
			delete serialize;
			throw runtime_error("clone failed");
		}
		node->addInput(*pin);
	}
}

void Graph::generateSignature(GraphCodeGenerationContext& context, CodeFunctionSignature& signature)
{
	for (int i = 0; i < getInputCount(); i++) {
		ValuePin* pin = dynamic_cast<ValuePin*>(entryNode->getOutput(i));
		if (pin) {
			CodeSymbolDefinition& definition = signature.parameters.emplace_back();
			definition.type = pin->getVariableType();
			Name pinName = pin->getName();
			context.assignParameter(pin, pinName);
			definition.name = pinName;
		}
	}
}
