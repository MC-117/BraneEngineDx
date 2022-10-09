#pragma once

#include "MeshPass.h"
#include "ImGUIPass.h"

class DeferredRenderGraph : public RenderGraph
{
public:
	Serialize(DeferredRenderGraph, RenderGraph);

	MeshPass geometryPass;
	ImGuiPass imGuiPass;

	list<RenderPass*> passes;

	virtual bool setRenderCommand(const IRenderCommand& cmd);
	virtual void setRenderCommandList(RenderCommandList& commandList);
	virtual void setMainRenderTarget(RenderTarget& renderTarget);
	virtual void setImGuiDrawData(ImDrawData* drawData);
	virtual void addPass(RenderPass& pass);
	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();

	static Serializable* instantiate(const SerializationInfo& from);
};