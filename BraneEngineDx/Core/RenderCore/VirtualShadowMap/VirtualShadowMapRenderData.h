#pragma once

#include "VirtualShadowMapClipmap.h"
#include "../CameraRenderData.h"

class VirtualShadowMapRenderData
{
public:
	LightRenderData& lightRenderData;
	VirtualShadowMapManager manager;
	VirtualShadowMapArray shadowMapArray;
	VSMMeshTransformIndexArray intanceIndexArray;

	VirtualShadowMapRenderData(LightRenderData& lightRenderData);

	VirtualShadowMapClipmap* newClipmap(CameraRender& cameraRender);

	void create();
	void release();
	void upload();
	void clean();

	void bindForLighting(IRenderContext& context);
};