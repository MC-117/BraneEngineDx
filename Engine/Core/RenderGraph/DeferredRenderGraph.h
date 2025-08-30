#pragma once

#include "MeshPass.h"
#include "ScreenHitPass.h"
#include "BuildProbeGridPass.h"
#include "ShadowDepthPass.h"
#include "VirtualShadowMapDepthPass.h"
#include "DeferredLightingPass.h"
#include "HiZPass.h"
#include "ScreenSpaceReflectionPass.h"
#include "TranslucentPass.h"
#include "GenMipPass.h"
#include "BlitPass.h"
#include "ImGUIPass.h"

struct DeferredSurfaceBuffer
	: public ISurfaceBuffer
	, public IGBufferGetter
	, public IHiZBufferGetter
	, public IScreenSpaceReflectionBufferGetter
	, public ISceneColorMipsGetter
	, public IDebugBufferGetter
{
	Texture2D gBufferA;
	Texture2D gBufferB;
	Texture2D gBufferC;
	Texture2D gBufferD;
	Texture2D gBufferE;
	RenderTarget renderTarget;

	Texture2D hizTexture;
	Texture2D hitDataMap;
	Texture2D hitColorMap;

	RenderTarget traceRenderTarget;
	RenderTarget resolveRenderTarget;

	Texture2D sceneColorMips;

	Texture2D debugBuffer;
	RenderTarget debugRenderTarget;

	DeferredSurfaceBuffer();

	virtual void create(CameraRenderData* cameraRenderData);
	virtual void resize(unsigned int width, unsigned int height);
	virtual void bind(IRenderContext& context);

	virtual RenderTarget* getRenderTarget();
	virtual Texture* getDepthTexture();

	virtual Texture* getGBufferA();
	virtual Texture* getGBufferB();
	virtual Texture* getGBufferC();
	virtual Texture* getGBufferD();
	virtual Texture* getGBufferE();

	virtual Texture* getHiZTexture();

	virtual Texture* getHitDataTexture();
	virtual Texture* getHitColorTexture();
	virtual RenderTarget* getTraceRenderTarget();
	virtual RenderTarget* getResolveRenderTarget();

	virtual Texture* getSceneColorMips();

	virtual Texture* getDebugBuffer();
	virtual RenderTarget* getDebugRenderTarget();
};

class DeferredRenderGraph : public RenderGraph
{
public:
	Serialize(DeferredRenderGraph, RenderGraph);

	Timer timer;

	bool enablePreDepthPass = false;

	SurfaceData defaultPreDepthSurfaceData;
	SurfaceData defaultGeometrySurfaceData;
	SurfaceData defaultLightingSurfaceData;

	MaterialRenderData* preDepthMaterialRenderData;

	BaseRenderDataCollector renderDataCollectorMainThread;
	BaseRenderDataCollector renderDataCollectorRenderThread;

	ScreenHitPass screenHitPass;
	MeshPass preDepthPass;
	BuildProbeGridPass buildProbeGridPass;
	MeshPass geometryPass;
	ShadowDepthPass shadowDepthPass;
	VirtualShadowMapDepthPass vsmDepthPass;
	DeferredLightingPass lightingPass;

	HiZPass hizPass;

	GenMipPass genMipPass;

	ScreenSpaceReflectionPass ssrPass;

	TranslucentPass translucentPass;

	BlitPass blitPass;
	ImGuiPass imGuiPass;

	list<RenderPass*> passes;

	DeferredRenderGraph();

	virtual bool loadDefaultResource();

	virtual ISurfaceBuffer* newSurfaceBuffer();
	virtual bool setRenderCommand(const IRenderCommand& cmd);
	virtual void setImGuiDrawData(ImDrawData* drawData);
	virtual void addPass(RenderPass& pass);
	virtual void prepare();
	virtual void execute(IRenderContext& context, long long renderFrame);
	virtual void reset();

	bool triggerPersistentDebugDrawThisFrame = false;
	void triggerPersistentDebugDraw();

	virtual void getPasses(vector<pair<string, RenderPass*>>& passes);
	virtual IRenderDataCollector* getRenderDataCollectorMainThread();
	virtual IRenderDataCollector* getRenderDataCollectorRenderThread();

	static Serializable* instantiate(const SerializationInfo& from);
};