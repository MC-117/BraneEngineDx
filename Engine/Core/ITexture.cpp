#include "ITexture.h"

bool TextureDesc::hasOwnedSourceData() const
{
    return !externalData && data;
}

bool TextureDesc::canReleaseAssetData() const
{
    return data && hasAssetData;
}