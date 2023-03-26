#include "DeferredRenderGraph.h"
#include "../RenderCore/DirectShadowRenderPack.h"
#include "../Engine.h"
#include "../Asset.h"

DeferredSurfaceBuffer::DeferredSurfaceBuffer()
{
	gBufferA.setAutoGenMip(false);
	gBufferB.setAutoGenMip(false);
	gBufferC.setAutoGenMip(false);
	gBufferD.setAutoGenMip(false);
	gBufferE.setAutoGenMip(false);
	hitDataMap.setAutoGenMip(false);
	hitColorMap.setAutoGenMip(false);
	debugBuffer.setAutoGenMip(false);
	renderTarget.addTexture("gBufferA", gBufferA);
	renderTarget.addTexture("gBufferB", gBufferB);
	renderTarget.addTexture("gBufferC", gBufferC);
	renderTarget.addTexture("gBufferD", gBufferD);
	renderTarget.addTexture("gBufferE", gBufferE);
	traceRenderTarget.addTexture("hitDataMap", hitDataMap);
	traceRenderTarget.addTexture("hitColorMap", hitColorMap);
	debugRenderTarget.addTexture("debugBuffer", debugBuffer);
}

void DeferredSurfaceBuffer::create(CameraRender* cameraRender)
{
	resolveRenderTarget.addTexture("gBufferA", *cameraRender->getSceneMap());
	resize(cameraRender->size.x, cameraRender->size.y);
	usedFrame = Time::frames();
}

void DeferredSurfaceBuffer::resize(unsigned int width, unsigned int height)
{
	renderTarget.resize(width, height);
	int widthP2 = 1 << max(int(log2(width)), 1);
	int heightP2 = 1 << max(int(log2(height)), 1);
	hizTexture.resize(widthP2, heightP2);
	traceRenderTarget.resize(width, height);
	resolveRenderTarget.resize(width, height);
	debugRenderTarget.resize(width, height);
}

void DeferredSurfaceBuffer::bind(IRenderContext& context)
{
}

RenderTarget* DeferredSurfaceBuffer::getRenderTarget()
{
	return &renderTarget;
}

Texture* DeferredSurfaceBuffer::getDepthTexture()
{
	return &gBufferB;
}

Texture* DeferredSurfaceBuffer::getGBufferA()
{
	return &gBufferA;
}

Texture* DeferredSurfaceBuffer::getGBufferB()
{
	return &gBufferB;
}

Texture* DeferredSurfaceBuffer::getGBufferC()
{
	return &gBufferC;
}

Texture* DeferredSurfaceBuffer::getGBufferD()
{
	return &gBufferD;
}

Texture* DeferredSurfaceBuffer::getGBufferE()
{
	return &gBufferE;
}

Texture* DeferredSurfaceBuffer::getHiZTexture()
{
	return &hizTexture;
}

Texture* DeferredSurfaceBuffer::getHitDataTexture()
{
	return &hitDataMap;
}

Texture* DeferredSurfaceBuffer::getHitColorTexture()
{
	return &hitColorMap;
}

RenderTarget* DeferredSurfaceBuffer::getTraceRenderTarget()
{
	return &traceRenderTarget;
}

RenderTarget* DeferredSurfaceBuffer::getResolveRenderTarget()
{
	return &resolveRenderTarget;
}

Texture* DeferredSurfaceBuffer::getDebugBuffer()
{
	return &debugBuffer;
}

RenderTarget* DeferredSurfaceBuffer::getDebugRenderTarget()
{
	return &debugRenderTarget;
}

SerializeInstance(DeferredRenderGraph);

DeferredRenderGraph::DeferredRenderGraph()
{
	defaultPreDepthSurfaceData.clearFlags = Clear_Depth;

	defaultGeometrySurfaceData.clearFlags = Clear_Colors | Clear_Stencil;
	defaultGeometrySurfaceData.clearColors.resize(5, { 0, 0, 0, 0 });
	defaultGeometrySurfaceData.clearColors[1] = { 1.0f, 1.0f, 1.0f, 1.0f };

	defaultLightingSurfaceData.clearFlags = Clear_All;

	forwardPass.minusClearFlags = Clear_All;

	screenHitPass.renderGraph = this;
	shadowDepthPass.renderGraph = this;
	preDepthPass.renderGraph = this;
	geometryPass.renderGraph = this;
	vsmDepthPass.renderGraph = this;
	lightingPass.renderGraph = this;
	hizPass.renderGraph = this;
	ssrPass.renderGraph = this;
	forwardPass.renderGraph = this;
	blitPass.renderGraph = this;
	imGuiPass.renderGraph = this;
}

ISurfaceBuffer* DeferredRenderGraph::newSurfaceBuffer()
{
	return new DeferredSurfaceBuffer();
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

	const bool enableVSMDepthPass = VirtualShadowMapConfig::isEnable();

	if (enableVSMDepthPass)
		vsmDepthPass.setRenderCommand(cmd);
	else
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
		ShaderProgram* lightingShader = NULL;

		if (enableVSMDepthPass) {
			Enum<ShaderFeature> lightingShaderFeature = shaderFeature;
			lightingShaderFeature |= Shader_Lighting | Shader_VSM;
			lightingShader = cmd.material->getShader()->getProgram(lightingShaderFeature, matchRule);
			if (lightingShader && !lightingShader->shaderType.has(Shader_VSM))
				lightingShader = NULL;
		}
		if (lightingShader == NULL) {
			Enum<ShaderFeature> lightingShaderFeature = shaderFeature;
			lightingShaderFeature |= Shader_Lighting;
			lightingShader = cmd.material->getShader()->getProgram(lightingShaderFeature, matchRule);
		}

		bool hasLightingPass = lightingShader && lightingShader->init();

		DeferredLightingTask lightingTask;
		lightingTask.sceneData = cmd.sceneData;
		lightingTask.program = lightingShader;
		lightingTask.material = cmd.material;
		lightingTask.surface = defaultLightingSurfaceData;

		for (auto& cameraRenderData : cmd.sceneData->cameraRenderDatas) {
			DeferredSurfaceBuffer* surfaceBuffer = dynamic_cast<DeferredSurfaceBuffer*>(cameraRenderData->surfaceBuffer);
			if (surfaceBuffer == NULL) {
				throw runtime_error("SurfaceBuffer is not allocated");
			}

			if (hasPreDepth) {
				preTask.cameraData = cameraRenderData;
				preTask.surface.renderTarget = &surfaceBuffer->renderTarget;
				preDepthPass.commandList.addRenderTask(preCommand, preTask);
			}

			geoTask.cameraData = cameraRenderData;
			geoTask.surface.renderTarget = &surfaceBuffer->renderTarget;
			geoTask.surface.clearColors[0] = cameraRenderData->cameraRender->clearColor;
			geometryPass.commandList.addRenderTask(cmd, geoTask);

			if (hasLightingPass) {
				lightingTask.gBufferRT = &surfaceBuffer->renderTarget;
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
	screenHitPass.prepare();
	preDepthPass.prepare();
	geometryPass.prepare();
	if (VirtualShadowMapConfig::isEnable())
		vsmDepthPass.prepare();
	else
		shadowDepthPass.prepare();
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

	preDepthPass.execute(context);

	timer.record("PreDepth");

	geometryPass.execute(context);

	timer.record("Base");

	if (VirtualShadowMapConfig::isEnable())
		vsmDepthPass.execute(context);
	else
		shadowDepthPass.execute(context);

	timer.record("ShadowDepth");

	context.clearFrameBindings();

	lightingPass.execute(context);

	timer.record("Lighting");

	clearTexFrameBindings();

	hizPass.execute(context);

	timer.record("HiZ");

	ssrPass.execute(context);

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
	for (auto sceneData : sceneDatas)
		sceneData->reset();
	sceneDatas.clear();

	screenHitPass.reset();
	preDepthPass.reset();
	geometryPass.reset();
	if (VirtualShadowMapConfig::isEnable())
		vsmDepthPass.reset();
	else
		shadowDepthPass.reset();
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
	passes.push_back(make_pair("PreDepth", &preDepthPass));
	passes.push_back(make_pair("Geometry", &geometryPass));
	passes.push_back(make_pair("ShadowDepth", VirtualShadowMapConfig::isEnable() ?
		(RenderPass*)&vsmDepthPass : (RenderPass*)&shadowDepthPass));
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
