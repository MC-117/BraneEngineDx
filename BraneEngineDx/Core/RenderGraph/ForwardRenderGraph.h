#pragma once

#include "MeshPass.h"
#include "ShadowDepthPass.h"
#include "BlitPass.h"
#include "ImGUIPass.h"

class ForwardRenderGraph : public RenderGraph
{
public:
	Serialize(ForwardRenderGraph, RenderGraph);

	ShadowDepthPass shadowDepthPass;
	MeshPass meshPass;
	BlitPass blitPass;
	ImGuiPass imGuiPass;

	list<RenderPass*> passes;

	virtual ISurfaceBuffer* newSurfaceBuffer();
	virtual bool setRenderCommand(const IRenderCommand& cmd);
	virtual void setImGuiDrawData(ImDrawData* drawData);
	virtual void addPass(RenderPass& pass);
	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();

	static Serializable* instantiate(const SerializationInfo& from);
};