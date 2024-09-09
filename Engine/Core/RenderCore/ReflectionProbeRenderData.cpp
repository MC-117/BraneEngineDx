#include "ReflectionProbeRenderData.h"
#include "../ProbeSystem/ReflectionCaptureProbeRender.h"

ReflectionProbeRenderData::ReflectionProbeRenderData(ProbePoolRenderData& probePool) : probePool(probePool)
{
}

int ReflectionProbeRenderData::setProbe(const ReflectionProbeUpdateData& updateData)
{
	TextureCube* cubeMap = updateData.cubeMap;
	int probeIndex = -1;
	ReflectionProbeData& data = probePool.emplace(ProbeType::ProbeType_Ref, probeIndex).reflectionProbeData;
	data.position = updateData.position;
	data.radius = updateData.radius;
	data.tintColor = updateData.tintColor;
	data.falloff = updateData.falloff;
	data.cutoff = updateData.cutoff;
	probeIndices.push_back(probeIndex);
	cubeMaps.push_back(cubeMap);
	return probeIndex;
}

void ReflectionProbeRenderData::create()
{
	cubeMapPool.setCubeMap(cubeMaps);
	int count = cubeMaps.size();
	for (int i = 0; i < count; i++) {
		ReflectionProbeData& data = probePool.getProbeByteData(probeIndices[i]).reflectionProbeData;
		data.cubeMapIndex = cubeMapPool.getCubeMapIndex(cubeMaps[i]);
		//data.reverseIndex = count - i - 1;
	}
	cubeMapPool.prepare();
}

void ReflectionProbeRenderData::release()
{
}

void ReflectionProbeRenderData::upload()
{
}

void ReflectionProbeRenderData::bind(IRenderContext& context)
{
	if (probeIndices.empty())
		return;
	static const ShaderPropertyName reflectionCubeMapName = "reflectionCubeMap";
	MipOption mipOption;
	mipOption.arrayBase = 0;
	mipOption.arrayCount = 6 * cubeMapPool.slots.size();
	context.bindTexture((ITexture*)cubeMapPool.cubeMapArray.getVendorTexture(), reflectionCubeMapName, mipOption);
}

void ReflectionProbeRenderData::clean()
{
	probeIndices.clear();
	cubeMaps.clear();
}
