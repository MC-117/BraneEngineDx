#include "GPUQuery.h"
#include "IVendor.h"

GPUQuery::GPUQuery(GPUQueryType type)
{
    desc.type = type;
}

GPUQuery::~GPUQuery()
{
    if (vendorGPUQuery)
        delete vendorGPUQuery;
}

bool GPUQuery::create()
{
    newVendorGPUQuery();
    return vendorGPUQuery->create();
}

void GPUQuery::begin()
{
    newVendorGPUQuery();
    vendorGPUQuery->begin();
}

void GPUQuery::end()
{
    newVendorGPUQuery();
    vendorGPUQuery->end();
}

bool GPUQuery::tryFetch()
{
    newVendorGPUQuery();
    return vendorGPUQuery->tryFetch();
}

void GPUQuery::blockFetch()
{
    if (vendorGPUQuery == NULL)
        return;
    while (!vendorGPUQuery->tryFetch()) { this_thread::yield(); }
}

uint64_t GPUQuery::getData() const
{
    return vendorGPUQuery ? vendorGPUQuery->getData() : 0;
}

IGPUQuery* GPUQuery::getVendorGPUQuery()
{
    return vendorGPUQuery;
}

void GPUQuery::newVendorGPUQuery()
{
    if (vendorGPUQuery == NULL) {
        vendorGPUQuery = VendorManager::getInstance().getVendor().newGPUQuery(desc);
        if (vendorGPUQuery == NULL) {
            throw runtime_error("Vendor new GPUQuery failed");
        }
    }
}
