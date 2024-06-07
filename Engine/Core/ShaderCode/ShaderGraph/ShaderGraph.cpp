#include "ShaderGraph.h"
#include "../../Attributes/TagAttribute.h"

SerializeInstance(ShaderGraph, DEF_ATTR(Tag, "Shader"));

ShaderGraph::ShaderGraph()
{
    flag = Flag::Expression;
    static const Name BxDFName = "BxDF";
    bxdfPin = castTo<ShaderStructPin>(GraphPinFactory::get().construct(BxDFName, BxDFName.str()));
    if (bxdfPin == NULL)
        throw runtime_error("BxDF ShaderStructPin not found");
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
