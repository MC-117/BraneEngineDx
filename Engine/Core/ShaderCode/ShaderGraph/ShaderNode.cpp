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

ShaderStructPin::ShaderStructPinSerialization::ShaderStructPinSerialization(const char* shaderPinType)
    : ValuePinSerialization((string(shaderPinType) + "ShaderStructPin").c_str(), "ValuePin")
{
    baseSerialization = &BaseSerializationClass::serialization;
    addAttribute({ DEF_ATTR(GraphPinCodeType, shaderPinType), DEF_ATTR(Tag, "Shader") });
}

ShaderStructPin::ShaderStructPinSerialization::ShaderStructPinSerialization(const char* shaderPinType, const char* type, const char* baseType)
    : ValuePinSerialization(type, baseType)
{
    baseSerialization = &BaseSerializationClass::serialization;
    addAttribute({ DEF_ATTR(GraphPinCodeType, shaderPinType), DEF_ATTR(Tag, "Shader") });
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
    Console::log("Register ShaderStruct %s", structName.c_str());
    ShaderStructPin::ShaderStructPinSerialization* serialization = new ShaderStructPin::ShaderStructPinSerialization(structName.c_str());
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
    bxdfPin = castTo<ShaderStructPin>(GraphPinFactory::get().construct(BxDFName, BxDFName.c_str()));
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
    : ShaderNodeSerialization((signature.name + "ShaderFunctionNode").c_str(), "ShaderNode")
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

bool ShaderFunctionNode::generate(GraphCodeGenerationContext& context)
{
    CodeFunctionInvocation invocation(signature.name);
    
    for (int index = 0; index < signature.parameters.size(); index++) {
        int finalIndex = parameterMap[index];
        if (finalIndex == -1) {
            invocation.parameters.emplace_back("context"_N);
        }
        else {
            GraphPin* pin = getInput(finalIndex);
            invocation.parameters.emplace_back(context.getParameter(pin));
        }
    }
    for (int index = 0; index < signature.outputs.size(); index++) {
        int finalIndex = returnMap[index];
        if (finalIndex == -1) {
        }
        else {
            GraphPin* pin = getOutput(finalIndex);
            invocation.outputs.emplace_back(context.getParameter(pin).symbol());
        }
    }
    context.getBackend().invoke(invocation);
    return true;
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
    name = signature.name.c_str();
    
    int numberedNameIndex = signature.parameters.size() - 1;

    auto toPinName = [] (const char* defaultName, const char* name, int index) -> string
    {
        name = name == NULL ? defaultName : name;
        if (index == 0 || name != defaultName)
            return name;
        else
            return name + to_string(index);
    };

    for (auto& input : signature.parameters) {
        if (input.qualifiers.has(CQF_Context)) {
            parameterMap.emplace_back(-1);
        }
        else {
            GraphPin* pin = GraphPinFactory::get().construct(input.type, toPinName(input.type.c_str(), input.name.c_str(), numberedNameIndex));
            if (pin == NULL) {
                error = true;
                continue;
            }
            parameterMap.emplace_back(inputs.size());
            addInput(*pin);
            --numberedNameIndex;
        }
    }
    
    numberedNameIndex = signature.outputs.size() - 1;
    for (auto& output : signature.outputs) {
        GraphPin* pin = GraphPinFactory::get().construct(output.type, toPinName(output.type.c_str(), output.name.c_str(), numberedNameIndex));
        if (pin == NULL) {
            error = true;
            continue;
        }
        returnMap.emplace_back(outputs.size());
        addOutput(*pin);
        --numberedNameIndex;
    }
}

Serialization* registerShaderFunctionNode(const CodeFunctionSignature& signature, std::initializer_list<Attribute*> attributes)
{
    Console::log("Register ShaderFunction %s", signature.name.c_str());
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

bool ShaderMakeStructNode::generate(GraphCodeGenerationContext& context)
{
    for (auto input : inputs) {
        context.getBackend().invoke(CodeFunctionInvocation(Code::assign_op).param(
            CodeFunctionInvocation(Code::access_op).param(context.getParameter(getOutStructPin()), Name(input->getName())),
            context.getParameter(input)));
    }
    return true;
}

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
    
    displayName = string("Make ") + signature.name.c_str();
}

GraphPin* ShaderMakeStructNode::getOutStructPin()
{
    return outputs.front();
}

SerializeInstanceShaderFunctionNode(ShaderBreakStructNode);

bool ShaderBreakStructNode::solveAndGenerateOutput(GraphCodeGenerationContext& context)
{
    return true;
}

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
    
    displayName = string("Break ") + signature.name.c_str();

    structPin = castTo<ShaderStructPin>(*inputs.begin());
}

void registerShaderStructFunctionNodes(const CodeFunctionSignature& definition, std::initializer_list<Attribute*> attributes)
{
    if (definition.isValid()) {
        CodeFunctionSignature convertedDefinition = definition;
        for (auto& param : convertedDefinition.parameters) {
            param.type = shaderCodeconvertKeyword(param.type);
        }
        for (auto& output : convertedDefinition.outputs) {
            output.type = shaderCodeconvertKeyword(output.type);
        }

        ShaderBreakStructNode::ShaderBreakStructNodeSerialization* breakSerialization = new ShaderBreakStructNode::ShaderBreakStructNodeSerialization(convertedDefinition);
        breakSerialization->addAttribute(attributes);
        breakSerialization->finalize();
    
        ShaderMakeStructNode::ShaderMakeStructNodeSerialization* makeSerialization = new ShaderMakeStructNode::ShaderMakeStructNodeSerialization(convertedDefinition);
        makeSerialization->addAttribute(attributes);
        makeSerialization->finalize();
    }
}

SerializeInstance(SwizzlePin, DEF_ATTR(Tag, "Shader"));

SwizzlePin::SwizzlePin(const string& name) : ValuePin(name)
{
}

bool SwizzlePin::isWildcard() const
{
    return true;
}

Serializable* SwizzlePin::instantiate(const SerializationInfo& from)
{
    return new SwizzlePin(from.name);
}

bool SwizzlePin::isWildcardAcceptable(const GraphPin* pin) const
{
    return isClassOf<const Vector2fPin>(pin) || isClassOf<const Vector3fPin>(pin) || isClassOf<const Vector4fPin>(pin);
}

SerializeInstance(SwizzleNode);

array<char, 5> getSwizzleString(const SwizzleNode::Components& components)
{
    array<char, 5> swizzle;
    for (int i = 0; i < 4; i++) {
        char& s = swizzle[i];
        switch (components[i]) {
        case SwizzleNode::X: s = 'x'; break;
        case SwizzleNode::Y: s = 'y'; break;
        case SwizzleNode::Z: s = 'z'; break;
        case SwizzleNode::W: s = 'w'; break;
        }
    }
    swizzle[4] = '\0';
    return swizzle;
}

bool parseSwizzleString(SwizzleNode::Components& components, const string& swizzle)
{
    const int count = min(swizzle.length(), components.size());
    for (int i = 0; i < count; i++) {
        SwizzleNode::ComponentType& type = components[i];
        switch (swizzle[i]) {
        case 'x': type = SwizzleNode::X; break;
        case 'y': type = SwizzleNode::Y; break;
        case 'z': type = SwizzleNode::Z; break;
        case 'w': type = SwizzleNode::W; break;
        default: return false;
        }
    }
    return true;
}

SwizzleNode::SwizzleNode()
    : ShaderNode()
    , components({ X, Y, Z, W })
{
    flag = Flag::Expression;
    displayName = "Swizzle";
    swizzleInPin = new SwizzlePin("Vec");
    vec2OutPin = new Vector2fPin("Vec2");
    vec3OutPin = new Vector3fPin("Vec3");
    vec4OutPin = new Vector4fPin("Vec4");
    addInput(*swizzleInPin);
    addOutput(*vec2OutPin);
    addOutput(*vec3OutPin);
    addOutput(*vec4OutPin);
}

bool SwizzleNode::generate(GraphCodeGenerationContext& context)
{
    array<char, 5> vec4Swizzle = getSwizzleString(components);
    array<char, 3> vec2Swizzle = { vec4Swizzle[0], vec4Swizzle[1], '\0' };
    array<char, 4> vec3Swizzle = { vec4Swizzle[0], vec4Swizzle[1], vec4Swizzle[2], '\0' };
    context.assignParameter(vec2OutPin, CodeFunctionInvocation(Code::access_op).param(
    context.getParameter(swizzleInPin), Name(vec2Swizzle.data())));
    context.assignParameter(vec3OutPin, CodeFunctionInvocation(Code::access_op).param(
    context.getParameter(swizzleInPin), Name(vec3Swizzle.data())));
    context.assignParameter(vec4OutPin, CodeFunctionInvocation(Code::access_op).param(
        context.getParameter(swizzleInPin), Name(vec4Swizzle.data())));
    return true;
}

SwizzleNode::Components SwizzleNode::getComponents() const
{
    return components;
}

void SwizzleNode::setComponents(const Components& components)
{
    this->components = components;
}

Serializable* SwizzleNode::instantiate(const SerializationInfo& from)
{
    return new SwizzleNode();
}

bool SwizzleNode::deserialize(const SerializationInfo& from)
{
    ShaderNode::deserialize(from);
    string swizzle;
    from.get("components", swizzle);
    return parseSwizzleString(components, swizzle);
}

bool SwizzleNode::serialize(SerializationInfo& to)
{
    ShaderNode::serialize(to);
    const array<char, 5> swizzle = getSwizzleString(components);
    to.set("components", swizzle.data());
    return true;
}
