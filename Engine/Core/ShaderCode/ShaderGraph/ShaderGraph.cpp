#include "ShaderGraph.h"
#include "../../Attributes/TagAttribute.h"

SerializeInstance(ShaderGraph, DEF_ATTR(Tag, "Shader"));

ShaderGraph::ShaderGraph()
{
    flag = Flag::Expression;
    bxdfPin = new ShaderBXDFPin("BXDF");
    addReturn(bxdfPin);
    addReturnNode();
}

bool ShaderGraph::addNode(GraphNode* node)
{
    if (dynamic_cast<EntryNode*>(node))
        return false;
    if (dynamic_cast<ReturnNode*>(node))
        return false;
    return Graph::addNode(node);
}

bool ShaderGraph::removeNode(GraphNode* node)
{
    if (node == returnNodes.front())
        return false;
    return Graph::removeNode(node);
}

Serializable* ShaderGraph::instantiate(const SerializationInfo& from)
{
    return new ShaderGraph();
}
