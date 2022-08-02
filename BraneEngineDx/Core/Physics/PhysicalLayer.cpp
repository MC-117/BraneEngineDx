#include "PhysicalLayer.h"
#include "../Console.h"
#include "../Engine.h"

PhysicalLayer::LayerBool PhysicalLayer::collisionMatrix[layerMaxCount][layerMaxCount];

PhysicalLayer::PhysicalLayer()
{
}

PhysicalLayer::PhysicalLayer(uint8_t layer, uint32_t ignoreMask)
    : layer(layer), ignoreMask(ignoreMask)
{
    if (!checkLayer(layer))
        layer = 0;
}

PhysicalLayer::PhysicalLayer(const PxFilterData& fd)
{
    layer = fd.word0;
    customLayer = fd.word1;
    ignoreMask = fd.word2;
    customMask = fd.word3;
}

void PhysicalLayer::setLayer(uint8_t layer)
{
    if (checkLayer(layer))
        this->layer = layer;
}

uint8_t PhysicalLayer::getLayer()
{
    return layer;
}

void PhysicalLayer::setIgnoreLayer(uint8_t layer, bool ignore)
{
    if (!checkLayer(layer))
        return;
    if (ignore)
        ignoreMask |= 1 << layer;
    else
        ignoreMask &= ~(1 << layer);
}

bool PhysicalLayer::isIgnoreLayer(uint8_t layer)
{
    if (!checkLayer(layer))
        return false;
    return ignoreMask & (1 << layer);
}

PhysicalLayer::operator PxFilterData() const
{
    return PxFilterData{
        layer,
        customLayer,
        ignoreMask,
        customMask
    };
}

void PhysicalLayer::serialize(SerializationInfo& info)
{
    info.type = "PhysicalLayer";
    info.set("layer", layer);
    char ignoreMaskStr[9];
    sprintf_s(ignoreMaskStr, "%x", ignoreMask);
    info.set("ignoreMask", string(ignoreMaskStr));
}

void PhysicalLayer::deserialize(const SerializationInfo& info)
{
    float layerf;
    if (info.get("layer", layerf))
        layer = layerf;
    string ignoreMaskStr;
    info.get("ignoreMask", ignoreMaskStr);
    ignoreMask = stoul(ignoreMaskStr, nullptr, 16);
}

void PhysicalLayer::setCollisionEnable(uint8_t layer0, uint8_t layer1, bool enable)
{
    if (!checkLayer(layer0) || !checkLayer(layer1))
        return;
    collisionMatrix[layer0][layer1] = enable;
    if (layer0 != layer1)
        collisionMatrix[layer1][layer0] = enable;
}

bool PhysicalLayer::getCollisionEnable(uint8_t layer0, uint8_t layer1)
{
    return collisionMatrix[layer0][layer1];
}

PxFilterFlags PhysicalLayer::SimulationFilterShader(
    PxFilterObjectAttributes attributes0, PxFilterData filterData0,
    PxFilterObjectAttributes attributes1, PxFilterData filterData1,
    PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
    PX_UNUSED(constantBlock);
    PX_UNUSED(constantBlockSize);

    // let triggers through
    if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
    {
        pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
        return PxFilterFlags();
    }

    // Collision matrix
    if (!collisionMatrix[filterData0.word0][filterData1.word0])
    {
        return PxFilterFlag::eSUPPRESS;
    }

    // Ignore function
    bool ignore = ((1 << filterData0.word0) & filterData1.word2) || ((1 << filterData1.word0) & filterData0.word2);

    if (ignore)
    {
        return PxFilterFlag::eSUPPRESS;
    }

    pairFlags = PxPairFlag::eCONTACT_DEFAULT;

    return PxFilterFlags();
}

bool PhysicalLayer::checkLayer(uint8_t layer)
{
    if (layer >= layerMaxCount) {
        Console::warn("Layer Number can only less than %d", layerMaxCount);
        return false;
    }
    return true;
}
