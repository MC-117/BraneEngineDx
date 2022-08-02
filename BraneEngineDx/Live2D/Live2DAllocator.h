#pragma once

#include "Live2DConfig.h"

class Live2DAllocator : public Csm::ICubismAllocator
{
    void* Allocate(const Csm::csmSizeType size);
    void Deallocate(void* memory);
    void* AllocateAligned(const Csm::csmSizeType size, const Csm::csmUint32 alignment);
    void DeallocateAligned(void* alignedMemory);
};