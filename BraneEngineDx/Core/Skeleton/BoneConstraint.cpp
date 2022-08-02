#include "BoneConstraint.h"
#include "../Bone.h"

SerializeInstance(BoneConstraint);

bool BoneConstraint::isValid() const
{
    return false;
}

bool BoneConstraint::isEnable() const
{
    return enable && isValid();
}

bool BoneConstraint::setup()
{
    return false;
}

void BoneConstraint::solve()
{
}

Serializable* BoneConstraint::instantiate(const SerializationInfo& from)
{
    return nullptr;
}

bool BoneConstraint::deserialize(const SerializationInfo& from)
{
    string boolStr;
    if (from.get("enable", boolStr))
        enable = boolStr == "true";
    return true;
}

bool BoneConstraint::serialize(SerializationInfo& to)
{
    Serializable::serialize(to);
    to.set("enable", enable ? "true" : "false");
    return true;
}

void BoneConstraint::setSerializeUseBoneName(bool value)
{
    serializeUseBoneName = value;
}
