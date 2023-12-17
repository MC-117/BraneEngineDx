#include "FlowNode.h"

#include "Graph.h"
#include "GraphCodeGeneration.h"

SerializeInstance(EntryNode);

bool EntryNode::generateParameter(GraphCodeGenerationContext& context)
{
    return true;
}

bool EntryNode::solveAndGenerateOutput(GraphCodeGenerationContext& context)
{
    bool success = true;
    for (int i = outputs.size() - 1; i >= 0; i--) {
        if (FlowPin* flowPin = dynamic_cast<FlowPin*>(outputs[i]))
            success &= flowPin->generate(context);
    }
    return success;
}

bool EntryNode::generate(GraphCodeGenerationContext& context)
{
    return true;
}

Serializable* EntryNode::instantiate(const SerializationInfo& from)
{
    EntryNode* node = new EntryNode();
    const SerializationInfo* inputInfos = from.get("inputs");
    if (inputInfos) {
        for each (const auto& info in inputInfos->sublists)
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

SerializeInstance(ReturnNode);

bool ReturnNode::generate(GraphCodeGenerationContext& context)
{
    vector<CodeParameter> returnValues;
    for (auto& pin : inputs) {
        if (ValuePin* valuePin = dynamic_cast<ValuePin*>(pin))
            returnValues.emplace_back(context.getParameter(valuePin));
    }
    return context.getBackend().output(returnValues);
}

Serializable* ReturnNode::instantiate(const SerializationInfo& from)
{
    ReturnNode* node = new ReturnNode();
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

SerializeInstance(InOutFlowNode);

InOutFlowNode::InOutFlowNode()
{
    inPin = new FlowPin("In");
    outPin = new FlowPin("Out");
    addInput(*inPin);
    addOutput(*outPin);
}

FlowPin* InOutFlowNode::getInPin()
{
    return inPin;
}

FlowPin* InOutFlowNode::getOutPin()
{
    return outPin;
}

Serializable* InOutFlowNode::instantiate(const SerializationInfo& from)
{
    return new InOutFlowNode();
}

SerializeInstance(SetVariableNode);

SetVariableNode::SetVariableNode() : InOutFlowNode()
{
}

ValuePin* SetVariableNode::getValuePin()
{
    return valuePin;
}

GraphVariable* SetVariableNode::getVariable()
{
    return variable;
}

void SetVariableNode::init(GraphVariable* variable)
{
    if (variable == NULL)
        return;
    name = "Set";
    this->valuePin = variable->newValuePin("variable");
    this->variable = variable;
    valuePin->setDisplayName(variable->getName());
    addInput(*valuePin);
}

bool SetVariableNode::process(GraphContext& context)
{
    GraphVariable* pVariable = variable;
    if (pVariable == NULL)
        return false;
    pVariable->assignFromPin(valuePin);
    return true;
}

bool SetVariableNode::generate(GraphCodeGenerationContext& context)
{
    GraphVariable* pVariable = variable;
    if (pVariable == NULL)
        return false;
    return context.getBackend().invoke(CodeFunctionInvocation(Code::assign_op)
        .param(context.getParameter(pVariable), context.getParameter(valuePin)));
}

Serializable* SetVariableNode::instantiate(const SerializationInfo& from)
{
    const SerializationInfo* pinInfos = from.get("inputs");
    if (pinInfos == NULL)
        return NULL;
    const SerializationInfo* pinInfo = pinInfos->get("variable");
    if (!pinInfo->serialization->isChildOf(ValuePin::ValuePinSerialization::serialization))
        return NULL;
    Serializable* serializable = pinInfo->serialization->instantiate(*pinInfo);
    if (serializable == NULL)
        return NULL;
    ValuePin* pin = (ValuePin*)serializable;
    SetVariableNode* node = new SetVariableNode();
    node->valuePin = pin;
    node->addInput(*pin);
    return node;
}

bool SetVariableNode::deserialize(const SerializationInfo& from)
{
    if (!InOutFlowNode::deserialize(from))
        return false;
    const SerializationInfo* info = from.get("variable");
    if (info)
        variable.deserialize(*info);
    return true;
}

bool SetVariableNode::serialize(SerializationInfo& to)
{
    if (!InOutFlowNode::serialize(to))
        return false;
    SerializationInfo* info = to.add("variable");
    if (info)
        variable.serialize(*info);
    return true;
}

SerializeInstance(SequenceNode);

SequenceNode::SequenceNode()
{
    displayName = "Sequence";
    inFlowPin = new FlowPin("In");
    addInput(*inFlowPin);
    addFlowPin();
    addFlowPin();
}

void SequenceNode::addFlowPin()
{
    FlowPin* flowPin = new FlowPin("Out " + to_string(outputs.size()));
    addOutput(*flowPin);
}

Serializable* SequenceNode::instantiate(const SerializationInfo& from)
{
    SequenceNode* node = new SequenceNode();
    const SerializationInfo* inputInfo = from.get("outputs");
    if (inputInfo)
        for (int i = 2; i < inputInfo->sublists.size(); i++)
            node->addFlowPin();
    return node;
}

SerializeInstance(BranchNode);

BranchNode::BranchNode()
{
    displayName = "Branch";
    inPin = new FlowPin("In");
    conditionPin = new BoolPin("Condition");
    truePin = new FlowPin("True");
    falsePin = new FlowPin("False");
    addInput(*inPin);
    addInput(*conditionPin);
    addOutput(*truePin);
    addOutput(*falsePin);
}

bool BranchNode::flowControl(GraphContext& context)
{
    bool success = true;
    if (conditionPin->getValue())
        success &= truePin->process(context);
    else
        success &= falsePin->process(context);
    return success;
}

bool BranchNode::solveAndGenerateOutput(GraphCodeGenerationContext& context)
{
    return true;
}

bool BranchNode::generate(GraphCodeGenerationContext& context)
{
    vector<ICodeScopeBackend*> backends;
    bool success = context.getBackend().branch({ context.getParameter(conditionPin) }, backends);
    assert(backends.size() == 2);
    GraphNode* falsePopNode = context.nodeStack.top();
    context.pushSubscopeBackend(backends[1]);
    success &= falsePin->generate(context);
    GraphNode* truePopNode = context.nodeStack.top();
    context.pushSubscopeBackend(backends[0]);
    success &= truePin->generate(context);
    return success;
}

Serializable* BranchNode::instantiate(const SerializationInfo& from)
{
    return new BranchNode();
}

SerializeInstance(LoopNode);

LoopNode::LoopNode()
{
    displayName = "Loop";
    inPin = new FlowPin("In");
    loopPin = new FlowPin("Loop");
    outPin = new FlowPin("Out");
    addInput(*inPin);
    addOutput(*loopPin);
    addOutput(*outPin);
}

bool LoopNode::flowControl(GraphContext& context)
{
    bool success = true;
    bool isBreak = context.loopCount < 0;
    if (isBreak) {
        context.loopCount = 0;
        success &= outPin->process(context);
    }
    else {
        context.nodeStack.push(this);
        success &= loopPin->process(context);
        context.loopCount++;
    }
    return true;
}

bool LoopNode::solveAndGenerateOutput(GraphCodeGenerationContext& context)
{
    return true;
}

bool LoopNode::generate(GraphCodeGenerationContext& context)
{
    bool success = outPin->generate(context);
    ICodeScopeBackend* backend = context.getBackend().loop(CodeBool(true));
    assert(backend);
    context.pushSubscopeBackend(backend);
    success &= loopPin->generate(context);
    return backend && success;
}

Serializable* LoopNode::instantiate(const SerializationInfo& from)
{
    return new LoopNode();
}

SerializeInstance(BreakNode);

BreakNode::BreakNode()
{
    displayName = "Break";
    breakPin = new FlowPin("Break");
    addInput(*breakPin);
}

bool BreakNode::flowControl(GraphContext& context)
{
    context.loopCount = -1;
    return true;
}

bool BreakNode::generate(GraphCodeGenerationContext& context)
{
    return context.getBackend().jumpOut();
}

Serializable* BreakNode::instantiate(const SerializationInfo& from)
{
    return new BreakNode();
}

SerializeInstance(ForLoopNode);

ForLoopNode::ForLoopNode()
{
    displayName = "ForLoop";
    inPin = new FlowPin("In");
    countPin = new IntPin("Count");
    loopPin = new FlowPin("Loop");
    indexPin = new IntPin("Index");
    outPin = new FlowPin("Out");
    addInput(*inPin);
    addInput(*countPin);
    addOutput(*loopPin);
    addOutput(*indexPin);
    addOutput(*outPin);
}

bool ForLoopNode::flowControl(GraphContext& context)
{
    bool success = true;
    bool isBreak = context.loopCount < 0 || context.loopCount >= countPin->getValue();
    if (isBreak) {
        context.loopCount = 0;
        indexPin->setValue(context.loopCount);
        success &= outPin->process(context);
    }
    else {
        indexPin->setValue(context.loopCount);
        context.nodeStack.push(this);
        success &= loopPin->process(context);
        context.loopCount++;
    }
    return true;
}

bool ForLoopNode::solveAndGenerateOutput(GraphCodeGenerationContext& context)
{
    return true;
}

bool ForLoopNode::generate(GraphCodeGenerationContext& context)
{
    bool success = indexPin->generate(context);
    ICodeScopeBackend& backend = context.getBackend();
    const CodeParameter& indexParameter = context.getParameter(indexPin);
    const CodeParameter& counterParameter = context.getParameter(countPin);
    backend.invoke(CodeFunctionInvocation(Code::assign_op)
        .param(indexParameter, Decimal(0)));
    ICodeScopeBackend* loopScope = backend.loop(CodeFunctionInvocation(Code::les_op)
        .param(indexParameter, counterParameter));
    assert(loopScope);
    ICodeScopeBackend* subscope = loopScope->subscope();
    assert(subscope);
    success &= outPin->generate(context);
    context.pushSubscopeBackend(subscope);
    ICodeScopeBackend* countScope = loopScope->subscope();
    assert(countScope);
    countScope->invoke(CodeFunctionInvocation(Code::add_op)
        .param(indexParameter, Decimal(1)).out(indexParameter.symbol()));
    success &= loopPin->generate(context);
    return subscope && success;
}

Serializable* ForLoopNode::instantiate(const SerializationInfo& from)
{
    return new ForLoopNode();
}
