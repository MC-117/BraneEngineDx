#pragma once

#include "RenderInterface.h"
#include "CameraData.h"
#include "../GPUBuffer.h"

class CameraRender;

struct CameraRenderData : public IRenderData
{
	CameraRender* cameraRender;
	CameraData data;
	vector<Color> clearColors;
	int renderOrder;
	RenderTarget* renderTarget;
	GPUBuffer buffer = GPUBuffer(GB_Constant, sizeof(CameraData));

	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
};