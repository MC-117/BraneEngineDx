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

	void addMeshCommand(const VSMMeshTransformIndexArray::CallItem& callItem);

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

	Matrix4f worldToLightViewMatrix;

	VirtualShadowMapManager::LightEntry* lightEntry;

	int firstLevel;
	float resolutionLodBias;
	float maxRadius;

	vector<LevelData> levelData;
};