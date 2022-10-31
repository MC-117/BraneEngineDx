#pragma once

#include "MeshPass.h"
#include "DeferredLightingPass.h"
#include "HiZPass.h"
#include "ScreenSpaceReflectionPass.h"
#include "ImGUIPass.h"

class DeferredRenderGraph : public RenderGraph
{
public:
	Serialize(DeferredRenderGraph, RenderGraph);

	struct GBufferRT
	{
		int age = 0;
		Camera* camera = NULL;
		CameraRenderData cameraData;
		Texture2D gBufferA = Texture2D(1280, 720, 4, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RGBA8_UF });
		Texture2D gBufferB = Texture2D(1280, 720, 1, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_R32_F });
		Texture2D gBufferC = Texture2D(1280, 720, 4, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RGBA8_F });
		Texture2D gBufferD = Texture2D(1280, 720, 4, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RGBA8_F });
		Texture2D gBufferE = Texture2D(1280, 720, 4, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RGBA8_UI });
		RenderTarget renderTarget = RenderTarget(1280, 720, 4, true);

		Texture2D hizTexture = Texture2D(1280, 720, 1, false,
			{ TW_Border, TW_Border, TF_Point, TF_Point, TIT_R32_F, { 255, 255, 255, 255 } });
		Texture2D hitDataMap = Texture2D(1280, 720, 4, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RGBA8_F });
		Texture2D hitColorMap = Texture2D(1280, 720, 4, false, { TW_Clamp, TW_Clamp, TF_Linear, TF_Linear, TIT_RGBA8_UF });

		GBufferRT();
		void prepare();
	};

	MeshPass geometryPass;
	DeferredLightingPass lightingPass;

	HiZPass hizPass;

	ScreenSpaceReflectionPass ssrPass;

	RenderCommandList forwardList;
	MeshPass forwardPass;

	ImGuiPass imGuiPass;

	unordered_map<Camera*, GBufferRT*> gBufferRTs;

	list<RenderPass*> passes;

	DeferredRenderGraph();

	GBufferRT* getGBufferRT(Camera* camera);

	virtual bool setRenderCommand(const IRenderCommand& cmd);
	virtual void setRenderCommandList(RenderCommandList& commandList);
	virtual void setMainRenderTarget(RenderTarget& renderTarget);
	virtual void setImGuiDrawData(ImDrawData* drawData);
	virtual void addPass(RenderPass& pass);
	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();

	virtual void getPasses(vector<pair<string, RenderPass*>>& passes);

	static Serializable* instantiate(const SerializationInfo& from);
};