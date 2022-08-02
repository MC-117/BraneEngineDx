#include "Variable.h"

SerializeInstance(GraphVariable);

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

IMP_VAR_CLASS(FloatVariable);
IMP_VAR_CLASS(IntVariable);
IMP_VAR_CLASS(BoolVariable);
IMP_VAR_CLASS(StringVariable);
IMP_VAR_CLASS(CharVariable);
IMP_VAR_CLASS(KeyCodeVariable);

IMP_OBJECT_VAR_CLASS(Vector2f, Vector2fVariable, Vector2fPin, Vector2f(),
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
IMP_OBJECT_VAR_CLASS(Vector3f, Vector3fVariable, Vector3fPin, Vector3f(),
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
IMP_OBJECT_VAR_CLASS(Quaternionf, QuaternionfVariable, QuaternionfPin, Quaternionf(),
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
IMP_OBJECT_VAR_CLASS(Color, ColorVariable, ColorPin, Color(),
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
