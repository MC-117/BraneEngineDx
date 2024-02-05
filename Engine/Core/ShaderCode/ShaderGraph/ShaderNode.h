#pragma once
#include "../../Graph/Node.h"
#include "../../Graph/FlowNode.h"

class ShaderNode : public GraphNode
{
public:
    Serialize(ShaderNode, GraphNode);

    ShaderNode() = default;
    virtual ~ShaderNode() = default;

    static Serializable* instantiate(const SerializationInfo & from);
};

class ShaderBXDFPin : public ValuePin
{
public:
    Serialize(ShaderBXDFPin, ValuePin);

    ShaderBXDFPin(const string& name);

    virtual Color getPinColor() const;
    virtual Name getVariableType() const;
    static Serializable* instantiate(const SerializationInfo& from);
};

class ShaderOutputNode : public ReturnNode
{
public:
    Serialize(ShaderOutputNode, ReturnNode);

    ShaderOutputNode();
    virtual ~ShaderOutputNode() = default;

    ShaderBXDFPin* getBSDFPin();
    
    static Serializable* instantiate(const SerializationInfo& from);
protected:
    ShaderBXDFPin* bxdfPin;
};
