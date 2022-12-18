#include "DeferredRenderGraph.h"
#include "../RenderCore/DirectShadowRenderPack.h"
#include "../Engine.h"

SerializeInstance(DeferredRenderGraph);

DeferredRenderGraph::DeferredViewData::DeferredViewData()
{
	gBufferA.setAutoGenMip(false);
	gBufferB.setAutoGenMip(false);
	gBufferC.setAutoGenMip(false);
	gBufferD.setAutoGenMip(false);
	gBufferE.setAutoGenMip(false);
	hitDataMap.setAutoGenMip(false);
	hitColorMap.setAutoGenMip(false);
	renderTarget.addTexture("gBufferA", gBufferA);
	renderTarget.addTexture("gBufferB", gBufferB);
	renderTarget.addTexture("gBufferC", gBufferC);
	renderTarget.addTexture("gBufferD", gBufferD);
	renderTarget.addTexture("gBufferE", gBufferE);
	traceRenderTarget.addTexture("hitDataMap", hitDataMap);
	traceRenderTarget.addTexture("hitColorMap", hitColorMap);
}

void DeferredRenderGraph::DeferredViewData::prepare()
{
	renderTarget.resize(cameraRender->size.x, cameraRender->size.y);
	int widthP2 = 1 << max(int(log2(cameraRender->size.x)), 1);
	int heightP2 = 1 << max(int(log2(cameraRender->size.y)), 1);
	hizTexture.resize(widthP2, heightP2);
	traceRenderTarget.resize(cameraRender->size.x, cameraRender->size.y);
	resolveRenderTarget.resize(cameraRender->size.x, cameraRender->size.y);
}

bool DeferredRenderGraph::DeferredViewData::isCubeFace() const
{
	return cameraRender != NULL && cameraRender->getSceneMap()->getArrayCount() > 1;
}

DeferredRenderGraph::DeferredRenderGraph()
{
	forwardPass.requireClearFrame = false;
}

DeferredRenderGraph::DeferredViewData* DeferredRenderGraph::getGBufferRT(CameraRender* cameraRender, SceneRenderData* sceneRenderData)
{
	DeferredViewData* rt = NULL;
	auto iter = viewDatas.find(cameraRender);
	if (iter == viewDatas.end()) {
		rt = new DeferredViewData();
		rt->sceneData = sceneRenderData;
		rt->cameraRender = cameraRender;
		rt->cameraData.cameraRender = cameraRender;
		rt->resolveRenderTarget.addTexture("gBufferA", *cameraRender->getSceneMap());
		viewDatas.insert(make_pair(cameraRender, rt));
	}
	else {
		iter->second->age = 0;
		rt = iter->second;
	}
	if (rt->cameraData.usedFrame < (long long)Time::frames()) {
		rt->cameraData.create();
		rt->cameraData.renderTarget = &rt->renderTarget;
		rt->cameraData.clearColors.resize(5, { 0, 0, 0, 0 });
		rt->cameraData.clearColors[0] = cameraRender->clearColor;
		rt->cameraData.clearColors[1] = { 1.0f, 1.0f, 1.0f, 1.0f };
		rt->cameraData.upload();
		rt->cameraData.usedFrame = Time::frames();
	}
	return rt;
}

bool DeferredRenderGraph::setRenderCommand(const IRenderCommand& cmd)
{
	if (!cmd.isValid())
		return false;

	if (cmd.sceneData)
		sceneDatas.insert(cmd.sceneData);

	ShaderMatchRule matchRule;
	matchRule.fragmentFlag = ShaderMatchFlag::Best;

	uint16_t renderStage = cmd.getRenderMode().getRenderStage();

	Enum<ShaderFeature> shaderFeature = cmd.getShaderFeature();
	Enum<ShaderFeature> deferredShaderFeature = shaderFeature;
	deferredShaderFeature |= Shader_Deferred;
	ShaderProgram* deferredShader = cmd.material->getShader()->getProgram(deferredShaderFeature, matchRule);

	shadowDepthPass.setRenderCommand(cmd);

	if (renderStage >= RS_Transparent || deferredShader == NULL) {
		forwardPass.commandList.setRenderCommand(cmd);
	}
	else {
		for (auto binding : cmd.bindings) {
			if (binding->usedFrame < (long long)Time::frames()) {
				binding->create();
				binding->upload();
				binding->usedFrame = Time::frames();
			}
		}

		MeshData* meshData = cmd.mesh == NULL ? NULL : cmd.mesh->meshData;
		if (meshData)
			meshData->init();

		// Deferred geometry pass
		if (!deferredShader->init()) {
			return false;
		}

		MaterialRenderData* deferredMaterialRenderData = dynamic_cast<MaterialRenderData*>(cmd.material->getRenderData());
		if (deferredMaterialRenderData == NULL) {
			return false;
		}
		if (deferredMaterialRenderData->usedFrame < (long long)Time::frames()) {
			deferredMaterialRenderData->program = deferredShader;
			deferredMaterialRenderData->create();
			deferredMaterialRenderData->upload();
			deferredMaterialRenderData->usedFrame = Time::frames();
		}

		RenderTask task;
		task.age = 0;
		task.sceneData = cmd.sceneData;
		task.transformData = cmd.transformData;
		task.shaderProgram = deferredShader;
		task.renderMode = cmd.getRenderMode();
		task.materialData = deferredMaterialRenderData;
		task.meshData = meshData;
		task.extraData = cmd.bindings;

		// Lighting pass
		Enum<ShaderFeature> lightingShaderFeature = shaderFeature;
		lightingShaderFeature |= Shader_Lighting;
		ShaderProgram* lightingShader = cmd.material->getShader()->getProgram(lightingShaderFeature, matchRule);

		bool hasLightingPass = lightingShader && lightingShader->init();

		DeferredLightingTask lightingTask;
		lightingTask.sceneData = cmd.sceneData;
		lightingTask.program = lightingShader;
		lightingTask.material = cmd.material;

		for (auto& cameraRenderData : cmd.sceneData->cameraRenderDatas) {
			DeferredViewData* gBufferRT = getGBufferRT(cameraRenderData->cameraRender, cmd.sceneData);
			task.cameraData = &gBufferRT->cameraData;
			geometryPass.commandList.addRenderTask(cmd, task);
			if (hasLightingPass) {
				lightingTask.gBufferRT = &gBufferRT->renderTarget;
				lightingTask.cameraRenderData = cameraRenderData;
				lightingPass.addTask(lightingTask);
			}
		}
	}
	return true;
}

void DeferredRenderGraph::setImGuiDrawData(ImDrawData* drawData)
{
	imGuiPass.setDrawData(drawData);
}

void DeferredRenderGraph::addPass(RenderPass& pass)
{
	pass.renderGraph = this;
	passes.push_back(&pass);
}

void DeferredRenderGraph::prepare()
{
	for (auto sceneData : sceneDatas)
		sceneData->create();
	for (auto& rt : viewDatas)
		rt.second->prepare();
	shadowDepthPass.prepare();
	geometryPass.prepare();
	lightingPass.prepare();
	hizPass.prepare();
	ssrPass.prepare();
	forwardPass.prepare();
	for (auto pass : passes)
		pass->prepare();
	blitPass.prepare();
	imGuiPass.prepare();
}

void DeferredRenderGraph::execute(IRenderContext& context)
{
	for (auto sceneData : sceneDatas)
		sceneData->upload();
	context.bindTexture(NULL, Fragment_Shader_Stage, 6, -1);
	context.bindTexture(NULL, Fragment_Shader_Stage, 7, -1);
	context.bindTexture(NULL, Fragment_Shader_Stage, 8, -1);
	context.bindTexture(NULL, Fragment_Shader_Stage, 9, -1);
	context.bindTexture(NULL, Fragment_Shader_Stage, 10, -1);
	context.bindTexture(NULL, Compute_Shader_Stage, 6, -1);
	context.bindTexture(NULL, Compute_Shader_Stage, 7, -1);
	context.bindTexture(NULL, Compute_Shader_Stage, 8, -1);
	context.bindTexture(NULL, Compute_Shader_Stage, 9, -1);
	context.bindTexture(NULL, Compute_Shader_Stage, 10, -1);
	shadowDepthPass.execute(context);
	geometryPass.execute(context);

	context.clearFrameBindings();

	lightingPass.execute(context);

	context.clearFrameBindings();
	context.bindTexture(NULL, Fragment_Shader_Stage, 6, -1);
	context.bindTexture(NULL, Fragment_Shader_Stage, 7, -1);
	context.bindTexture(NULL, Fragment_Shader_Stage, 8, -1);
	context.bindTexture(NULL, Fragment_Shader_Stage, 9, -1);
	context.bindTexture(NULL, Fragment_Shader_Stage, 10, -1);
	context.bindTexture(NULL, Compute_Shader_Stage, 6, -1);
	context.bindTexture(NULL, Compute_Shader_Stage, 7, -1);
	context.bindTexture(NULL, Compute_Shader_Stage, 8, -1);
	context.bindTexture(NULL, Compute_Shader_Stage, 9, -1);
	context.bindTexture(NULL, Compute_Shader_Stage, 10, -1);

	for (auto& item : viewDatas) {
		hizPass.depthTexture = &item.second->gBufferB;
		hizPass.hizTexture = &item.second->hizTexture;
		hizPass.execute(context);
	}

	for (auto& item : viewDatas) {
		Texture* sceneMap = item.second->cameraRender->getSceneMap();
		if (sceneMap->getArrayCount() > 1)
			continue;
		ssrPass.sceneData = item.second->sceneData;
		ssrPass.cameraData = &item.second->cameraData;
		ssrPass.gBufferA = sceneMap;
		ssrPass.gBufferB = &item.second->gBufferB;
		ssrPass.gBufferC = &item.second->gBufferC;
		ssrPass.gBufferE = &item.second->gBufferE;
		ssrPass.hiZMap = &item.second->hizTexture;
		ssrPass.hitDataMap = &item.second->hitDataMap;
		ssrPass.hitColorMap = &item.second->hitColorMap;
		ssrPass.traceRenderTarget = &item.second->traceRenderTarget;
		ssrPass.resolveRenderTarget = &item.second->resolveRenderTarget;
		ssrPass.execute(context);
	}

	context.clearFrameBindings();
	context.bindTexture(NULL, Fragment_Shader_Stage, 6, -1);
	context.bindTexture(NULL, Fragment_Shader_Stage, 7, -1);
	context.bindTexture(NULL, Fragment_Shader_Stage, 8, -1);
	context.bindTexture(NULL, Fragment_Shader_Stage, 9, -1);
	context.bindTexture(NULL, Fragment_Shader_Stage, 10, -1);
	context.bindTexture(NULL, Compute_Shader_Stage, 6, -1);
	context.bindTexture(NULL, Compute_Shader_Stage, 7, -1);
	context.bindTexture(NULL, Compute_Shader_Stage, 8, -1);
	context.bindTexture(NULL, Compute_Shader_Stage, 9, -1);
	context.bindTexture(NULL, Compute_Shader_Stage, 10, -1);

	for (auto sceneData : sceneDatas)
		sceneData->reflectionProbeDataPack.cubeMapPool.refreshCubePool(context);

	forwardPass.execute(context);
	context.clearFrameBindings();

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

void DeferredRenderGraph::reset()
{
	for (auto b = viewDatas.begin(), e = viewDatas.end(); b != e;) {
		b->second->age++;
		if (b->second->age >= 2) {
			delete b->second;
			b = viewDatas.erase(b);
		}
		else b++;
	}

	for (auto sceneData : sceneDatas)
		sceneData->reset();
	sceneDatas.clear();

	shadowDepthPass.reset();
	geometryPass.reset();
	lightingPass.reset();
	hizPass.reset();
	ssrPass.reset();
	forwardPass.reset();
	for (auto pass : passes) {
		pass->reset();
		pass->renderGraph = NULL;
	}
	blitPass.reset();
	imGuiPass.reset();
	passes.clear();
}

void DeferredRenderGraph::getPasses(vector<pair<string, RenderPass*>>& passes)
{
	passes.push_back(make_pair("Geometry", &geometryPass));
	passes.push_back(make_pair("Lighting", &lightingPass));
	passes.push_back(make_pair("HiZ", &hizPass));
	passes.push_back(make_pair("SSR", &ssrPass));
	passes.push_back(make_pair("Forward", &forwardPass));
	int i = 0;
	for (auto& pass : this->passes) {
		passes.push_back(make_pair("Pass_" + to_string(i), pass));
	}
	passes.push_back(make_pair("Blit", &blitPass));
	passes.push_back(make_pair("ImGui", &imGuiPass));
}

Serializable* DeferredRenderGraph::instantiate(const SerializationInfo& from)
{
	return new DeferredRenderGraph();
}
