#include "DX11GPUQuery.h"

DX11GPUQuery::DX11GPUQuery(DX11Context& context, GPUQueryDesc& desc)
    : IGPUQuery(desc)
    , dxContext(context)
    , data(0)
{
}

DX11GPUQuery::~DX11GPUQuery()
{
    release();
}

bool DX11GPUQuery::create()
{
    if (dx11Query == NULL) {
        D3D11_QUERY_DESC dxDesc;
        switch (desc.type)
        {
        case GQT_Occlusion:
            dxDesc.Query = D3D11_QUERY_OCCLUSION;
            break;
        case GQT_Timestamp:
            dxDesc.Query = D3D11_QUERY_TIMESTAMP;
            break;
        default:
            throw runtime_error("Unknown gpu query type");
        }
        dxDesc.MiscFlags = 0;
        if (FAILED(dxContext.device->CreateQuery(&dxDesc, dx11Query.GetAddressOf())))
            throw runtime_error("CreateQuery failed");
    }
    return dx11Query;
}

void DX11GPUQuery::release()
{
    if (dx11Query != NULL)
        dx11Query.Reset();
    fetched = false;
}

void DX11GPUQuery::begin()
{
    if (dx11Query == NULL)
        return;
    dxContext.deviceContext->Begin(dx11Query.Get());
    fetched = false;
}

void DX11GPUQuery::end()
{
    if (dx11Query == NULL)
        return;
    dxContext.deviceContext->End(dx11Query.Get());
    fetched = false;
}

bool DX11GPUQuery::tryFetch()
{
    if (dx11Query == NULL)
        return true;
    if (fetched)
        return true;
    fetched = dxContext.deviceContext->GetData(dx11Query.Get(), &data, sizeof(data), 0) != S_FALSE;
    return fetched;
}

uint64_t DX11GPUQuery::getData() const
{
    return data;
}
