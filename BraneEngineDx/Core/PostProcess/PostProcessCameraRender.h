#pragma once
#ifndef _POSTPROCESSCAMERA_H_
#define _POSTPROCESSCAMERA_H_

#include "../CameraRender.h"
#include "../DirectLight.h"
#include "PostProcessGraph.h"

#define SSAORES 0.5

class PostProcessCameraRender : public CameraRender
{
public:
	DirectLight* volumnicLight = NULL;

	PostProcessGraph graph;
	RenderTarget postRenderTarget = RenderTarget(size.x, size.y, 4);

	PostProcessCameraRender(RenderTarget& renderTarget = RenderTarget::defaultRenderTarget, Material& material = Material::nullMaterial);
	~PostProcessCameraRender();

	virtual Texture2D* getSceneBlurTex();

	virtual void setSize(Unit2Di size);
	virtual void setVolumnicLight(DirectLight& light);

	virtual void render(RenderInfo& info);
};

#endif // !_POSTPROCESSCAMERA_H_
