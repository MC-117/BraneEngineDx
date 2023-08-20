#include "ObjectGraph.h"
#include "../Console.h"
#include "../Object.h"

SerializeInstance(ObjectGraph);

ObjectGraph::ObjectGraph()
{
    beginNode = new GraphNode();
    beginNode->setName("Begin");
    graph.addNodeOutput(beginNode, new FlowPin("Entry"));
    tickNode = new GraphNode();
    tickNode->setName("Tick");
    graph.addNodeOutput(tickNode, new FlowPin("Entry"));
    graph.addNodeOutput(tickNode, new FloatPin("DeltaTime"));
    afterTickNode = new GraphNode();
    afterTickNode->setName("AfterTick");
    graph.addNodeOutput(afterTickNode, new FlowPin("Entry"));
    endNode = new GraphNode();
    endNode->setName("End");
    graph.addNodeOutput(endNode, new FlowPin("Entry"));

    graph.setName("ObjectGraph");
    graph.addNode(beginNode);
    graph.addNode(tickNode);
    graph.addNode(afterTickNode);
    graph.addNode(endNode);

    selfVariable = new ObjectRefVariable("Self");
    graph.addVariable(selfVariable);
}

void ObjectGraph::setTickable(bool value)
{
    tickable = value;
}

bool ObjectGraph::isTickable() const
{
    return tickable;
}

void ObjectGraph::setObject(Object* object)
{
    selfVariable->setValue(object);
}

Object* ObjectGraph::getObject() const
{
    return selfVariable->getValue();
}

Graph& ObjectGraph::getGraph()
{
    return graph;
}

bool ObjectGraph::init(Object* object)
{
    setObject(object);
    return true;
}

void ObjectGraph::begin()
{
    if (beginNode == NULL)
        return;
    GraphContext context;
    context.executeNode(beginNode);
}

void ObjectGraph::tick(float deltaTime)
{
    if (!tickable || tickNode == NULL)
        return;
    if (tickNode->getOutputCount() >= 2) {
        FloatPin* pin = dynamic_cast<FloatPin*>(tickNode->getOutput(1));
        if (pin) {
            pin->setValue(deltaTime);
            GraphContext context;
            context.deltaTime = deltaTime;
            context.executeNode(tickNode);
        }
    }
}

void ObjectGraph::afterTick()
{
    if (afterTickNode == NULL)
        return;
    GraphContext context;
    context.executeNode(afterTickNode);
}

void ObjectGraph::end()
{
    if (endNode == NULL)
        return;
    GraphContext context;
    context.executeNode(endNode);
}

Serializable* ObjectGraph::instantiate(const SerializationInfo& from)
{
    return new ObjectGraph();
}

bool ObjectGraph::deserialize(const SerializationInfo& from)
{
    ObjectBehavior::deserialize(from);

    graph.clear();

    const SerializationInfo* info = NULL;
    info = from.get("graph");
    if (info) {
        graph.deserialize(*info);
    }
    Ref<GraphNode> ref;
    info = from.get("beginNode");
    if (info) {
        ref.deserialize(*info);
        beginNode = ref;
    }
    info = NULL;
    info = from.get("tickNode");
    if (info) {
        ref.deserialize(*info);
        tickNode = ref;
    }
    info = NULL;
    info = from.get("afterTickNode");
    if (info) {
        ref.deserialize(*info);
        afterTickNode = ref;
    }
    info = NULL;
    info = from.get("endNode");
    if (info) {
        ref.deserialize(*info);
        endNode = ref;
    }

    selfVariable = dynamic_cast<ObjectRefVariable*>(graph.getVariable("Self"));

    return beginNode && tickNode && afterTickNode && endNode && selfVariable;
}

bool ObjectGraph::serialize(SerializationInfo& to)
{
    ObjectBehavior::serialize(to);

    SerializationInfo* info = NULL;
    info = to.add("graph");
    if (info) {
        graph.serialize(*info);
    }
    info = NULL;
    info = to.add("beginNode");
    if (info)
        Ref<GraphNode>(beginNode).serialize(*info);
    info = NULL;
    info = to.add("tickNode");
    if (info)
        Ref<GraphNode>(tickNode).serialize(*info);
    info = NULL;
    info = to.add("afterTickNode");
    if (info)
        Ref<GraphNode>(afterTickNode).serialize(*info);
    info = NULL;
    info = to.add("endNode");
    if (info)
        Ref<GraphNode>(endNode).serialize(*info);
    return true;
}

ObjectGraph::ObjectGraph(Object* object)
{
}
