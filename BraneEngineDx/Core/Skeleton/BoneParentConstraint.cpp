#include "BoneParentConstraint.h"
#include "Skeleton.h"
#include "../Bone.h"

SerializeInstance(BoneParentConstraint);

bool BoneParentConstraint::isValid() const
{
    return parentBone != NULL && childBone != NULL;
}

bool BoneParentConstraint::setup()
{
    if (!isValid())
        return false;
    Bone* pParentBone = parentBone;
    Bone* pChildBone = childBone;
    offsetPostion = parentBone->getPosition(WORLD) - childBone->getPosition(WORLD);
    offsetRotation = parentBone->getRotation(WORLD).inverse() * childBone->getRotation(WORLD);
    return true;
}

void BoneParentConstraint::solve()
{
    if (!isEnable())
        return;
    Bone* pParentBone = parentBone;
    Bone* pChildBone = childBone;

    if (flags.has(Position)) {
        Vector3f position = pParentBone->getPosition(WORLD);
        position += offsetPostion * weight;
        pChildBone->setPosition(position, WORLD);
    }
    if (flags.has(Rotation)) {
        Quaternionf rotation = pParentBone->getRotation(WORLD);
        rotation = rotation * Quaternionf::Identity().slerp(1, offsetRotation);
        pChildBone->setRotation(rotation, WORLD);
    }
}

Serializable* BoneParentConstraint::instantiate(const SerializationInfo& from)
{
    return new BoneParentConstraint();
}

bool BoneParentConstraint::deserialize(const SerializationInfo& from)
{
    BoneConstraint::deserialize(from);

    const SerializationInfo* info = NULL;

    info = from.get("parentBone");
    if (info != NULL)
        parentBone.deserialize(*info);
    else if (skeleton != NULL) {
        string boneName;
        if (from.get("parentBone", boneName)) {
            Skeleton::BoneInfo* boneInfo = skeleton->getBone(boneName);
            if (boneInfo != NULL) {
                parentBone = boneInfo->bone;
            }
        }
    }

    info = NULL;
    info = from.get("childBone");
    if (info != NULL)
        childBone.deserialize(*info);
    else if (skeleton != NULL) {
        string boneName;
        if (from.get("childBone", boneName)) {
            Skeleton::BoneInfo* boneInfo = skeleton->getBone(boneName);
            if (boneInfo != NULL) {
                childBone = boneInfo->bone;
            }
        }
    }

    SVector3f svector;
    if (from.get("offsetPostion", svector))
        offsetPostion = svector;
    SQuaternionf squaternion;
    if (from.get("offsetRotation", squaternion))
        offsetRotation = squaternion;

    from.get("weight", weight);

    from.get("flags", SEnum(&flags));

    return true;
}

bool BoneParentConstraint::serialize(SerializationInfo& to)
{
    BoneConstraint::serialize(to);

    if (serializeUseBoneName) {
        to.add("parentBone", parentBone == NULL ? "" : parentBone->name);
        to.add("childBone", childBone == NULL ? "" : childBone->name);
    }
    else {
        SerializationInfo* info = NULL;
        info = to.add("parentBone");
        if (info != NULL)
            parentBone.serialize(*info);

        info = NULL;
        info = to.add("childBone");
        if (info != NULL)
            childBone.serialize(*info);
    }

    SVector3f svector;
    svector = offsetPostion;
    to.set("offsetPostion", svector);

    SQuaternionf squaternion;
    squaternion = offsetRotation;
    to.set("offsetRotation", squaternion);

    to.set("weight", weight);

    to.set("flags", (int)flags);

    return true;
}
