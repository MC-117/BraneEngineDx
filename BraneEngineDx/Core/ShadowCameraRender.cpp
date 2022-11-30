#include "ShadowCameraRender.h"

ShadowCameraRender::ShadowCameraRender(Material& shadowMaterial) : CameraRender(depthRenderTarget, shadowMaterial)
{
	renderOrder = -10;
	depthTex.setAutoGenMip(false);
}

void ShadowCameraRender::preRender()
{
	depthRenderTarget.bindFrame();
}

void ShadowCameraRender::render(RenderInfo & info)
{
}
