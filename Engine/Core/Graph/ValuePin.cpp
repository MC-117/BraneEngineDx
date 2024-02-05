#include "ValuePin.h"

#include "GraphCodeGeneration.h"
#include "../Utility/hash.h"

SerializeInstance(ValuePin);

void ValuePin::resetToDefault()
{
}

void ValuePin::assign(const ValuePin* other)
{
}

void ValuePin::castFrom(const ValuePin* other)
{
    ValueCasterManager::get().doCast(other, this);
}

Name ValuePin::getVariableType() const
{
    return Name::none;
}

bool ValuePin::generateDefaultVariable(GraphCodeGenerationContext& context)
{
    generateTempVariable(context, CodeParameter::none);
    return true;
}

bool ValuePin::generate(GraphCodeGenerationContext& context)
{
    ICodeScopeBackend& backend = context.getBackend();
    if (isOutputPin()) {
        Name varName = backend.newTempVariableName();
        CodeSymbolDefinition definition(getVariableType(), varName);
        backend.declareVariable(definition);
        context.assignParameter(this, varName);
    }
    else if (connectedPin) {
        ValuePin* otherValuePin = dynamic_cast<ValuePin*>((GraphPin*)connectedPin);
        if (otherValuePin) {
            context.assignParameter(this, context.getParameter(otherValuePin));
        }
        else {
            return false;
        }
    }
    else {
        return generateDefaultVariable(context);
    }
    return true;
}

Serializable* ValuePin::instantiate(const SerializationInfo& from)
{
    return nullptr;
}

bool ValuePin::deserialize(const SerializationInfo& from)
{
    return GraphPin::deserialize(from);
}

bool ValuePin::serialize(SerializationInfo& to)
{
    return GraphPin::serialize(to);
}

ValuePin::ValuePin(const string& name) : GraphPin(name)
{
}

Name ValuePin::generateTempVariable(GraphCodeGenerationContext& context, const CodeParameter& defaultParameter)
{
    ICodeScopeBackend& backend = context.getBackend();
    Name varName = backend.newTempVariableName();
    CodeSymbolDefinition definition;
    definition.type = getVariableType();
    definition.name = varName;
    backend.declareVariable(definition, defaultParameter);
    context.assignParameter(this, varName);
    return varName;
}

void ValueCasterManager::addCaster(Serialization& fromType, Serialization& toType, CastFunction caster, CastCodeGenFunction codeGen)
{
    size_t hash = (size_t)&fromType;
    hash_combine(hash, &toType);
    casters.insert({ hash, { caster, codeGen } });
}

void ValueCasterManager::doCast(const ValuePin* from, ValuePin* to)
{
    if (from == NULL || to == NULL)
        return;
    size_t hash = (size_t)&from->getSerialization();
    hash_combine(hash, &to->getSerialization());
    auto iter = casters.find(hash);
    if (iter == casters.end())
        return;
    iter->second.castFunction(from, to);
}

bool ValueCasterManager::generate(GraphCodeGenerationContext& context, const ValuePin* from, const ValuePin* to)
{
    if (from == NULL || to == NULL)
        return false;
    size_t hash = (size_t)&from->getSerialization();
    hash_combine(hash, &to->getSerialization());
    auto iter = casters.find(hash);
    if (iter == casters.end())
        return false;
    CastCodeGenFunction func = iter->second.castCodeGenFunction;
    if (func)
        return func(context, from, to);
    return context.getBackend().invoke(
        CodeFunctionInvocation(to->getVariableType())
        .param(context.getParameter(from))
        .out(context.getParameter(to).symbol()));
}

ValueCasterManager& ValueCasterManager::get()
{
    static ValueCasterManager manager;
    return manager;
}

IMP_VALUE_PIN(float, Float, Color(147, 226, 74), CodeParameter(getDefaultValue()));
IMP_VALUE_PIN(int, Int, Color(68, 201, 156), CodeParameter(getDefaultValue()));
IMP_VALUE_PIN(bool, Bool, Color(220, 48, 48), CodeParameter(CodeBool(getDefaultValue())));
IMP_VALUE_PIN(string, String, Color(124, 21, 153), CodeParameter(getDefaultValue()));
IMP_VALUE_PIN(int, Char, Color(98, 16, 176), CodeParameter(CodeChar(getDefaultValue())));
IMP_VALUE_PIN(int, KeyCode, Color(203, 217, 22), CodeParameter(getDefaultValue()));

IMP_OBJECT_VALUE_PIN(Vector2f, Color(92, 179, 34),
{
    Vector2f defaultValue = getDefaultValue();
    Name varName = generateTempVariable(context, CodeParameter::none);
    context.getBackend().invoke(
        CodeFunctionInvocation(getVariableType())
        .param(Decimal(defaultValue.x()), Decimal(defaultValue.y()))
        .out(varName));
    return true;
},
{
    SVector2f vec;
    vec.deserialize(info);
    value = vec;
},
{
    SVector2f vec = value;
    vec.serialize(info);
});

IMP_OBJECT_VALUE_PIN(Vector3f, Color(92, 179, 34),
{
    Vector3f defaultValue = getDefaultValue();
    Name varName = generateTempVariable(context, CodeParameter::none);
    return context.getBackend().invoke(
        CodeFunctionInvocation(getVariableType())
        .param(Decimal(defaultValue.x()), Decimal(defaultValue.y()), Decimal(defaultValue.z()))
        .out(varName));
},
{
    SVector3f vec;
    vec.deserialize(info);
    value = vec;
},
{
    SVector3f vec = value;
    vec.serialize(info);
});

IMP_OBJECT_VALUE_PIN(Vector4f, Color(92, 179, 34),
{
    Vector4f defaultValue = getDefaultValue();
    Name varName = generateTempVariable(context, CodeParameter::none);
    return context.getBackend().invoke(
        CodeFunctionInvocation(getVariableType())
        .param(Decimal(defaultValue.x()), Decimal(defaultValue.y()), Decimal(defaultValue.z()), Decimal(defaultValue.w()))
        .out(varName));
},
{
    SVector4f vec;
    vec.deserialize(info);
    value = vec;
},
{
    SVector4f vec = value;
    vec.serialize(info);
});

IMP_OBJECT_VALUE_PIN(Quaternionf, Color(92, 179, 34),
{
    Quaternionf defaultValue = getDefaultValue();
    Name varName = generateTempVariable(context, CodeParameter::none);
    return context.getBackend().invoke(
        CodeFunctionInvocation(getVariableType())
        .param(Decimal(defaultValue.x()), Decimal(defaultValue.y()), Decimal(defaultValue.z()), Decimal(defaultValue.w()))
        .out(varName));
},
{
    SQuaternionf quat;
    quat.deserialize(info);
    value = quat;
},
{
    SQuaternionf quat = value;
    quat.serialize(info);
});

IMP_OBJECT_VALUE_PIN(Color, Color(3, 76, 173),
{
    Color defaultValue = getDefaultValue();
    Name varName = generateTempVariable(context, CodeParameter::none);
    return context.getBackend().invoke(
        CodeFunctionInvocation(getVariableType())
        .param(Decimal(defaultValue.r), Decimal(defaultValue.g), Decimal(defaultValue.b), Decimal(defaultValue.a))
        .out(varName));
},
{
    SColor color;
    color.deserialize(info);
    value = color;
},
{
    SColor color = value;
    color.serialize(info);
});

REGIST_VALUE_PIN_CAST_FUNC(FloatPin, IntPin, int);
REGIST_VALUE_PIN_CAST_FUNC(IntPin, FloatPin, float);
REGIST_VALUE_PIN_CAST_FUNC(BoolPin, IntPin, int);
REGIST_VALUE_PIN_CAST_FUNC(IntPin, BoolPin, bool);
REGIST_VALUE_PIN_CAST_FUNC(BoolPin, FloatPin, float);
REGIST_VALUE_PIN_CAST_FUNC(FloatPin, BoolPin, bool);
REGIST_VALUE_PIN_CAST_FUNC(CharPin, IntPin, int);
REGIST_VALUE_PIN_CAST_FUNC(IntPin, CharPin, char);
REGIST_VALUE_PIN_CAST_FUNC(KeyCodePin, IntPin, int);
REGIST_VALUE_PIN_CAST_FUNC(IntPin, KeyCodePin, char);
