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

	//struct BloomItem {
	//	Texture2D* bloomBlurTex;
	//	RenderTarget* bloomBlurTarget;
	//	Texture2D* tempXTex;
	//	RenderTarget* tempXTarget;
	//	Texture2D* tempYTex;
	//	RenderTarget* tempYTarget;
	//};

	//vector<BloomItem> bloomList;
	////Texture2D depthTex = Texture2D(size.x, size.y, 1, false,
	////{ GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, GL_NEAREST, GL_NEAREST, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT24,{ 255, 255, 255, 255 } });
	//Texture2D ssaoTex = Texture2D(size.x * SSAORES, size.y * SSAORES, 4, true);
	//Texture2D ssaoBlurXTex = Texture2D(size.x * SSAORES, size.y * SSAORES, 4, true);
	//Texture2D ssaoBlurYTex = Texture2D(size.x * SSAORES, size.y * SSAORES, 4, true);
	//Texture2D screenTex = Texture2D(size.x, size.y, 4, true);
	//Texture2D screenBlurTex = Texture2D(size.x, size.y, 4, true, { GL_CLAMP, GL_CLAMP, GL_LINEAR, GL_LINEAR });
	//Texture2D screenBlurXTex = Texture2D(size.x, size.y, 4, true, { GL_CLAMP, GL_CLAMP, GL_LINEAR, GL_LINEAR });
	//Texture2D screenBlurYTex = Texture2D(size.x, size.y, 4, true, { GL_CLAMP, GL_CLAMP, GL_LINEAR, GL_LINEAR });
	//Texture2D volumnicLightTex = Texture2D(size.x, size.y, 4, true, { GL_CLAMP, GL_CLAMP, GL_LINEAR, GL_LINEAR });
	//Texture2D volumnicLightXTex = Texture2D(size.x, size.y, 4, true, { GL_CLAMP, GL_CLAMP, GL_LINEAR, GL_LINEAR });
	////RenderTarget depthTarget = RenderTarget(size.x, size.y, depthTex);
	//RenderTarget ssaoTarget = RenderTarget(size.x * SSAORES, size.y * SSAORES, 4);
	//RenderTarget ssaoBlurXTarget = RenderTarget(size.x * SSAORES, size.y * SSAORES, 4);
	//RenderTarget ssaoBlurYTarget = RenderTarget(size.x * SSAORES, size.y * SSAORES, 4);
	//RenderTarget screenTarget = RenderTarget(size.x, size.y, 4);
	//RenderTarget screenBlurTarget = RenderTarget(size.x, size.y, 4);
	//RenderTarget screenBlurXTarget = RenderTarget(size.x, size.y, 4);
	//RenderTarget screenBlurYTarget = RenderTarget(size.x, size.y, 4);
	//RenderTarget bloomTarget = RenderTarget(size.x, size.y, 4);
	//RenderTarget volumnicLightTarget = RenderTarget(size.x, size.y, 4);
	//RenderTarget volumnicLightXTarget = RenderTarget(size.x, size.y, 4);

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
