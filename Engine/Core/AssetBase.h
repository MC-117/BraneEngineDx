#pragma once

class IAssetBase
{
public:
    virtual ~IAssetBase() = default;
};

class IStreamableAssetBase : public IAssetBase
{
public:
    virtual bool isStreamed(bool includedSubAsset) const = 0;
    virtual void streamIn() = 0;
    virtual void streamOut() = 0;
};
