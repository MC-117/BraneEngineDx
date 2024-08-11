#include "Variable.h"

#include "GraphCodeGeneration.h"
#include "../Attributes/TagAttribute.h"

SerializeInstance(GraphVariable, DEF_ATTR(Tag, "Graph"));

GraphVariable::GraphVariable(const string& name) : name(name)
{
}

string GraphVariable::getName() const
{
    return name;
}

Color GraphVariable::getDisplayColor() const
{
    return Color(128, 128, 128);
}

bool GraphVariable::isGlobalVariable() const
{
    return false;
}

ValuePin* GraphVariable::newValuePin(const string& name) const
{
    return nullptr;
}

void GraphVariable::assign(const GraphVariable* other)
{
}

void GraphVariable::assignToPin(ValuePin* pin)
{
}

void GraphVariable::assignFromPin(const ValuePin* pin)
{
}

void GraphVariable::resetToDefault()
{
}

Name GraphVariable::getVariableType() const
{
    return Name::none;
}

CodeParameter GraphVariable::getDefaultParameter() const
{
    return CodeParameter::none;
}

bool GraphVariable::generate(GraphCodeGenerationContext& context)
{
    Name varName = getName();
    context.assignParameter(this, varName);
    return context.getBackend().declareVariable(CodeSymbolDefinition(getVariableType(), varName), getDefaultParameter());
}

Serializable* GraphVariable::instantiate(const SerializationInfo& from)
{
    return nullptr;
}

bool GraphVariable::deserialize(const SerializationInfo& from)
{
    Base::deserialize(from);
    return true;
}

bool GraphVariable::serialize(SerializationInfo& to)
{
    Base::serialize(to);
    return true;
}

IMP_VAR_CLASS(float, Float, Color(147, 226, 74), CodeParameter(getDefaultValue()));
IMP_VAR_CLASS(int, Int, Color(68, 201, 156), CodeParameter(getDefaultValue()));
IMP_VAR_CLASS(bool, Bool, Color(220, 48, 48), CodeParameter(CodeBool(getDefaultValue())));
IMP_VAR_CLASS(string, String, Color(124, 21, 153), CodeParameter(getDefaultValue()));
IMP_VAR_CLASS(int, Char, Color(98, 16, 176), CodeParameter(CodeChar(getDefaultValue())));
IMP_VAR_CLASS(int, KeyCode, Color(203, 217, 22), CodeParameter(getDefaultValue()));

IMP_OBJECT_VAR_CLASS(Vector2f,
{
    SVector2f vec;
    vec.deserialize(info);
    value = vec;
},
{
    SVector2f vec = value;
    vec.serialize(info);
},
Color(92, 179, 34));
IMP_OBJECT_VAR_CLASS(Vector3f,
{
    SVector3f vec;
    vec.deserialize(info);
    value = vec;
},
{
    SVector3f vec = value;
    vec.serialize(info);
},
Color(92, 179, 34));
IMP_OBJECT_VAR_CLASS(Quaternionf,
{
    SQuaternionf quat;
    quat.deserialize(info);
    value = quat;
},
{
    SQuaternionf quat = value;
    quat.serialize(info);
},
Color(92, 179, 34));
IMP_OBJECT_VAR_CLASS(Color,
{
    SColor color;
    color.deserialize(info);
    value = color;
},
{
    SColor color = value;
    color.serialize(info);
},
Color(3, 76, 173));

SerializeInstance(VariableNode);

VariableNode::VariableNode()
{
    flag = Flag::Expression;
}

VariableNode::~VariableNode()
{
    if (valuePin != NULL) {
        delete valuePin;
        valuePin = NULL;
    }
}

Color VariableNode::getNodeColor() const
{
    GraphVariable* pVariable = variable;
    return pVariable ? pVariable->getDisplayColor() : Color(128, 128, 128);
}

ValuePin* VariableNode::getValuePin()
{
    return valuePin;
}

GraphVariable* VariableNode::getVariable()
{
    return variable;
}

void VariableNode::init(GraphVariable* variable)
{
    if (variable == NULL)
        return;
    name = "Get";
    this->valuePin = variable->newValuePin("variable");
    this->variable = variable;
    valuePin->setDisplayName(variable->getName());
    addOutput(*valuePin);
}

bool VariableNode::process(GraphContext& context)
{
    GraphVariable* pVariable = variable;
    if (pVariable == NULL)
        return false;
    pVariable->assignToPin(valuePin);
    return true;
}

bool VariableNode::solveAndGenerateOutput(GraphCodeGenerationContext& context)
{
    GraphVariable* pVariable = variable;
    if (pVariable == NULL)
        return false;
    context.assignParameter(valuePin, context.getParameter(pVariable));
    return true;
}

bool VariableNode::generate(GraphCodeGenerationContext& context)
{
    return true;
}

Serializable* VariableNode::instantiate(const SerializationInfo& from)
{
    const SerializationInfo* pinInfos = from.get("outputs");
    if (pinInfos == NULL)
        return NULL;
    const SerializationInfo* pinInfo = pinInfos->get("variable");
    if (!pinInfo->serialization->isChildOf(ValuePin::ValuePinSerialization::serialization))
        return NULL;
    Serializable* serializable = pinInfo->serialization->instantiate(*pinInfo);
    if (serializable == NULL)
        return NULL;
    ValuePin* pin = (ValuePin*)serializable;
    VariableNode* node = new VariableNode();
    node->addOutput(*pin);
    node->valuePin = pin;
    return node;
}

bool VariableNode::deserialize(const SerializationInfo& from)
{
    if (!GraphNode::deserialize(from))
        return false;
    string variableName;
    if (from.get("variableName", variableName)) {
        if (valuePin)
            valuePin->setDisplayName(variableName);
    }
    const SerializationInfo* info = from.get("variable");
    if (info)
        variable.deserialize(*info);
    return true;
}

bool VariableNode::serialize(SerializationInfo& to)
{
    if (!GraphNode::serialize(to))
        return false;
    GraphVariable* pVariable = variable;
    if (pVariable) {
        to.set("variableName", pVariable->getName());
    }
    SerializationInfo* info = to.add("variable");
    if (info)
        variable.serialize(*info);
    return true;
}
