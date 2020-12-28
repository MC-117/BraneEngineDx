#include "ShadowCameraRender.h"

ShadowCameraRender::ShadowCameraRender(Material& shadowMaterial) : CameraRender(depthRenderTarget, shadowMaterial)
{
	renderOrder = -10;
}

void ShadowCameraRender::preRender()
{
	depthRenderTarget.bindFrame();
}

void ShadowCameraRender::render(RenderInfo & info)
{
}
