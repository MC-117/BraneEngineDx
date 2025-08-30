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
	float clearDepth = 0;
	int clearStencil = 0;

	void bindAndClear(IRenderContext& context, Enum<ClearFlags> plusClearFlags = Clear_None, Enum<ClearFlags> minusClearFlags = Clear_None);

	TexInternalType getDepthStencilFormat() const;
	TexInternalType getRenderTargetFormat(uint8_t index) const;
};

class CameraRenderData : public IRenderData
{
public:
	struct CameraUploadData
	{
		CameraData cameraData;
		ProbeGridInfo probeGridInfo;
	};
	int cameraRenderID = 0;
	bool isMainCamera = false;
	CameraRender* cameraRender = NULL;
	CameraData data;
	ProbeGridInfo probeGridInfo;
	ProbeGridRenderData probeGrid;
	int renderOrder = 0;
	bool forceStencilTest = false;
	RenderComparionType stencilCompare = RCT_Equal;
	uint8_t stencilRef = 0;
	Enum<CameraRenderFlags> flags = CameraRender_Default;
	ScreenHitData* hitData = NULL;
	ISurfaceBuffer* surfaceBuffer = NULL;
	SurfaceData surface;
	Texture* sceneTexture = NULL;
	ViewCullingContext cullingContext;
	ViewCullingContext staticCullingContext;
	GPUBuffer buffer = GPUBuffer(GB_Constant, GBF_Struct, sizeof(CameraUploadData));

	virtual void updateSurfaceBuffer(RenderGraph* renderGraph);

	void setDebugProbeIndex(int probeIndex);
	int getDebugProbeIndex() const;

	void applyValidViewCullingContext(BatchDrawData& batchDrawData);

	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
	virtual void bindCameraBuffOnly(IRenderContext& context);
};