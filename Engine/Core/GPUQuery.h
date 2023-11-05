#pragma once
#include "IGPUQuery.h"

class ENGINE_API GPUQuery
{
public:
    GPUQuery(GPUQueryType type);
    ~GPUQuery();
    
    bool create();
    void begin();
    void end();
    bool tryFetch();
    void blockFetch();
    uint64_t getData() const;
    
    IGPUQuery* getVendorGPUQuery();
protected:
    GPUQueryDesc desc;
    IGPUQuery* vendorGPUQuery = NULL;

    void newVendorGPUQuery();
};
