#pragma once

#include "ProbeGridRenderData.h"
#include "../RenderTarget.h"
#include "../TextureCube.h"
#include "CameraRenderData.h"
#include "TextureCubePool.h"

class ReflectionCaptureProbeRender;

struct ReflectionProbeUpdateData
{
	TextureCube* cubeMap;
	Vector3f position;
	float radius = 0;
	Color tintColor;
	float falloff = 5;
	float cutoff = 0;
};

struct ReflectionProbeRenderData : public IRenderData
{
	ProbePoolRenderData& probePool;
	vector<TextureCube*> cubeMaps;
	vector<int> probeIndices;
	TextureCubePool cubeMapPool;

	ReflectionProbeRenderData(ProbePoolRenderData& probePool);

	int setProbe(const ReflectionProbeUpdateData& updateData);
	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
	void clean();
};