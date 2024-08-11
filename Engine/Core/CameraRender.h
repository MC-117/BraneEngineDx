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

	void setCameraTag(const Name& tag);

	virtual bool isMainCameraRender() const;

	void setCameraRenderFlags(Enum<CameraRenderFlags> flags);
	Enum<CameraRenderFlags> getCameraRenderFlags() const;

	virtual void createDefaultPostProcessGraph();

	virtual void triggerScreenHit(const Vector2u& hitPosition);
	virtual bool fetchScreenHit(ScreenHitInfo& hitInfo) const;
	
	virtual Texture* getSceneTexture();
	virtual RenderTarget& getRenderTarget();
	virtual Texture2D* getSceneBlurTex();

	virtual void setSize(Unit2Di size);

	virtual bool isValid();
	virtual IRendering::RenderType getRenderType() const;
	virtual void preRender(PreRenderInfo& info);
	virtual void render(RenderInfo& info);

	CameraRenderData* getRenderData();
	void setDebugProbeIndex(int probeIndex);
protected:
	static int cameraRenderNextID;
	int cameraRenderID = 0;
	int debugProbeIndex = -1;
	Name tag;
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
