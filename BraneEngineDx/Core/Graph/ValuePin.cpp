#include "ValuePin.h"
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

void ValueCasterManager::addCaster(Serialization& fromType, Serialization& toType, CastFunction caster)
{
    size_t hash = (size_t)&fromType;
    hash_combine(hash, &toType);
    casters.insert({ hash, caster });
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
    iter->second(from, to);
}

ValueCasterManager& ValueCasterManager::get()
{
    static ValueCasterManager manager;
    return manager;
}

IMP_VALUE_PIN(float, FloatPin);
IMP_VALUE_PIN(int, IntPin);
IMP_VALUE_PIN(bool, BoolPin);
IMP_VALUE_PIN(string, StringPin);
IMP_VALUE_PIN(char, CharPin);
IMP_VALUE_PIN(char, KeyCodePin);

IMP_VALUE_PIN(Vector2f, Vector2fPin);
IMP_VALUE_PIN(Vector3f, Vector3fPin);
IMP_VALUE_PIN(Quaternionf, QuaternionfPin);
IMP_VALUE_PIN(Color, ColorPin);

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
