#pragma once

#include "MeshPass.h"
#include "MultiSampleResolvePass.h"
#include "ImGUIPass.h"

class ForwardRenderGraph : public RenderGraph
{
public:
	Serialize(ForwardRenderGraph, RenderGraph);

	MeshPass meshPass;
	MultiSampleResolvePass resolvePass;
	ImGuiPass imGuiPass;

	list<RenderPass*> passes;

	virtual void setRenderCommandList(RenderCommandList& commandList);
	virtual void setMainRenderTarget(RenderTarget& renderTarget);
	virtual void setImGuiDrawData(ImDrawData* drawData);
	virtual void addPass(RenderPass& pass);
	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();

	static Serializable* instantiate(const SerializationInfo& from);
};