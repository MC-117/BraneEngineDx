#pragma once

#include "ScreenHitData.h"

class CameraRender;

struct SurfaceData
{
	struct Hasher
	{
		size_t operator()(const SurfaceData& s) const;
		size_t operator()(const SurfaceData* s) const;
	};
	Enum<ClearFlags> clearFlags = Clear_None;
	RenderTarget* renderTarget = NULL;
	vector<Color> clearColors;
	float clearDepth = 1;
	int clearStencil = 0;

	void bind(IRenderContext& context, Enum<ClearFlags> plusClearFlags = Clear_None, Enum<ClearFlags> minusClearFlags = Clear_None);
};

struct CameraRenderData : public IRenderData
{
	CameraRender* cameraRender;
	CameraData data;
	int renderOrder;
	ScreenHitData* hitData = NULL;
	SurfaceData surface;
	GPUBuffer buffer = GPUBuffer(GB_Constant, GBF_Struct, sizeof(CameraData));

	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
};