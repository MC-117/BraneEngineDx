#pragma once
#ifndef _CAMERARENDER_H_
#define _CAMERARENDER_H_

#include "Render.h"
#include "Material.h"
#include "RenderTarget.h"
#include "PostProcess/PostProcessGraph.h"

class ENGINE_API CameraRender : public Render
{
	friend struct CameraRenderData;
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

	void setCameraRenderFlags(Enum<CameraRenderFlags> flags);
	Enum<CameraRenderFlags> getCameraRenderFlags() const;

	virtual void createDefaultPostProcessGraph();

	virtual void triggerScreenHit(const Vector2u& hitPosition);
	virtual bool fetchScreenHit(ScreenHitInfo& hitInfo) const;
	
	virtual Texture* getSceneMap();
	virtual RenderTarget& getRenderTarget();
	virtual Texture2D* getSceneBlurTex();

	virtual void setSize(Unit2Di size);

	virtual bool isValid();
	virtual IRendering::RenderType getRenderType() const;
	virtual void preRender(PreRenderInfo& info);
	virtual void render(RenderInfo& info);

	CameraRenderData* getRenderData();
	void setDebugProbeIndex(int probeIndex);

	static CameraRender* getMainCameraRender();
protected:
	static int cameraRenderNextID;
	static CameraRender* mainCameraRender;
	int cameraRenderID = 0;
	int debugProbeIndex = -1;
	Enum<CameraRenderFlags> flags;
	CameraRenderData* renderData = NULL;
	Texture2D* internalTexture = NULL;
	RenderTarget* internalRenderTarget = NULL;
	ScreenHitData* hitData = NULL;

	void createInternalRenderTarget();
	void createInternalHitData();

	ScreenHitData* getTriggeredScreenHitData();
};

#endif // !_CAMERARENDER_H_
