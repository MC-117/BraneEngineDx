#include "ITexture.h"

bool Texture2DDesc::hasOwnedSourceData() const
{
    return !externalData && data;
}

bool Texture2DDesc::canReleaseAssetData() const
{
    return data && hasAssetData;
}