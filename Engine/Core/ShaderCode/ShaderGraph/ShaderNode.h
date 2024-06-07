#pragma once
#include "../../Graph/Node.h"
#include "../../Graph/FlowNode.h"

class ShaderGraphPinFactory
{
public:
    ShaderGraphPinFactory();

    static ShaderGraphPinFactory& get();

    void registerType(const Name& type, const Serialization& serialization);
    const Serialization* getFactory(const Name& type);
    GraphPin* construct(const Name& type, const string& name);
protected:
    unordered_map<Name, const Serialization*> codeTypeToShaderGraphPinSerialization;

    void registerDefaultTypes();
};

class ShaderNode : public GraphNode
{
public:
    Serialize(ShaderNode, GraphNode);

    ShaderNode() = default;
    virtual ~ShaderNode() = default;

    static Serializable* instantiate(const SerializationInfo & from);
};

class ShaderStructPin : public ValuePin
{
public:
    class ShaderStructPinSerialization : public ValuePinSerialization
    {
    public:
        typedef ValuePinSerialization BaseSerializationClass;
        virtual bool isClassOf(const Serializable& serializable) const;
        virtual Serializable* instantiate(const SerializationInfo& from) const;
        virtual Serializable* deserialize(const SerializationInfo& from) const;

        friend Serialization* registerShaderStructPin(const Name& structName, std::initializer_list<Attribute*> attributes);
    protected:
        ShaderStructPinSerialization(const char* type);
    };

    ShaderStructPin(const string& name);
    virtual ~ShaderStructPin() = default;

    virtual const Serialization& getSerialization() const;
    
    virtual Name getVariableType() const;

    static Serializable* instantiate(const SerializationInfo& from);
protected:
    Name variableType;
    const ShaderStructPinSerialization* serialization;

    void initialize(const Name& type, const ShaderStructPinSerialization& serialization);
};

Serialization* registerShaderStructPin(const Name& structName, std::initializer_list<Attribute*> attributes);
#define REGISTER_SHADER_STRUCT_PIN(StructName, ...) registerShaderStructPin(StructName, { __VA_ARGS__ })

class ShaderOutputNode : public ReturnNode
{
public:
    Serialize(ShaderOutputNode, ReturnNode);

    ShaderOutputNode();
    virtual ~ShaderOutputNode() = default;

    ShaderStructPin* getBSDFPin();
    
    static Serializable* instantiate(const SerializationInfo& from);
protected:
    ShaderStructPin* bxdfPin;
};

class ShaderFunctionNode : public ShaderNode
{
public:
    class ShaderFunctionNodeSerialization : public ShaderNodeSerialization
    {
    public:
        typedef ShaderNodeSerialization BaseSerializationClass;
        CodeFunctionSignature signature;
        virtual bool isClassOf(const Serializable& serializable) const;
        virtual Serializable* instantiate(const SerializationInfo& from) const;
        virtual Serializable* deserialize(const SerializationInfo& from) const;

        friend Serialization* registerShaderFunctionNode(const CodeFunctionSignature& signature, std::initializer_list<Attribute*> attributes);
    protected:
        ShaderFunctionNodeSerialization(const CodeFunctionSignature& signature);
        ShaderFunctionNodeSerialization(const CodeFunctionSignature& signature, const char* type, const char* baseType);
    };

    ShaderFunctionNode();
    virtual ~ShaderFunctionNode() = default;

    virtual Color getPinColor() const;

    virtual const Serialization& getSerialization() const;

    CodeFunctionSignature getSignature() const;

    virtual Name getFunctionName() const;

    static Serializable* instantiate(const SerializationInfo& from);
protected:
    bool error = false;
    CodeFunctionSignature signature;
    const ShaderFunctionNodeSerialization* serialization;

    void initialize(const CodeFunctionSignature& signature, const ShaderFunctionNodeSerialization& serialization);
};

#define SerializeShaderFunctionNode(Type, BaseType) \
class Type##Serialization : public BaseType::BaseType##Serialization \
{ \
public: \
	typedef BaseType##::BaseType##Serialization BaseSerializationClass; \
    virtual bool isClassOf(const Serializable& serializable) const \
    { \
        return castTo<Type>(serializable); \
    } \
    virtual Serializable* instantiate(const SerializationInfo& from) const \
    { \
        Type* function = ::instantiate<Type>(from); \
        function->initialize(signature, *this); \
        return function; \
    } \
 \
    friend void registerShaderStructFunctionNodes(const CodeFunctionSignature& structConstruction, std::initializer_list<Attribute*> attributes); \
protected: \
    Type##Serialization(const CodeFunctionSignature& signature) \
        : BaseSerializationClass(signature, (string(signature.name.str()) + #Type).c_str(), "ShaderNode") \
    { init(); } \
 \
    Type##Serialization(const CodeFunctionSignature& signature, const char* type, const char* baseType) \
        : BaseSerializationClass(signature, type, baseType) \
    { init(); } \
 \
    void init(); \
};

#define SerializeInstanceShaderFunctionNode(Type, ...) \
void Type::Type##Serialization::init() \
{ \
    baseSerialization = &BaseSerializationClass::serialization; \
    addAttribute({ __VA_ARGS__ }); \
}

Serialization* registerShaderFunctionNode(const CodeFunctionSignature& signature, std::initializer_list<Attribute*> attributes);
#define REGISTER_SHADER_FUNCTION_NODE(FunctionSignature, ...) registerShaderFunctionNode(FunctionSignature, { __VA_ARGS__ })

class ShaderMakeStructNode : public ShaderFunctionNode
{
public:
    SerializeShaderFunctionNode(ShaderMakeStructNode, ShaderFunctionNode);

    ShaderMakeStructNode() = default;
    virtual ~ShaderMakeStructNode() = default;
    
    static Serializable* instantiate(const SerializationInfo& from);
protected:
    void initialize(const CodeFunctionSignature& signature, const ShaderFunctionNodeSerialization& serialization);
};

class ShaderBreakStructNode : public ShaderFunctionNode
{
public:
    SerializeShaderFunctionNode(ShaderBreakStructNode, ShaderFunctionNode);

    ShaderBreakStructNode() = default;
    virtual ~ShaderBreakStructNode() = default;

    virtual bool generate(GraphCodeGenerationContext& context);
    
    static Serializable* instantiate(const SerializationInfo& from);
protected:
    ShaderStructPin* structPin;
    void initialize(const CodeFunctionSignature& signature, const ShaderFunctionNodeSerialization& serialization);
};

void registerShaderStructFunctionNodes(const CodeFunctionSignature& structConstruction, std::initializer_list<Attribute*> attributes);
#define REGISTER_SHADER_STRUCT_FUNCTION_NODES(Construction, ...) registerShaderStructFunctionNodes(Construction, { __VA_ARGS__ })
