#include "VirtualShadowMapClipmap.h"
#include "../DirectShadowRenderPack.h"
#include "../../Utility/MathUtility.h"

float VirtualShadowMapClipmap::getLevelRadius(int level)
{
	return pow(2.0f, float(level + 1));
}

Matrix4f getOrthoMatrix(float width, float height, float zScale, float zOffset)
{
	Matrix4f Result = Matrix4f::Identity();
	Result(0, 0) = 1 / width;
	Result(1, 1) = 1 / height;
	Result(2, 2) = zScale;
	Result(2, 3) = zOffset * zScale;
	return Result;
}

VirtualShadowMapClipmap::VirtualShadowMapClipmap(VirtualShadowMapArray& virtualShadowMapArray, CameraRenderData& cameraData, MainLightData& lightData)
	: cameraRenderData(&cameraData)
{
	if (virtualShadowMapArray.manager == NULL)
		return;

	VirtualShadowMapManager& manager = *virtualShadowMapArray.manager;
	VirtualShadowMapConfig& config = VirtualShadowMapConfig::instance();

	const float faceMatInit[16] {
		0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};

	const Matrix4f faceMat(faceMatInit);

	Matrix4f worldToLightRotationView = Matrix4f::Identity();
	worldToLightRotationView.block(0, 0, 3, 3) = lightData.worldToLightView.block(0, 0, 3, 3);

	// ZY plane to XY plane
	worldToLightViewMatrix = worldToLightRotationView * faceMat;
	const Matrix4f viewToWorldMatrix = worldToLightViewMatrix.transpose();

	float lodScale = 0.5f / cameraData.data.projectionMat(0, 0);
	lodScale *= float(VirtualShadowMapConfig::virtualShadowMapSize) / float(cameraData.data.viewSize.x());

	resolutionLodBias = config.resolutionLodBiasClipmap + log2(lodScale);
	resolutionLodBias = std::max(resolutionLodBias, 0.0f);

	firstLevel = config.firstClipmapLevel;
	const int lastLevel = config.lastClipmapLevel;

	const int levelCount = lastLevel - firstLevel + 1;

	levelData.resize(levelCount);

	worldOrigin = cameraData.data.cameraLoc;
	worldDirection = lightData.direction;

	lightEntry = manager.setLightEntry(0, cameraData.cameraRenderID);

	if (lightEntry)
		lightEntry->updateClipmap();

	const int lastLevelRadius = getLevelRadius(lastLevel);
	const int radiiPerLevel = 4;
	const int halfRadiiPerLevel = radiiPerLevel / 2;

	Vector3f snappedOrigin = worldToLightViewMatrix * Vector4f(worldOrigin, 1.0f);

	Vector3f snappedOriginLastLevel = Vector3f(
		roundf(snappedOrigin.x() / lastLevelRadius) * lastLevelRadius,
		roundf(snappedOrigin.y() / lastLevelRadius) * lastLevelRadius,
		snappedOrigin.z());

	for (int i = 0; i < levelCount; i++) {
		LevelData& level = levelData[i];

		const int absLevel = firstLevel + i;

		level.virtualShadowMap = virtualShadowMapArray.allocate();

		if (i == 0) {
			lightData.vsmID = level.virtualShadowMap->vsmID;
		}

		const float rawLevelRadius = getLevelRadius(absLevel);

		float halfLevelDim = 2.0 * rawLevelRadius;
		float snapSize = rawLevelRadius;

		Vector2f snappedCenterUnit(
			roundf(snappedOrigin.x() / snapSize),
			roundf(snappedOrigin.y() / snapSize)
		);

		Vector3f viewCenter(
			snappedCenterUnit.x() * snapSize,
			snappedCenterUnit.y() * snapSize,
			snappedOrigin.z()
		);

		Vector2i cornerOffset(
			-snappedCenterUnit.x() + halfRadiiPerLevel,
			+snappedCenterUnit.y() + halfRadiiPerLevel
		);

		Vector3f snappedWorldCenter = viewToWorldMatrix * Vector4f(viewCenter, 1.0f);

		level.worldCenter = snappedWorldCenter;
		level.cornerOffset = cornerOffset;

		VirtualShadowMapManager::ShadowMap* shadowMap = NULL;
		if (lightEntry) {
			shadowMap = lightEntry->setShadowMap(i);
		}

		float viewRadiusZ = rawLevelRadius * config.clipmapRadiusZScale;
		float viewCenterDeltaZ = 0.0f;

		if (shadowMap) {
			Vector2i pageOffset(cornerOffset * (VirtualShadowMapConfig::level0Pages >> 2));

			shadowMap->updateClipMap(
				level.virtualShadowMap->vsmID,
				worldToLightRotationView,
				pageOffset,
				cornerOffset,
				rawLevelRadius,
				viewCenter.z(),
				viewRadiusZ,
				*lightEntry
			);

			level.virtualShadowMap->shadowMap = shadowMap;

			viewCenterDeltaZ = viewCenter.z() - shadowMap->clipmap.viewCenterZ;
			viewRadiusZ = shadowMap->clipmap.viewRadiusZ;
		}

		const float zScale = 0.5 / viewRadiusZ;
		const float zOffset = viewRadiusZ + viewCenterDeltaZ;
		level.viewToClip = /*Math::orthotropic(-halfLevelDim, halfLevelDim, -halfLevelDim, halfLevelDim, zScale, zOffset);*/getOrthoMatrix(halfLevelDim, halfLevelDim, zScale, zOffset);
	}
}

int VirtualShadowMapClipmap::getLevelCount() const
{
	return levelData.size();
}

VirtualShadowMap* VirtualShadowMapClipmap::getVirtualShadowMap(unsigned int clipmapIndex) const
{
	if (clipmapIndex < levelData.size())
		return levelData[clipmapIndex].virtualShadowMap;
	return NULL;
}

VirtualShadowMapManager::LightEntry* VirtualShadowMapClipmap::getLightEntry() const
{
	return lightEntry;
}

CameraRenderData* VirtualShadowMapClipmap::getCameraRenderData() const
{
	return cameraRenderData;
}

void VirtualShadowMapClipmap::addMeshCommand(const VSMMeshTransformIndexArray::CallItem& callItem)
{
	if (lightEntry) {
		lightEntry->addMeshCommand(callItem);
	}
}

void VirtualShadowMapClipmap::getProjectData(unsigned int clipmapIndex, VirtualShadowMapProjectionData& projData) const
{
	if (clipmapIndex < levelData.size()) {
		const LevelData& level = levelData[clipmapIndex];

		projData.worldToView = MATRIX_UPLOAD_OP(worldToLightViewMatrix);
		projData.viewToClip = MATRIX_UPLOAD_OP(level.viewToClip);
		Matrix4f worldToClip = level.viewToClip * worldToLightViewMatrix;
		projData.worldToClip = MATRIX_UPLOAD_OP(worldToClip);
		Matrix4f uvMatrix = Math::getTransitionMatrix(Vector3f(0.5f, 0.5f, 0.0f)) * 
			Math::getScaleMatrix(Vector3f(0.5f, -0.5f, 1.0f));
		projData.worldToUV = MATRIX_UPLOAD_OP(uvMatrix * worldToClip);

		projData.clipmapWorldOrigin = worldOrigin;
		projData.resolutionLodBias = resolutionLodBias;

		projData.clipmapOffset = level.cornerOffset;
		projData.clipmapIndex = clipmapIndex;
		projData.clipmapLevel = firstLevel + clipmapIndex;

		projData.worldCenter = level.worldCenter;
		projData.clipmapCount = levelData.size();

		projData.worldDirection = worldDirection;
		projData.vsmID = level.virtualShadowMap->vsmID;

		projData.lightType = VSM_DirectLight;

		projData.uncached = 0;
	}
}
