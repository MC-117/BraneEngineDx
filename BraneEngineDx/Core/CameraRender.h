#pragma once
#ifndef _CAMERARENDER_H_
#define _CAMERARENDER_H_

#include "Render.h"
#include "Material.h"
#include "RenderTarget.h"
#include "PostProcess/PostProcessGraph.h"

class CameraRender : public Render
{
public:
	Unit2Di size = { 0, 0 };
	RenderTarget* renderTarget = NULL;

	PostProcessGraph* graph = NULL;

	Color clearColor;
	CameraData cameraData;

	CameraRender();
	CameraRender(RenderTarget& renderTarget);
	virtual ~CameraRender();

	virtual bool isMainCameraRender() const;
	virtual void setMainCameraRender();

	virtual void createDefaultPostProcessGraph();
	
	virtual Texture* getSceneMap();
	virtual RenderTarget& getRenderTarget();
	virtual Texture2D* getSceneBlurTex();

	virtual void setSize(Unit2Di size);

	virtual bool isValid();
	virtual IRendering::RenderType getRenderType() const;
	virtual void preRender();
	virtual void render(RenderInfo& info);

	CameraRenderData* getRenderData();

	static CameraRender* getMainCameraRender();
protected:
	static CameraRender* mainCameraRender;
	CameraRenderData* renderData = NULL;
	Texture2D* internalTexture = NULL;
	RenderTarget* internalRenderTarget = NULL;

	void createInternalRenderTarget();
};

#endif // !_CAMERARENDER_H_
