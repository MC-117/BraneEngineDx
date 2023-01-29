#include "ReflectionProbeRenderData.h"
#include "../ProbeSystem/ReflectionCaptureProbeRender.h"

int ReflectionProbeRenderData::setProbe(ReflectionCaptureProbeRender* capture)
{
	TextureCube* cubeMap = capture->getProbeCubeMap();
	ReflectionProbeData& data = probeDatas.emplace_back();
	data.position = capture->getWorldPosition();
	data.radius = capture->getRadius();
	data.tintColor = capture->tintColor;
	cubeMaps.push_back(cubeMap);
	return cubeMaps.size() - 1;
}

void ReflectionProbeRenderData::create()
{
	cubeMapPool.setCubeMap(cubeMaps);
	int count = cubeMaps.size();
	for (int i = 0; i < count; i++) {
		ReflectionProbeData& data = probeDatas[i];
		data.cubeMapIndex = cubeMapPool.getCubeMapIndex(cubeMaps[i]);
		data.reverseIndex = count - i - 1;
	}
	cubeMapPool.prepare();
}

void ReflectionProbeRenderData::release()
{
	probeDataBuffer.resize(0);
}

void ReflectionProbeRenderData::upload()
{
	for (auto& data : probeDatas) {
		if (data.cubeMapIndex >= 0)
			continue;

	}
	probeDataBuffer.uploadData(probeDatas.size(), probeDatas.data());
}

void ReflectionProbeRenderData::bind(IRenderContext& context)
{
	if (probeDataBuffer.empty())
		return;
	context.bindBufferBase(probeDataBuffer.getVendorGPUBuffer(), "reflectionProbes");
	MipOption mipOption;
	mipOption.arrayBase = 0;
	mipOption.arrayCount = 6 * cubeMapPool.slots.size();
	context.bindTexture((ITexture*)cubeMapPool.cubeMapArray.getVendorTexture(), "reflectionCubeMap", mipOption);
}

void ReflectionProbeRenderData::clean()
{
	probeDatas.clear();
	cubeMaps.clear();
}
