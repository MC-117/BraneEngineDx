#pragma once

#include "MeshPass.h"
#include "ScreenHitPass.h"
#include "ShadowDepthPass.h"
#include "VirtualShadowMapDepthPass.h"
#include "DeferredLightingPass.h"
#include "HiZPass.h"
#include "ScreenSpaceReflectionPass.h"
#include "BlitPass.h"
#include "ImGUIPass.h"

struct DeferredSurfaceBuffer
	: public ISurfaceBuffer
	, public IGBufferGetter
	, public IHiZBufferGetter
	, public IScreenSpaceReflectionBufferGetter
{
	Texture2D gBufferA = Texture2D(1280, 720, 4, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RGB10A2_UF });
	Texture2D gBufferB = Texture2D(1280, 720, 1, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_R32_F });
	Texture2D gBufferC = Texture2D(1280, 720, 4, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RGBA8_F });
	Texture2D gBufferD = Texture2D(1280, 720, 4, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RGBA8_F });
	Texture2D gBufferE = Texture2D(1280, 720, 4, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RGBA8_UI });
	RenderTarget renderTarget = RenderTarget(1280, 720, 4, true);

	Texture2D hizTexture = Texture2D(1280, 720, 1, false,
		{ TW_Border, TW_Border, TF_Point, TF_Point, TIT_R32_F, { 255, 255, 255, 255 } });
	Texture2D hitDataMap = Texture2D(1280, 720, 4, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RGBA8_F });
	Texture2D hitColorMap = Texture2D(1280, 720, 4, false, { TW_Clamp, TW_Clamp, TF_Linear, TF_Linear, TIT_RGBA8_UF });

	RenderTarget traceRenderTarget = RenderTarget(1280, 720, 4);
	RenderTarget resolveRenderTarget = RenderTarget(1280, 720, 4);

	DeferredSurfaceBuffer();

	virtual void create(CameraRender* cameraRender);
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
};

class DeferredRenderGraph : public RenderGraph
{
public:
	Serialize(DeferredRenderGraph, RenderGraph);

	Timer timer;

	bool enablePreDepthPass = false;
	bool enableVSMDepthPass = false;

	SurfaceData defaultPreDepthSurfaceData;
	SurfaceData defaultGeometrySurfaceData;
	SurfaceData defaultLightingSurfaceData;

	Material* preDepthMaterial;

	ScreenHitPass screenHitPass;
	MeshPass preDepthPass;
	MeshPass geometryPass;
	ShadowDepthPass shadowDepthPass;
	VirtualShadowMapDepthPass vsmDepthPass;
	DeferredLightingPass lightingPass;

	HiZPass hizPass;

	ScreenSpaceReflectionPass ssrPass;

	MeshPass forwardPass;

	BlitPass blitPass;
	ImGuiPass imGuiPass;

	list<RenderPass*> passes;

	DeferredRenderGraph();

	virtual ISurfaceBuffer* newSurfaceBuffer();
	virtual bool setRenderCommand(const IRenderCommand& cmd);
	virtual void setImGuiDrawData(ImDrawData* drawData);
	virtual void addPass(RenderPass& pass);
	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();

	virtual void getPasses(vector<pair<string, RenderPass*>>& passes);

	static Serializable* instantiate(const SerializationInfo& from);
};