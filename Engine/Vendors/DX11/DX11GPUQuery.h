#pragma once
#include "../../Core/IGPUQuery.h"

#ifdef VENDOR_USE_DX11

#include "DX11.h"

class DX11GPUQuery : public IGPUQuery
{
public:
    DX11Context& dxContext;
    ComPtr<ID3D11Query> dx11Query;
    uint64_t data;
    bool fetched = false;

    DX11GPUQuery(DX11Context& context, GPUQueryDesc& desc);
    virtual ~DX11GPUQuery();

    virtual bool create();
    void release();
    virtual void begin();
    virtual void end();
    virtual bool tryFetch();
    virtual uint64_t getData() const;
};

#endif