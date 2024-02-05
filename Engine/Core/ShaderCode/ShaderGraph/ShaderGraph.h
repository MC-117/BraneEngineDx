#pragma once
#include "ShaderNode.h"
#include "../../Graph/Graph.h"

class ShaderGraph : public Graph
{
public:
	Serialize(ShaderGraph, Graph);

	ShaderGraph();
	virtual ~ShaderGraph() = default;

	virtual bool addNode(GraphNode* node);

	virtual bool removeNode(GraphNode* node);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	ShaderBXDFPin* bxdfPin;
};
