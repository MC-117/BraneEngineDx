#include "StringUtilityNode.h"
#include "../Console.h"

SerializeInstance(ToStringNode);

ToStringNode::ToStringNode()
{
    flag = Flag::Expression;
    displayName = "ToString";
    stringPin = new StringPin("String");
    addOutput(*stringPin);
}

StringPin* ToStringNode::getStringPin()
{
    return stringPin;
}

void ToStringNode::toString()
{
}

bool ToStringNode::process(GraphContext& context)
{
    toString();
    return true;
}

Serializable* ToStringNode::instantiate(const SerializationInfo& from)
{
    return new ToStringNode();
}

IMP_VALUE_TO_STRING_NODE(FloatToStringNode);
IMP_VALUE_TO_STRING_NODE(IntToStringNode);
IMP_VALUE_TO_STRING_NODE(BoolToStringNode);

IMP_VALUE_TO_STRING_NODE(Vector2fToStringNode);
IMP_VALUE_TO_STRING_NODE(Vector3fToStringNode);
IMP_VALUE_TO_STRING_NODE(QuaternionfToStringNode);

SerializeInstance(PrintNode);

PrintNode::PrintNode()
{
    displayName = "Print";
    stringPin = new StringPin("String");
    addInput(*stringPin);
}

StringPin* PrintNode::getStringPin()
{
    return stringPin;
}

bool PrintNode::process(GraphContext& context)
{
    Console::log(stringPin->getValue());
    return true;
}

Serializable* PrintNode::instantiate(const SerializationInfo& from)
{
    return new PrintNode();
}

SerializeInstance(PrintWarningNode);

PrintWarningNode::PrintWarningNode()
{
    displayName = "PrintWarning";
}

bool PrintWarningNode::process(GraphContext& context)
{
    Console::warn(stringPin->getValue());
    return true;
}

Serializable* PrintWarningNode::instantiate(const SerializationInfo& from)
{
    return new PrintWarningNode();
}

SerializeInstance(PrintErrorNode);

PrintErrorNode::PrintErrorNode()
{
    displayName = "PrintError";
}

bool PrintErrorNode::process(GraphContext& context)
{
    Console::error(stringPin->getValue());
    return true;
}

Serializable* PrintErrorNode::instantiate(const SerializationInfo& from)
{
    return new PrintErrorNode();
}
