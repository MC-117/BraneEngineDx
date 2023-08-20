#pragma once

#include "ProbeGridRenderData.h"
#include "../RenderTarget.h"
#include "../TextureCube.h"
#include "CameraRenderData.h"
#include "TextureCubePool.h"

class ReflectionCaptureProbeRender;

struct ReflectionProbeRenderData : public IRenderData
{
	ProbePoolRenderData& probePool;
	vector<TextureCube*> cubeMaps;
	vector<int> probeIndices;
	TextureCubePool cubeMapPool;

	ReflectionProbeRenderData(ProbePoolRenderData& probePool);

	int setProbe(ReflectionCaptureProbeRender* capture);
	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
	void clean();
};