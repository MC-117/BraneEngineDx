#include "ForwardRenderGraph.h"
#include "../Engine.h"

SerializeInstance(ForwardRenderGraph);

bool ForwardRenderGraph::setRenderCommand(const IRenderCommand& cmd)
{
	if (cmd.sceneData)
		sceneDatas.insert(cmd.sceneData);
	shadowDepthPass.setRenderCommand(cmd);
	return meshPass.commandList.setRenderCommand(cmd);
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
	shadowDepthPass.prepare();
	meshPass.prepare();
	for (auto pass : passes)
		pass->prepare();
	blitPass.prepare();
	imGuiPass.prepare();
}

void ForwardRenderGraph::execute(IRenderContext& context)
{
	for (auto sceneData : sceneDatas)
		sceneData->upload();
	shadowDepthPass.execute(context);
	meshPass.execute(context);
	for (auto sceneData : sceneDatas)
		for (auto& cameraRenderData : sceneData->cameraRenderDatas)
			context.resolveMultisampleFrame(cameraRenderData->renderTarget->getVendorRenderTarget());
	for (auto pass : passes)
		pass->execute(context);
	blitPass.execute(context);
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

	shadowDepthPass.reset();
	meshPass.reset();
	for (auto pass : passes) {
		pass->reset();
		pass->renderGraph = NULL;
	}
	blitPass.reset();
	imGuiPass.reset();
	passes.clear();
}

Serializable* ForwardRenderGraph::instantiate(const SerializationInfo& from)
{
	return new ForwardRenderGraph();
}
