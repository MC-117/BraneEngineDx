#include "ShaderNode.h"
#include "../../Attributes/TagAttribute.h"

SerializeInstance(ShaderNode, DEF_ATTR(Tag, "Shader"))

Serializable* ShaderNode::instantiate(const SerializationInfo& from)
{
    return NULL;
}

SerializeInstance(ShaderBXDFPin, DEF_ATTR(Tag, "Shader"))

ShaderBXDFPin::ShaderBXDFPin(const string& name) : ValuePin(name)
{
}

Color ShaderBXDFPin::getPinColor() const
{
    return Color(166, 145, 255);
}

Name ShaderBXDFPin::getVariableType() const
{
    static const Name varType = "BXDF";
    return varType;
}

Serializable* ShaderBXDFPin::instantiate(const SerializationInfo& from)
{
    return new ShaderBXDFPin(from.name);
}

SerializeInstance(ShaderOutputNode, DEF_ATTR(Tag, "Shader"))

ShaderOutputNode::ShaderOutputNode()
{
    displayName = "ShaderOutput";
    bxdfPin = new ShaderBXDFPin("BXDF");
    addOutput(*bxdfPin);
}

ShaderBXDFPin* ShaderOutputNode::getBSDFPin()
{
    return bxdfPin;
}

Serializable* ShaderOutputNode::instantiate(const SerializationInfo& from)
{
    return new ShaderOutputNode();
}
