#include "TwoBoneIKConstraint.h"
#include "Skeleton.h"
#include "../Bone.h"

SerializeInstance(TwoBoneIKConstraint);

bool TwoBoneIKConstraint::isValid() const
{
    return endBone != NULL && midBone != NULL && rootBone != NULL;
}

bool TwoBoneIKConstraint::setup()
{
    Bone* cBone = endBone;
    if (cBone == NULL)
        return false;
    Bone* bBone = midBone;
    if (bBone == NULL)
        return false;
    Bone* aBone = rootBone;
    if (aBone == NULL)
        return false;

    Vector3f A = aBone->getPosition(WORLD);
    Vector3f B = bBone->getPosition(WORLD);
    Vector3f C = cBone->getPosition(WORLD);
    Vector3f vAC = C - A;
    Vector3f vAB = B - A;

    Vector3f vACn = vAC.normalized();

    Vector3f vB = (vAB - vACn * vAB.dot(vACn)).normalize();
    Vector3f axis = vAB.cross(vAC);
    Quaternionf qA = aBone->getRotation(WORLD);
    refAxis = qA.inverse() * axis;
    refAxis.normalize();

    setEffectWorldPosition(C);
    setPoleWorldPosition(A + vB * vAB.norm());
    return true;
}

float triangleAngle(const float& aLen, const float& aLen1, const float& aLen2)
{
    float c = clamp((aLen1 * aLen1 + aLen2 * aLen2 - aLen * aLen) / (aLen1 * aLen2) / float(2.f), float(-1.f), float(1.f));
    return acos(c);
}

void TwoBoneIKConstraint::solve()
{
    const float minLenRatio = 0.05f;
    if (!isEnable())
        return;

    Vector3f polePosition = getPoleWorldPosition();
    Vector3f effectPosition = getEffectWorldPosition();

    Bone* aBone = rootBone;
    Bone* bBone = midBone;
    Bone* cBone = endBone;

    Quaternionf qwC = bBone->getRotation(WORLD);

    bBone->setRotation(Quaternionf::FromAngleAxis(PI * 0.5f, refAxis), LOCAL);

    Quaternionf qwA = aBone->getRotation(WORLD);
    Quaternionf qwB = bBone->getRotation(WORLD);

    Vector3f A = aBone->getPosition(WORLD);
    Vector3f B = bBone->getPosition(WORLD);
    Vector3f C = cBone->getPosition(WORLD);

    Vector3f vAB = B - A;
    Vector3f vBC = C - B;
    Vector3f vAC = C - A;
    Vector3f vAD = effectPosition - A;

    float lenAB = vAB.norm();
    float lenBC = vBC.norm();
    float lenAC = vAC.norm();
    float lenAD = vAD.norm();

    float angleAC = triangleAngle(lenAC, lenAB, lenBC);
    float angleAD = triangleAngle(lenAD, lenAB, lenBC);

    Vector3f axis = vAB.cross(vBC).normalize();

    float a = float(0.5f) * (angleAC - angleAD);
    float sin_a = sin(a);
    float cos_a = cos(a);
    Quaternionf q = Quaternionf(cos_a, axis.x() * sin_a,
        axis.y() * sin_a, axis.z() * sin_a);

    bBone->setRotation((q * qwB).normalized(), WORLD);

    C = cBone->getPosition(WORLD);
    vAC = C - A;

    q = Quaternionf::FromTwoVectors(vAC, vAD);

    aBone->setRotation((q * qwA).normalized(), WORLD);

    // Pole
    C = cBone->getPosition(WORLD);
    B = bBone->getPosition(WORLD);
    vAC = C - A;
    vAB = B - A;

    float vACLen = vAC.norm();
    if (vACLen > 0) {
        Vector3f vACn = vAC / vACLen;

        Vector3f vB = vAB - vACn * vAB.dot(vACn);

        Vector3f vAP = polePosition - A;
        Vector3f vP = vAP - vACn * vAP.dot(vACn);

        float vBLen = vB.norm();
        float vPLen = vP.norm();

        if (vBLen > 0 && vPLen > 0) {
            float minLen = minLenRatio * (lenAB + lenBC);

            if (vBLen > minLen) {
                q = Quaternionf::FromTwoVectors(vB, vP);

                qwA = aBone->getRotation(WORLD);

                aBone->setRotation((q * qwA).normalized(), WORLD);
            }
        }
    }
}

Vector3f TwoBoneIKConstraint::getPoleWorldPosition() const
{
    Transform* poleTran = poleTransform;
    if (poleTran != NULL)
        return poleTran->getPosition(WORLD);
    else if (poleSpace != WORLD) {
        Bone* aBone = rootBone;
        Transform* ikBone = effectTransform;
        if (aBone == NULL) {
            return polePosition;
        }
        else
        {
            if (ikBone == NULL) {
                return getEffectWorldPosition() + polePosition;
            }
            return (ikBone->getTransformMat() * Vector4f(
                polePosition.x(), polePosition.y(), polePosition.z(), 1)).block(0, 0, 3, 1);
        }
    }
    return polePosition;
}

Vector3f TwoBoneIKConstraint::getEffectWorldPosition() const
{
    Transform* effectTran = effectTransform;
    if (effectTran != NULL)
        return effectTran->getPosition(WORLD);
    else if (effectSpace != WORLD) {
        Bone* aBone = rootBone;
        if (aBone == NULL) {
            return effectPosition;
        }
        else
        {
            return aBone->getPosition(WORLD) + effectPosition;
        }
    }
    return effectPosition;
}

void TwoBoneIKConstraint::setPoleWorldPosition(const Vector3f& pos)
{
    Transform* poleTran = poleTransform;
    if (poleTran != NULL)
        return;
    if (poleSpace == WORLD)
        polePosition = pos;
    else {
        Bone* aBone = rootBone;
        Transform* ikBone = effectTransform;
        if (aBone == NULL) {
            polePosition = pos;
        }
        else
        {
            if (ikBone == NULL)
                polePosition = pos - getEffectWorldPosition();
            else
                polePosition = (ikBone->getTransformMat().inverse() * Vector4f(
                    pos.x(), pos.y(), pos.z(), 1)).block(0, 0, 3, 1);
        }
    }
}

void TwoBoneIKConstraint::setEffectWorldPosition(const Vector3f& pos)
{
    Transform* effectTran = effectTransform;
    if (effectTran != NULL)
        return;
    if (effectSpace == WORLD)
        effectPosition = pos;
    else {
        Bone* aBone = rootBone;
        if (aBone == NULL) {
            effectPosition = pos;
        }
        else
        {
            effectPosition = pos - aBone->getPosition(WORLD);
        }
    }
}

Serializable* TwoBoneIKConstraint::instantiate(const SerializationInfo& from)
{
    return new TwoBoneIKConstraint();
}

bool TwoBoneIKConstraint::deserialize(const SerializationInfo& from)
{
    BoneConstraint::deserialize(from);

    const SerializationInfo* info = NULL;

    info = from.get("endBone");
    if (info != NULL)
        endBone.deserialize(*info);
    else if (skeleton != NULL) {
        string boneName;
        if (from.get("endBone", boneName)) {
            Skeleton::BoneInfo* boneInfo = skeleton->getBone(boneName);
            if (boneInfo != NULL) {
                endBone = boneInfo->bone;
            }
        }
    }

    info = NULL;
    info = from.get("midBone");
    if (info != NULL)
        midBone.deserialize(*info);
    else if (skeleton != NULL) {
        string boneName;
        if (from.get("midBone", boneName)) {
            Skeleton::BoneInfo* boneInfo = skeleton->getBone(boneName);
            if (boneInfo != NULL) {
                midBone = boneInfo->bone;
            }
        }
    }

    info = NULL;
    info = from.get("rootBone");
    if (info != NULL)
        rootBone.deserialize(*info);
    else if (skeleton != NULL) {
        string boneName;
        if (from.get("rootBone", boneName)) {
            Skeleton::BoneInfo* boneInfo = skeleton->getBone(boneName);
            if (boneInfo != NULL) {
                rootBone = boneInfo->bone;
            }
        }
    }

    info = NULL;
    info = from.get("poleTransform");
    if (info != NULL)
        poleTransform.deserialize(*info);
    else if (skeleton != NULL) {
        string boneName;
        if (from.get("poleTransform", boneName)) {
            Skeleton::BoneInfo* boneInfo = skeleton->getBone(boneName);
            if (boneInfo != NULL) {
                poleTransform = boneInfo->bone;
            }
        }
    }

    info = NULL;
    info = from.get("effectTransform");
    if (info != NULL)
        effectTransform.deserialize(*info);
    else if (skeleton != NULL) {
        string boneName;
        if (from.get("effectTransform", boneName)) {
            Skeleton::BoneInfo* boneInfo = skeleton->getBone(boneName);
            if (boneInfo != NULL) {
                effectTransform = boneInfo->bone;
            }
        }
    }

    SVector3f svector;
    if (from.get("refAxis", svector))
        refAxis = svector;
    if (from.get("polePosition", svector))
        polePosition = svector;
    if (from.get("effectPosition", svector))
        effectPosition = svector;

    from.get("poleSpace", SEnum(&poleSpace));
    from.get("effectSpace", SEnum(&effectSpace));
    return true;
}

bool TwoBoneIKConstraint::serialize(SerializationInfo& to)
{
    BoneConstraint::serialize(to);

    if (serializeUseBoneName) {
        to.add("endBone", endBone == NULL ? "" : endBone->name);
        to.add("midBone", midBone == NULL ? "" : midBone->name);
        to.add("rootBone", rootBone == NULL ? "" : rootBone->name);
        to.add("poleTransform", poleTransform == NULL ? "" : poleTransform->name);
        to.add("effectTransform", effectTransform == NULL ? "" : effectTransform->name);
    }
    else {
        SerializationInfo* info = NULL;

        info = NULL;
        info = to.add("endBone");
        if (info != NULL)
            endBone.serialize(*info);

        info = NULL;
        info = to.add("midBone");
        if (info != NULL)
            midBone.serialize(*info);

        info = NULL;
        info = to.add("rootBone");
        if (info != NULL)
            rootBone.serialize(*info);

        info = NULL;
        info = to.add("poleTransform");
        if (info != NULL)
            poleTransform.serialize(*info);

        info = NULL;
        info = to.add("effectTransform");
        if (info != NULL)
            effectTransform.serialize(*info);
    }

    SVector3f svector;
    svector = refAxis;
    to.set("refAxis", svector);
    svector = polePosition;
    to.set("polePosition", svector);
    svector = effectPosition;
    to.set("effectPosition", svector);

    to.set("poleSpace", poleSpace);
    to.set("effectSpace", effectSpace);
    return true;
}
