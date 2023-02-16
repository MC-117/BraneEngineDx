#include "DeferredRenderGraph.h"
#include "../RenderCore/DirectShadowRenderPack.h"
#include "../Engine.h"
#include "../Asset.h"

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
	defaultPreDepthSurfaceData.clearFlags = Clear_Depth;

	defaultGeometrySurfaceData.clearFlags = Clear_Colors | Clear_Stencil;
	defaultGeometrySurfaceData.clearColors.resize(5, { 0, 0, 0, 0 });
	defaultGeometrySurfaceData.clearColors[1] = { 1.0f, 1.0f, 1.0f, 1.0f };

	defaultLightingSurfaceData.clearFlags = Clear_All;

	forwardPass.minusClearFlags = Clear_All;
}

DeferredRenderGraph::DeferredViewData* DeferredRenderGraph::getGBufferRT(CameraRender* cameraRender, SceneRenderData* sceneRenderData)
{
	DeferredViewData* rt = NULL;
	auto iter = viewDatas.find(cameraRender);
	if (iter == viewDatas.end()) {
		rt = new DeferredViewData();
		rt->sceneData = sceneRenderData;
		rt->cameraRender = cameraRender;
		rt->cameraData = cameraRender->getRenderData();
		rt->resolveRenderTarget.addTexture("gBufferA", *cameraRender->getSceneMap());
		viewDatas.insert(make_pair(cameraRender, rt));
	}
	else {
		iter->second->age = 0;
		rt = iter->second;
	}
	return rt;
}

bool DeferredRenderGraph::setRenderCommand(const IRenderCommand& cmd)
{
	if (!cmd.isValid())
		return false;

	if (cmd.sceneData)
		sceneDatas.insert(cmd.sceneData);

	const ScreenHitRenderCommand* screenHitRenderCommand = dynamic_cast<const ScreenHitRenderCommand*>(&cmd);
	if (screenHitRenderCommand)
		return screenHitPass.setRenderCommand(cmd);

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

		if (!deferredShader->init()) {
			return false;
		}

		// Pre depth pass
		if (preDepthMaterial == NULL) {
			preDepthMaterial = getAssetByPath<Material>("Engine/Shaders/Depth.mat");
		}

		const MeshRenderCommand* meshCommand = dynamic_cast<const MeshRenderCommand*>(&cmd);
		bool hasPreDepth = enablePreDepthPass && preDepthMaterial && meshCommand != NULL && meshCommand->hasPreDepth;

		ShaderProgram* preProgram = NULL;
		if (hasPreDepth) {
			preProgram = preDepthMaterial->getShader()->getProgram(cmd.getShaderFeature());
			hasPreDepth &= preProgram != NULL;
			hasPreDepth = preProgram->init();
		}

		RenderTask preTask;
		MeshRenderCommand preCommand;

		if (hasPreDepth) {
			MeshTransformIndex* transformIndex = NULL;
			for (int j = 0; j < meshCommand->instanceIDCount; j++)
				transformIndex = cmd.sceneData->setMeshPartTransform(
					meshCommand->mesh, preDepthMaterial, meshCommand->instanceID + j);

			preCommand.sceneData = cmd.sceneData;
			preCommand.transformData = cmd.transformData;
			preCommand.material = preDepthMaterial;
			preCommand.mesh = meshCommand->mesh;
			preCommand.transformIndex = transformIndex;
			preCommand.bindings = cmd.bindings;

			MaterialRenderData* materialRenderData = dynamic_cast<MaterialRenderData*>(preDepthMaterial->getRenderData());
			if (materialRenderData->usedFrame < (long long)Time::frames()) {
				materialRenderData->program = preProgram;
				materialRenderData->create();
				materialRenderData->upload();
				materialRenderData->usedFrame = Time::frames();
			}

			preTask.age = 0;
			preTask.sceneData = preCommand.sceneData;
			preTask.transformData = preCommand.transformData;
			preTask.shaderProgram = preProgram;
			preTask.surface = defaultPreDepthSurfaceData;
			preTask.renderMode = preCommand.getRenderMode();
			preTask.materialData = materialRenderData;
			preTask.meshData = meshData;
			preTask.extraData = preCommand.bindings;
		}

		// Deferred geometry pass
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

		RenderTask geoTask;
		geoTask.age = 0;
		geoTask.sceneData = cmd.sceneData;
		geoTask.transformData = cmd.transformData;
		geoTask.surface = defaultGeometrySurfaceData;
		if (!enablePreDepthPass)
			geoTask.surface.clearFlags |= Clear_Depth;
		geoTask.shaderProgram = deferredShader;
		geoTask.renderMode = cmd.getRenderMode();
		geoTask.materialData = deferredMaterialRenderData;
		geoTask.meshData = meshData;
		geoTask.extraData = cmd.bindings;

		// Lighting pass
		Enum<ShaderFeature> lightingShaderFeature = shaderFeature;
		lightingShaderFeature |= Shader_Lighting;
		ShaderProgram* lightingShader = cmd.material->getShader()->getProgram(lightingShaderFeature, matchRule);

		bool hasLightingPass = lightingShader && lightingShader->init();

		DeferredLightingTask lightingTask;
		lightingTask.sceneData = cmd.sceneData;
		lightingTask.program = lightingShader;
		lightingTask.material = cmd.material;
		lightingTask.surface = defaultLightingSurfaceData;

		for (auto& cameraRenderData : cmd.sceneData->cameraRenderDatas) {
			DeferredViewData* gBufferRT = getGBufferRT(cameraRenderData->cameraRender, cmd.sceneData);
			if (hasPreDepth) {
				preTask.cameraData = cameraRenderData;
				preTask.surface.renderTarget = &gBufferRT->renderTarget;
				preDepthPass.commandList.addRenderTask(preCommand, preTask);
			}

			geoTask.cameraData = cameraRenderData;
			geoTask.surface.renderTarget = &gBufferRT->renderTarget;
			geoTask.surface.clearColors[0] = cameraRenderData->cameraRender->clearColor;
			geometryPass.commandList.addRenderTask(cmd, geoTask);

			if (hasLightingPass) {
				lightingTask.gBufferRT = &gBufferRT->renderTarget;
				lightingTask.surface.renderTarget = cameraRenderData->surface.renderTarget;
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
	screenHitPass.prepare();
	shadowDepthPass.prepare();
	preDepthPass.prepare();
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
	auto clearTexFrameBindings = [&]() {
		context.clearFrameBindings();
		for (int i = 6; i < 16; i++) {
			context.bindTexture(NULL, Fragment_Shader_Stage, i, -1);
			context.bindTexture(NULL, Compute_Shader_Stage, i, -1);
		}
	};

	timer.reset();

	clearTexFrameBindings();

	context.waitSignalCPU();
	timer.record("WaitGPU");

	for (auto sceneData : sceneDatas)
		sceneData->upload();
	timer.record("SceneDataUpload");

	screenHitPass.execute(context);

	timer.record("ScreenHit");

	shadowDepthPass.execute(context);

	timer.record("ShadowDepth");

	preDepthPass.execute(context);

	timer.record("PreDepth");

	geometryPass.execute(context);

	timer.record("Base");

	context.clearFrameBindings();

	lightingPass.execute(context);

	timer.record("Lighting");

	clearTexFrameBindings();

	for (auto& item : viewDatas) {
		hizPass.depthTexture = &item.second->gBufferB;
		hizPass.hizTexture = &item.second->hizTexture;
		hizPass.execute(context);
	}

	timer.record("HiZ");

	for (auto& item : viewDatas) {
		Texture* sceneMap = item.second->cameraRender->getSceneMap();
		if (sceneMap->getArrayCount() > 1)
			continue;
		ssrPass.sceneData = item.second->sceneData;
		ssrPass.cameraData = item.second->cameraData;
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

	timer.record("SSR");

	clearTexFrameBindings();

	forwardPass.execute(context);

	context.setGPUSignal();
	context.clearFrameBindings();

	timer.record("Transparent");

	for (auto sceneData : sceneDatas) {
		for (auto& cameraRenderData : sceneData->cameraRenderDatas)
			context.resolveMultisampleFrame(cameraRenderData->surface.renderTarget->getVendorRenderTarget());
		sceneData->reflectionProbeDataPack.cubeMapPool.refreshCubePool(context);
		sceneData->envLightProbeDataPack.computeEnvLights(context);
	}

	timer.record("SceneFetch");

	for (auto pass : passes)
		pass->execute(context);
	blitPass.execute(context);

	timer.record("PostProcess");

	imGuiPass.execute(context);

	timer.record("ImGui");

	/*----- Vendor swap -----*/
	{
		if (!VendorManager::getInstance().getVendor().swap(Engine::engineConfig, Engine::windowContext))
			throw runtime_error("Vendor swap failed");
	}

	timer.record("Swap");

	Console::getTimer("RenderGraph") = timer;
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

	screenHitPass.reset();
	shadowDepthPass.reset();
	preDepthPass.reset();
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
	passes.push_back(make_pair("ScreenHit", &screenHitPass));
	passes.push_back(make_pair("ShadowDepth", &shadowDepthPass));
	passes.push_back(make_pair("PreDepth", &preDepthPass));
	passes.push_back(make_pair("Geometry", &geometryPass));
	passes.push_back(make_pair("Lighting", &lightingPass));
	passes.push_back(make_pair("HiZ", &hizPass));
	passes.push_back(make_pair("SSR", &ssrPass));
	passes.push_back(make_pair("Forward", &forwardPass));
	passes.push_back(make_pair("Blit", &blitPass));
	passes.push_back(make_pair("ImGui", &imGuiPass));
}

Serializable* DeferredRenderGraph::instantiate(const SerializationInfo& from)
{
	return new DeferredRenderGraph();
}
