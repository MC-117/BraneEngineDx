#pragma once

#include "../LightRenderData.h"
#include "VirtualShadowMap.h"

class VirtualShadowMapClipmap
{
public:
	static float getLevelRadius(int level);

	VirtualShadowMapClipmap(VirtualShadowMapArray& virtualShadowMapArray, CameraRenderData& cameraData, MainLightData& lightData);

	int getLevelCount() const;
	VirtualShadowMap* getVirtualShadowMap(unsigned int clipmapIndex) const;
	VirtualShadowMapManager::LightEntry* getLightEntry() const;
	CameraRenderData* getCameraRenderData() const;
	Vector3f getLevelWorldCenter(unsigned int clipmapIndex) const;
	bool getLevelShadowViewInfo(unsigned int clipmapIndex, VirtualShadowMapArray::ShadowViewInfo& shadowViewInfo) const;

	void addMeshCommand(const VSMMeshBatchDrawCallCollection::CallItem& callItem);
	void clean();

	void getProjectData(unsigned int clipmapIndex, VirtualShadowMapProjectionData& projData) const;
protected:
	struct LevelData
	{
		VirtualShadowMap* virtualShadowMap = NULL;
		Matrix4f viewToClip;
		Vector3f worldCenter;
		Vector2i cornerOffset;
	};

	CameraRenderData* cameraRenderData = NULL;

	Vector3f worldOrigin;
	Vector3f worldDirection;

	Matrix4f worldToLightViewMatrix;

	VirtualShadowMapManager::LightEntry* lightEntry;

	int firstLevel;
	float resolutionLodBias;
	float maxRadius;

	vector<LevelData> levelData;
};