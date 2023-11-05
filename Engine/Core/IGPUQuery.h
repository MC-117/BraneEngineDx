#pragma once
#include "Config.h"
#include "GraphicType.h"

struct GPUQueryDesc
{
    GPUQueryType type = GQT_Unknown;
};

class ENGINE_API IGPUQuery
{
public:
    GPUQueryDesc& desc;

    IGPUQuery(GPUQueryDesc& desc);
    virtual ~IGPUQuery() = default;

    virtual bool create() = 0;
    virtual void begin() = 0;
    virtual void end() = 0;
    virtual bool tryFetch() = 0;
    virtual uint64_t getData() const = 0;
};
