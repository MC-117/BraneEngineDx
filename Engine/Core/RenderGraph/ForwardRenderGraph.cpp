#include "ForwardRenderGraph.h"
#include "../Engine.h"

SerializeInstance(ForwardRenderGraph);

ISurfaceBuffer* ForwardRenderGraph::newSurfaceBuffer()
{
	return nullptr;
}

bool ForwardRenderGraph::setRenderCommand(const IRenderCommand& cmd)
{
	if (cmd.sceneData)
		sceneDatas.insert(cmd.sceneData);
	shadowDepthPass.setRenderCommand(cmd);
	return meshPass.commandList.setRenderCommand(cmd, renderDataCollector);
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
	renderDataCollector.upload();
	for (auto sceneData : sceneDatas)
		sceneData->upload();
	shadowDepthPass.execute(context);
	meshPass.execute(context);
	for (auto sceneData : sceneDatas)
		for (auto& cameraRenderData : sceneData->cameraRenderDatas)
			context.resolveMultisampleFrame(cameraRenderData->surface.renderTarget->getVendorRenderTarget());
	for (auto pass : passes)
		pass->execute(context);
	blitPass.execute(context);
	imGuiPass.execute(context);

	Engine::getMainDeviceSurface()->swapBuffer(Engine::engineConfig.vsnyc, Engine::engineConfig.maxFPS);
}

void ForwardRenderGraph::reset()
{
	renderDataCollector.clear();
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

IRenderDataCollector* ForwardRenderGraph::getRenderDataCollector()
{
	return &renderDataCollector;
}

Serializable* ForwardRenderGraph::instantiate(const SerializationInfo& from)
{
	return new ForwardRenderGraph();
}