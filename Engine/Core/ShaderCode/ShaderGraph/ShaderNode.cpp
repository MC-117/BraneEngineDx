#include "ShaderNode.h"
#include "../../Attributes/TagAttribute.h"
#include "Core/Console.h"
#include "Core/Graph/GraphCodeGeneration.h"

ShaderGraphPinFactory::ShaderGraphPinFactory()
{
    registerDefaultTypes();
}

ShaderGraphPinFactory& ShaderGraphPinFactory::get()
{
    static ShaderGraphPinFactory factory;
    return factory;
}

void ShaderGraphPinFactory::registerType(const Name& type, const Serialization& serialization)
{
    if (!serialization.isChildOf(GraphPin::GraphPinSerialization::serialization))
        throw runtime_error("This is not serialization of GraphPin");
    auto iter = codeTypeToShaderGraphPinSerialization.find(type);
    if (iter != codeTypeToShaderGraphPinSerialization.end())
        throw runtime_error("This code type already been bound with factory");
    codeTypeToShaderGraphPinSerialization.emplace(type, &serialization);
}

const Serialization* ShaderGraphPinFactory::getFactory(const Name& type)
{
    auto iter = codeTypeToShaderGraphPinSerialization.find(type);
    if (iter == codeTypeToShaderGraphPinSerialization.end())
        return GraphPinFactory::get().getFactory(type);
    return iter->second;
}

GraphPin* ShaderGraphPinFactory::construct(const Name& type, const string& name)
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

void ShaderGraphPinFactory::registerDefaultTypes()
{
    static const Name float2Name = "float2";
    static const Name float3Name = "float3";
    static const Name float4Name = "float4";
    registerType(float2Name, Vector2fPin::Vector2fPinSerialization::serialization);
    registerType(float3Name, Vector3fPin::Vector3fPinSerialization::serialization);
    registerType(float4Name, Vector4fPin::Vector4fPinSerialization::serialization);
}

SerializeInstance(ShaderNode, DEF_ATTR(Tag, "Shader"))

Serializable* ShaderNode::instantiate(const SerializationInfo& from)
{
    return NULL;
}

bool ShaderStructPin::ShaderStructPinSerialization::isClassOf(const Serializable& serializable) const
{
    return castTo<ShaderStructPin>(serializable);
}

Serializable* ShaderStructPin::ShaderStructPinSerialization::instantiate(const SerializationInfo& from) const
{
    ShaderStructPin* pin = ::instantiate<ShaderStructPin>(from);
    pin->initialize(getAttribute<GraphPinCodeTypeAttribute>()->getType(), *this);
    return pin;
}

Serializable* ShaderStructPin::ShaderStructPinSerialization::deserialize(const SerializationInfo& from) const
{
    Serializable* pin = instantiate(from);
    if (!pin->deserialize(from)) {
        delete pin;
        return NULL;
    }
    return pin;
}

ShaderStructPin::ShaderStructPinSerialization::ShaderStructPinSerialization(const char* type)
    : ValuePinSerialization((string(type) + "ShaderStructPin").c_str(), "ValuePin")
{
    baseSerialization = &BaseSerializationClass::serialization;
    addAttribute({ DEF_ATTR(GraphPinCodeType, type), DEF_ATTR(Tag, "Shader") });
}

Name shaderCodeconvertKeyword(const Name& keyword)
{
    static const unordered_map<Name, Name> shaderCodeKeywordMap = {
        { "float2", Code::Vector2f_t },
        { "float3", Code::Vector3f_t },
        { "float4", Code::Vector4f_t },
    };
    auto iter = shaderCodeKeywordMap.find(keyword);
    if (iter != shaderCodeKeywordMap.end())
        return iter->second;
    return keyword;
}

Serialization* registerShaderStructPin(const Name& structName, std::initializer_list<Attribute*> attributes)
{
    Console::log("Register ShaderStruct %s", structName.str());
    ShaderStructPin::ShaderStructPinSerialization* serialization = new ShaderStructPin::ShaderStructPinSerialization(structName.str());
    serialization->addAttribute(attributes);
    serialization->finalize();
    return serialization;
}

ShaderStructPin::ShaderStructPin(const string& name)
    : ValuePin(name)
    , serialization(NULL)
{
}

const Serialization& ShaderStructPin::getSerialization() const
{
    return *serialization;
}

Name ShaderStructPin::getVariableType() const
{
    return variableType;
}

Serializable* ShaderStructPin::instantiate(const SerializationInfo& from)
{
    return new ShaderStructPin(from.name);
}

void ShaderStructPin::initialize(const Name& type, const ShaderStructPinSerialization& serialization)
{
    variableType = type;
    this->serialization = &serialization;
}

SerializeInstance(ShaderOutputNode, DEF_ATTR(Tag, "Shader"))

ShaderOutputNode::ShaderOutputNode()
{
    displayName = "ShaderOutput";
    static const Name BxDFName = "BxDF";
    bxdfPin = castTo<ShaderStructPin>(GraphPinFactory::get().construct(BxDFName, BxDFName.str()));
    if (bxdfPin == NULL)
        throw runtime_error("BxDF ShaderStructPin not found");
    addOutput(*bxdfPin);
}

ShaderStructPin* ShaderOutputNode::getBSDFPin()
{
    return bxdfPin;
}

Serializable* ShaderOutputNode::instantiate(const SerializationInfo& from)
{
    return new ShaderOutputNode();
}

bool ShaderFunctionNode::ShaderFunctionNodeSerialization::isClassOf(const Serializable& serializable) const
{
    return castTo<ShaderFunctionNode>(serializable);
}

Serializable* ShaderFunctionNode::ShaderFunctionNodeSerialization::instantiate(const SerializationInfo& from) const
{
    ShaderFunctionNode* function = ::instantiate<ShaderFunctionNode>(from);
    function->initialize(signature, *this);
    return function;
}

Serializable* ShaderFunctionNode::ShaderFunctionNodeSerialization::deserialize(const SerializationInfo& from) const
{
    Serializable* function = instantiate(from);
    if (!function->deserialize(from)) {
        delete function;
        return NULL;
    }
    return function;
}

ShaderFunctionNode::ShaderFunctionNodeSerialization::ShaderFunctionNodeSerialization(const CodeFunctionSignature& signature)
    : ShaderNodeSerialization((string(signature.name.str()) + "ShaderFunctionNode").c_str(), "ShaderNode")
    , signature(signature)
{
    baseSerialization = &BaseSerializationClass::serialization;
}

ShaderFunctionNode::ShaderFunctionNodeSerialization::ShaderFunctionNodeSerialization(
    const CodeFunctionSignature& signature, const char* type, const char* baseType)
        : ShaderNodeSerialization(type, baseType)
        , signature(signature)
{
    baseSerialization = &BaseSerializationClass::serialization;
}

Color ShaderFunctionNode::getPinColor() const
{
    return Color(252, 252, 252);
}

const Serialization& ShaderFunctionNode::getSerialization() const
{
    return *serialization;
}

CodeFunctionSignature ShaderFunctionNode::getSignature() const
{
    return signature;
}

Name ShaderFunctionNode::getFunctionName() const
{
    return signature.name;
}

Serializable* ShaderFunctionNode::instantiate(const SerializationInfo& from)
{
    return new ShaderFunctionNode();
}

ShaderFunctionNode::ShaderFunctionNode()
    : ShaderNode()
    , serialization(NULL)
{
    flag = Flag::Expression;
}

void ShaderFunctionNode::initialize(const CodeFunctionSignature& signature,
    const ShaderFunctionNodeSerialization& serialization)
{
    this->serialization = &serialization;
    this->signature = signature;
    error = false;
    name = signature.name.str();
    
    int index = signature.parameters.size() - 1;

    auto toPinName = [] (const char* defaultName, const char* name, int index) -> string
    {
        name = name == NULL ? defaultName : name;
        if (index == 0 || name != defaultName)
            return name;
        else
            return name + to_string(index);
    };
    
    for (auto& input : signature.parameters) {
        GraphPin* pin = GraphPinFactory::get().construct(input.type, toPinName(input.type.str(), input.name.str(), index));
        if (pin == NULL) {
            error = true;
            continue;
        }
        addInput(*pin);
        --index;
    }
    
    index = signature.outputs.size() - 1;
    for (auto& output : signature.outputs) {
        GraphPin* pin = GraphPinFactory::get().construct(output.type, toPinName(output.type.str(), output.name.str(), index));
        if (pin == NULL) {
            error = true;
            continue;
        }
        addOutput(*pin);
        --index;
    }
}

Serialization* registerShaderFunctionNode(const CodeFunctionSignature& signature, std::initializer_list<Attribute*> attributes)
{
    Console::log("Register ShaderFunction %s", signature.name.str());
    CodeFunctionSignature convertedSignature = signature;
    for (auto& param : convertedSignature.parameters) {
        param.type = shaderCodeconvertKeyword(param.type);
    }
    for (auto& output : convertedSignature.outputs) {
        output.type = shaderCodeconvertKeyword(output.type);
    }
    ShaderFunctionNode::ShaderFunctionNodeSerialization* serialization = new ShaderFunctionNode::ShaderFunctionNodeSerialization(convertedSignature);
    serialization->addAttribute(attributes);
    serialization->finalize();
    return serialization;
}

SerializeInstanceShaderFunctionNode(ShaderMakeStructNode);

Serializable* ShaderMakeStructNode::instantiate(const SerializationInfo& from)
{
    return new ShaderMakeStructNode();
}

void ShaderMakeStructNode::initialize(const CodeFunctionSignature& signature,
    const ShaderFunctionNodeSerialization& serialization)
{
    CodeFunctionSignature makeSignature;
    makeSignature.name = signature.name;
    makeSignature.parameters = signature.parameters;
    makeSignature.outputs.emplace_back(signature.name, signature.name);

    ShaderFunctionNode::initialize(makeSignature, serialization);
    
    displayName = string("Make ") + signature.name.str();
}

SerializeInstanceShaderFunctionNode(ShaderBreakStructNode);

bool ShaderBreakStructNode::generate(GraphCodeGenerationContext& context)
{
    for (auto output : outputs) {
        if (output->getConnectedPin()) {
            CodeFunctionInvocation invocation(Code::access_op, true);
            invocation.param(context.getParameter(structPin), Name(output->getName()));
            context.assignParameter(output, CodeParameter(invocation));
        }
    }
    return true;
}

Serializable* ShaderBreakStructNode::instantiate(const SerializationInfo& from)
{
    return new ShaderBreakStructNode();
}

void ShaderBreakStructNode::initialize(const CodeFunctionSignature& signature,
    const ShaderFunctionNodeSerialization& serialization)
{
    CodeFunctionSignature breakSignature;
    breakSignature.name = signature.name;
    breakSignature.parameters.emplace_back(signature.name, signature.name);
    breakSignature.outputs = signature.parameters;

    ShaderFunctionNode::initialize(breakSignature, serialization);
    
    displayName = string("Break ") + signature.name.str();

    structPin = castTo<ShaderStructPin>(*inputs.begin());
}

void registerShaderStructFunctionNodes(const CodeFunctionSignature& structConstruction, std::initializer_list<Attribute*> attributes)
{
    CodeFunctionSignature convertedStructConstruction = structConstruction;
    for (auto& param : convertedStructConstruction.parameters) {
        param.type = shaderCodeconvertKeyword(param.type);
    }
    for (auto& output : convertedStructConstruction.outputs) {
        output.type = shaderCodeconvertKeyword(output.type);
    }

    ShaderMakeStructNode::ShaderMakeStructNodeSerialization* makeSerialization = new ShaderMakeStructNode::ShaderMakeStructNodeSerialization(convertedStructConstruction);
    makeSerialization->addAttribute(attributes);
    makeSerialization->finalize();

    ShaderBreakStructNode::ShaderBreakStructNodeSerialization* breakSerialization = new ShaderBreakStructNode::ShaderBreakStructNodeSerialization(convertedStructConstruction);
    breakSerialization->addAttribute(attributes);
    breakSerialization->finalize();
}
