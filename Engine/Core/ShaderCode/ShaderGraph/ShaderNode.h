#pragma once
#include "../../Graph/Node.h"
#include "../../Graph/FlowNode.h"
#include <array>

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
        ShaderStructPinSerialization(const char* shaderPinType);
        ShaderStructPinSerialization(const char* shaderPinType, const char* type, const char* baseType);
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

#define SerializeShaderPin(Type, BaseType) \
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
        Type* pin = ::instantiate<Type>(from); \
        pin->initialize(variableType, *this); \
        return function; \
    } \
 \
    friend void registerShaderStructPin(const Name& structName, std::initializer_list<Attribute*> attributes); \
protected: \
    Type##Serialization(const char* shaderPinType) \
        : BaseSerializationClass(signature, (string(shaderPinType) + #Type).c_str(), "ShaderPin") \
    { init(); } \
 \
    Type##Serialization(const char* shaderPinType, const char* type, const char* baseType) \
        : BaseSerializationClass(shaderPinType, type, baseType) \
    { init(); } \
 \
    void init(); \
};

#define SerializeInstanceShaderPin(Type, ...) \
void Type::Type##Serialization::init() \
{ \
    baseSerialization = &BaseSerializationClass::serialization; \
    addAttribute({ __VA_ARGS__ }); \
}

Serialization* registerShaderStructPin(const Name& structName, std::initializer_list<Attribute*> attributes);
#define REGISTER_SHADER_STRUCT_PIN(StructName, ShaderPath, ...) registerShaderStructPin(StructName, { __VA_ARGS__ })

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
    
	virtual bool generate(GraphCodeGenerationContext& context);

    static Serializable* instantiate(const SerializationInfo& from);
protected:
    bool error = false;
    CodeFunctionSignature signature;
    const ShaderFunctionNodeSerialization* serialization;
    vector<int> parameterMap;
    vector<int> returnMap;

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
    friend void registerShaderStructFunctionNodes(const CodeFunctionSignature& definition, std::initializer_list<Attribute*> attributes); \
protected: \
    Type##Serialization(const CodeFunctionSignature& signature) \
        : BaseSerializationClass(signature, (signature.name + #Type).c_str(), "ShaderNode") \
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
    
    virtual bool generate(GraphCodeGenerationContext& context);
    
    static Serializable* instantiate(const SerializationInfo& from);
protected:
    void initialize(const CodeFunctionSignature& signature, const ShaderFunctionNodeSerialization& serialization);

    GraphPin* getOutStructPin();
};

class ShaderBreakStructNode : public ShaderFunctionNode
{
public:
    SerializeShaderFunctionNode(ShaderBreakStructNode, ShaderFunctionNode);

    ShaderBreakStructNode() = default;
    virtual ~ShaderBreakStructNode() = default;

    bool solveAndGenerateOutput(GraphCodeGenerationContext& context);
    virtual bool generate(GraphCodeGenerationContext& context);
    
    static Serializable* instantiate(const SerializationInfo& from);
protected:
    ShaderStructPin* structPin;
    void initialize(const CodeFunctionSignature& signature, const ShaderFunctionNodeSerialization& serialization);
};

void registerShaderStructFunctionNodes(const CodeFunctionSignature& definition, std::initializer_list<Attribute*> attributes);
#define REGISTER_SHADER_STRUCT_FUNCTION_NODES(Definition, ...) registerShaderStructFunctionNodes(Definition, { __VA_ARGS__ })

class SwizzlePin : public ValuePin
{
public:
    Serialize(SwizzlePin, ValuePin);

    SwizzlePin(const string& name);

    virtual bool isWildcard() const;

    static Serializable* instantiate(const SerializationInfo& from);
protected:
    virtual bool isWildcardAcceptable(const GraphPin* pin) const;
};

class SwizzleNode : public ShaderNode
{
public:
    Serialize(SwizzleNode, ShaderNode);
    
    enum ComponentType
    {
        X, Y, Z, W
    };

    using Components = array<ComponentType, 4>;

    SwizzleNode();

    virtual bool generate(GraphCodeGenerationContext& context);

    Components getComponents() const;
    void setComponents(const Components& components);

    static Serializable* instantiate(const SerializationInfo & from);
    virtual bool deserialize(const SerializationInfo& from);
    virtual bool serialize(SerializationInfo& to);
protected:
    Components components;
    SwizzlePin* swizzleInPin;
    Vector2fPin* vec2OutPin;
    Vector3fPin* vec3OutPin;
    Vector4fPin* vec4OutPin;
};
