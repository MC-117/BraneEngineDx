#pragma once

#include "VirtualShadowMapClipmap.h"
#include "../CameraRenderData.h"

class VirtualShadowMapRenderData : public IRenderData
{
public:
	LightRenderData& lightRenderData;
	VirtualShadowMapManager manager;
	VirtualShadowMapArray shadowMapArray;
	VSMMeshTransformIndexArray intanceIndexArray;

	VirtualShadowMapRenderData(LightRenderData& lightRenderData);

	VirtualShadowMapClipmap* newClipmap(CameraRender& cameraRender);
	VirtualShadowMapLightEntry* newLocalLightShadow(int lightID, int persistentLightID);

	void create();
	void release();
	void upload();
	void clean();

	virtual void bind(IRenderContext& context);
	void bindForLighting(IRenderContext& context);
};