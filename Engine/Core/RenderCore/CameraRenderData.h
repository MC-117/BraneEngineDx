#pragma once

#include "ScreenHitData.h"
#include "ProbeGridRenderData.h"
#include "ViewCullingContext.h"

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
	struct CameraUploadData
	{
		CameraData cameraData;
		ProbeGridInfo probeGridInfo;
	};
	int cameraRenderID = 0;
	CameraRender* cameraRender = NULL;
	CameraData data;
	ProbeGridInfo probeGridInfo;
	ProbeGridRenderData probeGrid;
	int renderOrder = 0;
	Enum<CameraRenderFlags> flags = CameraRender_Default;
	ScreenHitData* hitData = NULL;
	ISurfaceBuffer* surfaceBuffer = NULL;
	SurfaceData surface;
	ViewCullingContext cullingContext;
	ViewCullingContext staticCullingContext;
	GPUBuffer buffer = GPUBuffer(GB_Constant, GBF_Struct, sizeof(CameraUploadData));

	void setDebugProbeIndex(int probeIndex);
	int getDebugProbeIndex() const;

	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
	virtual void bindCameraBuffOnly(IRenderContext& context);
};