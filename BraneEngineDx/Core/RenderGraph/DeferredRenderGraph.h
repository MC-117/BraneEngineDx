#pragma once

#include "MeshPass.h"
#include "DeferredLightingPass.h"
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
		Texture2D gBufferA = Texture2D(1280, 720, 4, true, { TW_Clamp, TW_Clamp, TF_Point, TF_Point });
		Texture2D gBufferB = Texture2D(1280, 720, 1, true, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_R32_F });
		Texture2D gBufferC = Texture2D(1280, 720, 4, true, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RGBA8_F });
		Texture2D gBufferD = Texture2D(1280, 720, 4, true, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RGBA8_F });
		Texture2D gBufferE = Texture2D(1280, 720, 4, true, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RGBA8_UI });
		RenderTarget renderTarget = RenderTarget(1280, 720, 4, true);

		GBufferRT();
		void prepare();
	};

	MeshPass geometryPass;
	DeferredLightingPass lightingPass;

	RenderCommandList transparentList;
	MeshPass transparentPass;

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