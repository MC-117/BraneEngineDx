#include "VirtualShadowMap.h"
#include "../../Utility/MathUtility.h"

bool VirtualShadowMapManager::ShadowMap::isValid() const
{
	return preVirtualShadowMapID != -1 && preRendered;
}

void VirtualShadowMapManager::ShadowMap::updateClipMap(MainLightData& lightData)
{
	
}

VirtualShadowMapManager::ShadowMap* VirtualShadowMapManager::LightEntry::setShadowMap(int index)
{
	shadowMaps.resize(std::max(index + 1, (int)shadowMaps.size()), NULL);
	ShadowMap*& shadowMap = shadowMaps[index];
	if (index == NULL)
		shadowMap = new ShadowMap();
	return shadowMap;
}

VirtualShadowMapManager::VirtualShadowMapManager()
	: maxPageCount(4096), poolTexture(Texture2D(poolWidth, maxPageCount, 1, false,
		{ TW_Border, TW_Border, TF_Point, TF_Point, TIT_D32_F, { 255, 255, 255, 255 } }))
{
}

VirtualShadowMapManager::LightEntry* VirtualShadowMapManager::setLightEntry(int lightID)
{
	auto iter = curShadowMaps.find(lightID);
	if (iter == curShadowMaps.end())
		return iter->second;

	iter = preShadowMaps.find(lightID);
	LightEntry* shadowMap;
	if (iter == preShadowMaps.end()) {
		shadowMap = new LightEntry();
	}
	else {
		shadowMap = iter->second;
	}

	curShadowMaps.insert(make_pair(lightID, shadowMap));

	return shadowMap;
}

VirtualShadowMapManager::ShadowMap* VirtualShadowMapManager::setShadowMap(int lightID, int index)
{
	return setLightEntry(lightID)->setShadowMap(index);
}
