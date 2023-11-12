#include "ReflectionCaptureProbeRender.h"
#include "../Asset.h"
#include "../RenderCore/RenderCore.h"

SerializeInstance(ReflectionCaptureProbeRender);

ReflectionCaptureProbeRender::ReflectionCaptureProbeRender()
{
	setRadius(200);
}

TextureCube* ReflectionCaptureProbeRender::getProbeCubeMap() const
{
	return reflectionCubeMap;
}

void ReflectionCaptureProbeRender::setProbeCubeMap(TextureCube* reflectionCubeMap)
{
	this->reflectionCubeMap = reflectionCubeMap;
}

void ReflectionCaptureProbeRender::setWorldPositionAndScale(const Vector3f& position, float scale)
{
	CaptureProbeRender::setWorldPositionAndScale(position, scale);
	sceneCaptureCube.worldPosition = position;
}

void ReflectionCaptureProbeRender::updateCapture()
{
	update = true;
	if (reflectionCubeMap == NULL) {
		reflectionCubeMap = new TextureCube(resolution, 4, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RGBA8_UF });
		reflectionCubeMap->setAutoGenMip(false);
	}
}

int ReflectionCaptureProbeRender::getProbeIndex() const
{
	return probeIndex;
}

void ReflectionCaptureProbeRender::render(RenderInfo& info)
{
	if (reflectionCubeMap == NULL)
		return;
	probeIndex = info.sceneData->setReflectionCapture(this);
	if (update) {
		sceneCaptureCube.setTexture(*reflectionCubeMap);
		sceneCaptureCube.setSize({ resolution, resolution });
		sceneCaptureCube.render(info);
		info.sceneData->reflectionProbeDataPack.cubeMapPool.markRefreshCubeMap(reflectionCubeMap);
		update = false;
	}
}

Serializable* ReflectionCaptureProbeRender::instantiate(const SerializationInfo& from)
{
	return new ReflectionCaptureProbeRender();
}

bool ReflectionCaptureProbeRender::deserialize(const SerializationInfo& from)
{
	if (!CaptureProbeRender::deserialize(from))
		return false;
	string cubeMapPath;
	if (from.get("cubeMapPath", cubeMapPath))
		TextureCube* cubeMap = getAssetByPath<TextureCube>(cubeMapPath);
	return true;
}

bool ReflectionCaptureProbeRender::serialize(SerializationInfo& to)
{
	if (!CaptureProbeRender::serialize(to))
		return false;
	string cubeMapPath = AssetInfo::getPath(getProbeCubeMap());
	to.set("cubeMapPath", cubeMapPath);
	return true;
}
