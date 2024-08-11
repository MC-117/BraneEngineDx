#include "Node.h"
#include "GraphCodeGeneration.h"
#include "GraphNodeEditor.h"
#include "../Attributes/TagAttribute.h"
#include "../Utility/NameUtility.h"

void GraphContext::execute()
{
    while (!nodeStack.empty()) {
        GraphNode* curNode = nodeStack.top();
        nodeStack.pop();
        curNode->solveState(*this);
    }
}

void GraphContext::executeNode(GraphNode* node)
{
    GraphNode* endNode = nodeStack.empty() ? NULL : nodeStack.top();
    nodeStack.push(node);
    while (!nodeStack.empty()) {
        GraphNode* curNode = nodeStack.top();
        if (endNode == curNode)
            break;
        nodeStack.pop();
        curNode->solveState(*this);
    }
}

SerializeInstance(GraphPin, DEF_ATTR(Tag, "Graph"));

string GraphPin::getName() const
{
    return name;
}

string GraphPin::getDisplayName() const
{
    return displayName.empty() ? name : displayName;
}

Color GraphPin::getPinColor() const
{
    return Color(128, 128, 128);
}

GraphPinShape GraphPin::getPinShape() const
{
    return GraphPinShape::Circle;
}

GraphNode* GraphPin::getNode() const
{
    return node;
}

bool GraphPin::isOutputPin() const
{
    return isOutput;
}

void GraphPin::setName(const string& name)
{
    this->name = name;
}

void GraphPin::setDisplayName(const string& name)
{
    displayName = name;
}

bool GraphPin::isWildcard() const
{
    return false;
}

bool GraphPin::isConnectable(const GraphPin* pin) const
{
    bool connectable = pin != NULL && pin->isOutput != isOutput && pin->node != node;
    bool this_Wildcard = isWildcard();
    bool that_Wildcard = pin->isWildcard();
    if (this_Wildcard) {
        connectable &= isWildcardAcceptable(pin);
    }
    if (that_Wildcard) {
        connectable &= pin->isWildcardAcceptable(this);
    }
    if (!this_Wildcard && !that_Wildcard) {
        connectable = &getSerialization() == &pin->getSerialization();
    }
    return connectable;
}

GraphPin* GraphPin::getConnectedPin()
{
    return connectedPin;
}

Ref<GraphPin>& GraphPin::getConnectedPinRef()
{
    return connectedPin;
}

bool GraphPin::connect(GraphPin* pin)
{
    GraphPin* otherPin = connectedPin;
    if (otherPin == pin)
        return true;
    if (!isConnectable(pin))
        return false;
    if (!isOutput) {
        disconnect(otherPin);
    }
    connectedPin = pin;
    pin->connect(this);
    return true;
}

bool GraphPin::disconnect(GraphPin* pin)
{
    if (pin == NULL)
        return false;
    bool success = false;
    GraphPin* otherPin = connectedPin;
    if (otherPin == NULL)
        return true;
    if (pin == otherPin) {
        connectedPin = NULL;
        success = true;
    }
    success |= pin->disconnect(this);
    return success;
}

bool GraphPin::process(GraphContext& context)
{
    return true;
}

bool GraphPin::generate(GraphCodeGenerationContext& context)
{
    return true;
}

Serializable* GraphPin::instantiate(const SerializationInfo& from)
{
    return nullptr;
}

bool GraphPin::deserialize(const SerializationInfo& from)
{
    Base::deserialize(from);
    const SerializationInfo* info = from.get("connectedPin");
    if (info) {
        connectedPin.deserialize(*info);
    }
    return true;
}

bool GraphPin::serialize(SerializationInfo& to)
{
    Base::serialize(to);
    to.name = name;
    SerializationInfo* info = to.add("connectedPin");
    if (info) {
        connectedPin.serialize(*info);
    }
    return true;
}

GraphPin::GraphPin(const string& name) : Base(), name(name)
{
}

bool GraphPin::isWildcardAcceptable(const GraphPin* pin) const
{
    return true;
}

GraphPinFactory& GraphPinFactory::get()
{
    static GraphPinFactory factory;
    return factory;
}

void GraphPinFactory::registerType(const Name& type, const Serialization& serialization)
{
    if (type.isNone())
        throw runtime_error("None type is invalid");
    if (!serialization.isChildOf(GraphPin::GraphPinSerialization::serialization))
        throw runtime_error("This is not serialization of GraphPin");
    auto iter = codeTypeToGraphPinSerialization.find(type);
    if (iter != codeTypeToGraphPinSerialization.end())
        throw runtime_error("This code type already been bound with factory");
    codeTypeToGraphPinSerialization.emplace(type, &serialization);
    
}

const Serialization* GraphPinFactory::getFactory(const Name& type)
{
    auto iter = codeTypeToGraphPinSerialization.find(type);
    if (iter == codeTypeToGraphPinSerialization.end())
        return NULL;
    return iter->second;
}

GraphPin* GraphPinFactory::construct(const Name& type, const string& name)
{
    const Serialization* factory = getFactory(type);
    if (factory == NULL)
        return NULL;
    SerializationInfo info;
    info.name = name;
    Serializable* serializable = factory->instantiate(info);
    GraphPin* pin = dynamic_cast<GraphPin*>(serializable);
    if (pin == NULL)
        delete serializable;
    return pin;
}

GraphPinCodeTypeAttribute::GraphPinCodeTypeAttribute(const Name& type)
    : Attribute("GraphPinCodeType", true)
    , typeFinal(type)
{
}

bool GraphPinCodeTypeAttribute::checkType(const Name& type) const
{
    return type == typeFinal;
}

const Name& GraphPinCodeTypeAttribute::getType() const
{
    return typeFinal;
}

void GraphPinCodeTypeAttribute::resolve(Attribute* sourceAttribute, Serialization& serialization)
{
    if (sourceAttribute) {
        const GraphPinCodeTypeAttribute* attr = sourceAttribute->cast<GraphPinCodeTypeAttribute>();
        typeFinal = attr->typeFinal;
    }
}

void GraphPinCodeTypeAttribute::finalize(Serialization& serialization)
{
    GraphPinFactory::get().registerType(typeFinal, serialization);
}

GraphCodeHeaderFileAttribute::GraphCodeHeaderFileAttribute(const Name& path)
    : Attribute("GraphCodeHeaderFile", true)
    , path(path)
{
}

const Name& GraphCodeHeaderFileAttribute::getPath() const
{
    return path;
}

void GraphCodeHeaderFileAttribute::resolve(Attribute* sourceAttribute, Serialization& serialization)
{
    if (sourceAttribute) {
        const GraphCodeHeaderFileAttribute* attr = sourceAttribute->cast<GraphCodeHeaderFileAttribute>();
        path = attr->path;
    }
}

SerializeInstance(FlowPin);

FlowPin::FlowPin(const string& name) : GraphPin(name)
{
}

Color FlowPin::getPinColor() const
{
    return Color(255, 255, 255);
}

GraphPinShape FlowPin::getPinShape() const
{
    return GraphPinShape::Flow;
}

bool FlowPin::connect(GraphPin* pin)
{
    GraphPin* otherPin = connectedPin;
    if (otherPin == pin)
        return true;
    if (!isConnectable(pin))
        return false;

    disconnect(otherPin);

    connectedPin = pin;
    pin->connect(this);
    return true;
}

bool FlowPin::process(GraphContext& context)
{
    if (isOutput) {
        FlowPin* nextPin = dynamic_cast<FlowPin*>((GraphPin*)connectedPin);
        if (nextPin && nextPin->node)
            context.nodeStack.push(nextPin->node);
    }
    return true;
}

bool FlowPin::generate(GraphCodeGenerationContext& context)
{
    if (isOutput) {
        FlowPin* nextPin = dynamic_cast<FlowPin*>((GraphPin*)connectedPin);
        if (nextPin && nextPin->node)
            context.nodeStack.push(nextPin->node);
    }
    return true;
}

Serializable* FlowPin::instantiate(const SerializationInfo& from)
{
    return new FlowPin(from.name);
}

SerializeInstance(GraphNode, DEF_ATTR(Tag, "Graph"));

GraphNode::GraphNode() : Base()
{
}

GraphNode::~GraphNode()
{
    clearPins();
}

void GraphNode::setName(const Name& name)
{
    this->name = name;
}

Name GraphNode::getName() const
{
    return name;
}

Name GraphNode::getDisplayName() const
{
    return displayName.empty() ? name : displayName;
}

Color GraphNode::getNodeColor() const
{
    return nodeColor;
}

int GraphNode::getInputCount() const
{
    return inputs.size();
}

GraphPin* GraphNode::getInput(int index) const
{
    return inputs[index];
}

GraphPin* GraphNode::getInput(const string& name) const
{
    for each (GraphPin * pin in inputs)
    {
        if (pin->name == name)
            return pin;
    }
    return NULL;
}

int GraphNode::getOutputCount() const
{
    return outputs.size();
}

GraphPin* GraphNode::getOutput(int index) const
{
    return outputs[index];
}

GraphPin* GraphNode::getOutput(const string& name) const
{
    for each (GraphPin * pin in outputs)
    {
        if (pin->name == name)
            return pin;
    }
    return NULL;
}

void GraphNode::getRootStates(unordered_set<GraphNode*>& states)
{
    if (inputs.empty())
        states.insert(this);
    else for each (auto pin in inputs)
    {
        if (pin->node)
            pin->node->getRootStates(states);
    }
}

bool GraphNode::solveInput(GraphContext& context)
{
    bool success = true;
    for (auto input : inputs) {
        FlowPin* flowPin = dynamic_cast<FlowPin*>(input);
        if (flowPin)
            continue;
        GraphPin* output = input->getConnectedPin();
        if (output != NULL && output->node != NULL) {
            if (output->node->flag == Flag::Expression) {
                success &= output->node->solveState(context);
            }
        }
        success &= input->process(context);
    }
    return success;
}

bool GraphNode::flowControl(GraphContext& context)
{
    bool success = true;
    for (int i = outputs.size() - 1; i >= 0; i--)
    {
        success &= outputs[i]->process(context);
    }
    return success;
}

bool GraphNode::solveState(GraphContext& context)
{
    bool success = solveInput(context);
    success &= process(context);
    success &= flowControl(context);
    return success;
}

bool GraphNode::process(GraphContext& context)
{
    return true;
}

Name GraphNode::getFunctionName() const
{
    const CodeFunctionNameAttribute* attr = getSerialization().getAttribute<CodeFunctionNameAttribute>();
    return attr ? attr->funcName : getName();
}

bool GraphNode::generateParameter(GraphCodeGenerationContext& context)
{
    bool success = true;
    for (auto pin : inputs) {
        FlowPin* flowPin = dynamic_cast<FlowPin*>(pin);
        if (flowPin)
            continue;
        GraphPin* output = pin->getConnectedPin();
        if (output != NULL && output->node != NULL) {
            if (output->node->flag == Flag::Expression) {
                success &= output->node->generateStatement(context);
            }
        }
        success &= pin->generate(context);
    }
    return success;
}

bool GraphNode::solveAndGenerateOutput(GraphCodeGenerationContext& context)
{
    bool success = true;
    for (int i = outputs.size() - 1; i >= 0; i--) {
        success &= outputs[i]->generate(context);
    }
    return success;
}

bool GraphNode::generateStatement(GraphCodeGenerationContext& context)
{
    if (context.isNodeGenerated(this))
        return true;
    bool success = generateParameter(context);
    success &= solveAndGenerateOutput(context);
    success &= generate(context);
    context.markNodeGenerated(this);
    if (const GraphCodeHeaderFileAttribute* headerFile = getSerialization().getAttribute<GraphCodeHeaderFileAttribute>())
        context.addIncludeFile(headerFile->getPath());
    return success;
}

bool GraphNode::generate(GraphCodeGenerationContext& context)
{
    CodeFunctionInvocation invocation(getFunctionName());
    for (auto pin : inputs) {
        if (dynamic_cast<FlowPin*>(pin))
            continue;
        invocation.parameters.push_back(context.getParameter(pin));
    }
    for (auto pin : outputs) {
        if (dynamic_cast<FlowPin*>(pin))
            continue;
        const CodeParameter& param = context.getParameter(pin);
        invocation.outputs.push_back(param.symbol());
    }
    context.getBackend().invoke(invocation);
    return true;
}

Serializable* GraphNode::instantiate(const SerializationInfo& from)
{
    GraphNode* node = new GraphNode();
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

bool GraphNode::deserialize(const SerializationInfo& from)
{
    Base::deserialize(from);
    from.get("name", name);
    setName(name);
    const SerializationInfo* inputInfos = from.get("inputs");
    if (inputInfos) {
        for each (const auto & info in inputInfos->sublists)
        {
            GraphPin* pin = getInput(info.name);
            if (pin) {
                pin->deserialize(info);
            }
        }
    }
    const SerializationInfo* outputInfos = from.get("outputs");
    if (outputInfos) {
        for each (const auto & info in outputInfos->sublists)
        {
            GraphPin* pin = getOutput(info.name);
            if (pin) {
                pin->deserialize(info);
            }
        }
    }
    SVector2f nodePos;
    if (from.get("nodePos", nodePos)) {
        GraphNodeEditor* nodeEditor = dynamic_cast<GraphNodeEditor*>(EditorManager::getEditor(*this));
        if (nodeEditor) {
            nodeEditor->setNodePosition(nodePos);
        }
    }
    return true;
}

bool GraphNode::serialize(SerializationInfo& to)
{
    Base::serialize(to);
    to.set("name", name);
    SerializationInfo* inputInfos = to.add("inputs");
    if (inputInfos) {
        for each (auto pin in inputs)
        {
            SerializationInfo* info = inputInfos->add(pin->name);
            if (info) {
                pin->serialize(*info);
            }
        }
    }
    SerializationInfo* outputInfos = to.add("outputs");
    if (inputInfos) {
        for each (auto pin in outputs)
        {
            SerializationInfo* info = outputInfos->add(pin->name);
            if (info) {
                pin->serialize(*info);
            }
        }
    }

    GraphNodeEditor* nodeEditor = dynamic_cast<GraphNodeEditor*>(EditorManager::findEditor(this));
    if (nodeEditor) {
        SVector2f nodePos = nodeEditor->getNodePosition();
        to.set("nodePos", nodePos);
    }
    return true;
}

void GraphNode::addInternalInput(GraphPin& input)
{
    input.autoDelete = false;
    addInput(input);
}

void GraphNode::addInternalOutput(GraphPin& output)
{
    output.autoDelete = false;
    addOutput(output);
}

void GraphNode::addInput(GraphPin& input)
{
    if (&input == NULL)
        throw runtime_error("Input Pin is NULL");
    for each (GraphPin* pin in inputs)
    {
        if (pin->name == input.name)
            throw runtime_error("Input Pin use same name");
    }
    input.node = this;
    input.isOutput = false;
    inputs.push_back(&input);
}

void GraphNode::addOutput(GraphPin& output)
{
    if (&output == NULL)
        throw runtime_error("Output Pin is NULL");
    for each (GraphPin * pin in outputs)
    {
        if (pin->name == output.name)
            throw runtime_error("Output Pin use same name");
    }
    output.node = this;
    output.isOutput = true;
    outputs.push_back(&output);
}

void GraphNode::clearInputs()
{
    for each (GraphPin * pin in inputs)
    {
        if (pin->autoDelete)
            delete pin;
    }
    inputs.clear();
}

void GraphNode::clearOutputs()
{
    for each (GraphPin * pin in outputs)
    {
        if (pin->autoDelete)
            delete pin;
    }
    outputs.clear();
}

void GraphNode::clearPins()
{
    clearInputs();
    clearOutputs();
}
