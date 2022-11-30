#pragma once
#ifndef _SHADOWCAMERARENDER_H_
#define _SHADOWCAMERARENDER_H_

#include "CameraRender.h"

class ShadowCameraRender : public CameraRender
{
public:
	Texture2D depthTex = Texture2D(size.x, size.y, 1, false,
	{ TW_Border, TW_Border, TF_Point, TF_Point, TIT_D32_F, { 255, 255, 255, 255 } });

	RenderTarget depthRenderTarget = RenderTarget(size.x, size.y, depthTex);

	ShadowCameraRender(Material& shadowMaterial);

	virtual void preRender();
	virtual void render(RenderInfo& info);
};

#endif // !_SHADOWCAMERARENDER_H_
