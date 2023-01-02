#pragma once

#include "RenderInterface.h"
#include "../GPUBuffer.h"
#include "../RenderTarget.h"
#include "../TextureCube.h"
#include "CameraRenderData.h"
#include "TextureCubePool.h"

class ReflectionCapture;

struct ReflectionProbeData
{
	Color tintColor;
	Vector3f position;
	float radius = 0;
	float falloff = 5;
	float cutoff = 0;
	int cubeMapIndex = -1;
	int reverseIndex = 0;
};

struct ReflectionProbeRenderData : public IRenderData
{
	vector<TextureCube*> cubeMaps;
	vector<ReflectionProbeData> probeDatas;
	GPUBuffer probeDataBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(ReflectionProbeData));
	TextureCubePool cubeMapPool;

	int setProbe(ReflectionCapture* capture);
	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
	void clean();
};