#include "ForwardRenderGraph.h"
#include "../Engine.h"

SerializeInstance(ForwardRenderGraph);

bool ForwardRenderGraph::setRenderCommand(const IRenderCommand& cmd)
{
	if (cmd.sceneData)
		sceneDatas.insert(cmd.sceneData);
	return meshPass.commandList->setRenderCommand(cmd);
}

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
	pass.renderGraph = this;
	passes.push_back(&pass);
}

void ForwardRenderGraph::prepare()
{
	for (auto sceneData : sceneDatas)
		sceneData->create();
	meshPass.prepare();
	resolvePass.prepare();
	for (auto pass : passes)
		pass->prepare();
	imGuiPass.prepare();
}

void ForwardRenderGraph::execute(IRenderContext& context)
{
	for (auto sceneData : sceneDatas)
		sceneData->upload();
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
	for (auto sceneData : sceneDatas)
		sceneData->reset();
	sceneDatas.clear();

	meshPass.reset();
	resolvePass.reset();
	for (auto pass : passes) {
		pass->reset();
		pass->renderGraph = NULL;
	}
	imGuiPass.reset();
	passes.clear();
}

Serializable* ForwardRenderGraph::instantiate(const SerializationInfo& from)
{
	return new ForwardRenderGraph();
}
