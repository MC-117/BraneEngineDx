#include "ForwardRenderGraph.h"
#include "../Engine.h"

SerializeInstance(ForwardRenderGraph);

void ForwardRenderGraph::setRenderCommandList(RenderCommandList& commandList)
{
	meshPass.commandList = &commandList;
}

void ForwardRenderGraph::setMainRenderTarget(RenderTarget& renderTarget)
{
	resolvePass.renderTarget = &renderTarget;
}

void ForwardRenderGraph::setImGuiDrawData(ImDrawData* drawData)
{
	imGuiPass.setDrawData(drawData);
}

void ForwardRenderGraph::addPass(RenderPass& pass)
{
	passes.push_back(&pass);
}

void ForwardRenderGraph::prepare()
{
	meshPass.prepare();
	resolvePass.prepare();
	for (auto pass : passes)
		pass->prepare();
	imGuiPass.prepare();
}

void ForwardRenderGraph::execute(IRenderContext& context)
{
	meshPass.execute(context);
	resolvePass.execute(context);
	for (auto pass : passes)
		pass->execute(context);
	imGuiPass.execute(context);

	/*----- Vendor swap -----*/
	{
		if (!VendorManager::getInstance().getVendor().swap(Engine::engineConfig, Engine::windowContext))
			throw runtime_error("Vendor swap failed");
	}
}

void ForwardRenderGraph::reset()
{
	passes.clear();
}

Serializable* ForwardRenderGraph::instantiate(const SerializationInfo& from)
{
	return new ForwardRenderGraph();
}
