#include "DeferredRenderGraph.h"

#include "../RenderCore/DirectShadowRenderPack.h"
#include "../Engine.h"
#include "../Asset.h"
#include "../Profile/RenderProfile.h"

DeferredSurfaceBuffer::DeferredSurfaceBuffer()
	: gBufferA(1280, 720, 4, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RGB10A2_UF })
	, gBufferB(1280, 720, 1, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_R32_F })
	, gBufferC(1280, 720, 3, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RGB10A2_UF })
	, gBufferD(1280, 720, 4, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RGBA8_F })
	, gBufferE(1280, 720, 4, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RGBA8_UI })
	, gBufferF(1280, 720, 4, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RGBA8_F })
	, renderTarget(1280, 720, 4, true)
	, hizTexture(1280, 720, 1, false, { TW_Border, TW_Border, TF_Point, TF_Point, TIT_R32_F, { 255, 255, 255, 255 } })
	, hitDataMap(1280, 720, 4, false, { TW_Clamp, TW_Clamp, TF_Linear, TF_Linear, TIT_RGBA16_FF })
	, hitColorMap(1280, 720, 4, false, { TW_Clamp, TW_Clamp, TF_Linear, TF_Linear, TIT_RGBA8_UF })
	, traceRenderTarget(1280, 720, 4)
	, resolveRenderTarget(1280, 720, 4)
	, sceneColorMips(1280, 720, 4, false, { TW_Clamp, TW_Clamp, TF_Linear_Mip_Linear, TF_Linear_Mip_Linear, TIT_RGB10A2_UF })
	, debugBuffer(1280, 720, 1, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_RGBA8_F })
	, debugRenderTarget(1280, 720, 1)
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
	renderTarget.addTexture("gBufferF", gBufferF);
	traceRenderTarget.addTexture("hitDataMap", hitDataMap);
	traceRenderTarget.addTexture("hitColorMap", hitColorMap);
	debugRenderTarget.addTexture("debugBuffer", debugBuffer);
}

void DeferredSurfaceBuffer::create(CameraRenderData* cameraRenderData)
{
	resolveRenderTarget.addTexture("gBufferA", *cameraRenderData->sceneTexture);
	resize(cameraRenderData->data.viewSize.x(), cameraRenderData->data.viewSize.y());
	usedFrame = Time::frames();
}

void DeferredSurfaceBuffer::resize(unsigned int width, unsigned int height)
{
	renderTarget.resize(width, height);
	int widthMips = max(int(log2(width)), 1);
	int heightMips = max(int(log2(height)), 1);
	int widthP2 = 1 << widthMips;
	int heightP2 = 1 << heightMips;
	hizTexture.resize(widthP2, heightP2);
	traceRenderTarget.resize(width, height);
	resolveRenderTarget.resize(width, height);
	debugRenderTarget.resize(width, height);

	int mipLevel = max(widthMips, heightMips);
	mipLevel = min(mipLevel, 5);
	Texture2DInfo info = sceneColorMips.getTextureInfo();
	info.sampleCount = mipLevel;
	sceneColorMips.setTextureInfo(info);
	sceneColorMips.resize(width / 2, height / 2);
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

Texture* DeferredSurfaceBuffer::getGBufferF()
{
	return &gBufferF;
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

Texture* DeferredSurfaceBuffer::getSceneColorMips()
{
	return &sceneColorMips;
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
	defaultPreDepthSurfaceData.clearFlags = Clear_Depth | Clear_Stencil;;

	defaultGeometrySurfaceData.clearFlags = Clear_Colors;
	defaultGeometrySurfaceData.clearColors.resize(5, { 0, 0, 0, 0 });
	defaultGeometrySurfaceData.clearColors[1] = { 1.0f, 1.0f, 1.0f, 1.0f };

	defaultLightingSurfaceData.clearFlags = Clear_All;

	translucentPass.minusClearFlags = Clear_All;

	screenHitPass.renderGraph = this;
	shadowDepthPass.renderGraph = this;
	preDepthPass.renderGraph = this;
	buildProbeGridPass.renderGraph = this;
	geometryPass.renderGraph = this;
	vsmDepthPass.renderGraph = this;
	lightingPass.renderGraph = this;
	hizPass.renderGraph = this;
	genMipPass.renderGraph = this;
	ssrPass.renderGraph = this;
	translucentPass.renderGraph = this;
	blitPass.renderGraph = this;
	imGuiPass.renderGraph = this;
}

bool DeferredRenderGraph::loadDefaultResource()
{
	if (preDepthMaterialRenderData == NULL) {
		Material* preDepthMaterial = getAssetByPath<Material>("Engine/Shaders/Depth.mat");
		preDepthMaterialRenderData = preDepthMaterial->getMaterialRenderData();
	}
	if (preDepthMaterialRenderData == NULL)
		return false;
	renderDataCollectorMainThread.add(*preDepthMaterialRenderData);
	if (!screenHitPass.loadDefaultResource())
		return false;
	if (!shadowDepthPass.loadDefaultResource())
		return false;
	if (!preDepthPass.loadDefaultResource())
		return false;
	if (!buildProbeGridPass.loadDefaultResource())
		return false;
	if (!geometryPass.loadDefaultResource())
		return false;
	if (!vsmDepthPass.loadDefaultResource())
		return false;
	if (!lightingPass.loadDefaultResource())
		return false;
	if (!hizPass.loadDefaultResource())
		return false;
	if (!genMipPass.loadDefaultResource())
		return false;
	if (!ssrPass.loadDefaultResource())
		return false;
	if (!translucentPass.loadDefaultResource())
		return false;
	if (!blitPass.loadDefaultResource())
		return false;
	if (!imGuiPass.loadDefaultResource())
		return false;
	return true;
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

	uint16_t renderStage = cmd.getRenderStage();

	Enum<ShaderFeature> shaderFeature = cmd.getShaderFeature();
	Enum<ShaderFeature> deferredShaderFeature = shaderFeature;
	deferredShaderFeature |= Shader_Deferred;
	Enum<ShaderFeature> preDepthShaderFeature = shaderFeature;
	preDepthShaderFeature |= Shader_Depth;
	IMaterial* deferredMaterial = cmd.materialRenderData->getVariant(deferredShaderFeature, matchRule);

	const bool enableVSMDepthPass = VirtualShadowMapConfig::isEnable();

	if (enableVSMDepthPass)
		vsmDepthPass.setRenderCommand(cmd);
	else
		shadowDepthPass.setRenderCommand(cmd);

	if (renderStage >= RS_Transparent || deferredMaterial == NULL) {
		translucentPass.setRenderCommand(cmd);
	}
	else {
		MeshData* meshData = cmd.mesh == NULL ? NULL : cmd.mesh->meshData;
		if (meshData)
			meshData->init();

		if (!deferredMaterial->init()) {
			return false;
		}

		// Pre depth pass

		const MeshRenderCommand* meshCommand = dynamic_cast<const MeshRenderCommand*>(&cmd);
		
		bool hasPreDepth = enablePreDepthPass && preDepthMaterialRenderData && preDepthMaterialRenderData->isValid() && meshCommand != NULL && meshCommand->hasPreDepth;
		bool hasGeometryPass = meshCommand->hasGeometryPass;
		
		IMaterial* preMaterial = NULL;
		bool useSharedPreMaterial = false;
		if (hasPreDepth) {
			preMaterial = cmd.materialRenderData->getVariant(preDepthShaderFeature, matchRule);
			if (preMaterial == NULL) {
				preMaterial = preDepthMaterialRenderData->getVariant(shaderFeature);
				useSharedPreMaterial = true;
			}
			hasPreDepth &= preMaterial != NULL;
			hasPreDepth = preMaterial->init();
		}

		RenderTask preTask;
		MeshRenderCommand preCommand;

		if (hasPreDepth) {
			MeshBatchDrawCall* sharedBatchDrawCall = NULL;
			if (useSharedPreMaterial) {
				for (int j = 0; j < meshCommand->instanceIDCount; j++)
				{
					unsigned int transformID = meshCommand->instanceID + j;
					IMeshBatchDrawCommandArray* drawCommandArray = dynamic_cast<IMeshBatchDrawCommandArray*>(meshCommand->batchDrawData.batchDrawCommandArray);
					const MeshBatchDrawKey renderKey(meshCommand->mesh, preDepthMaterialRenderData, meshCommand->reverseCullMode);
					sharedBatchDrawCall = drawCommandArray->setMeshBatchDrawCall(renderKey, transformID);
				}
			}

			preCommand.sceneData = cmd.sceneData;
			preCommand.batchDrawData = cmd.batchDrawData;
			preCommand.materialRenderData = cmd.materialRenderData;
			preCommand.mesh = meshCommand->mesh;
			preCommand.meshBatchDrawCall = useSharedPreMaterial ? sharedBatchDrawCall : meshCommand->meshBatchDrawCall;
			preCommand.bindings = cmd.bindings;

			preTask.age = 0;
			preTask.sceneData = preCommand.sceneData;
			preTask.batchDrawData = preCommand.batchDrawData;
			preTask.shaderProgram = preMaterial->program;
			preTask.materialVariant = preMaterial;
			preTask.surface = defaultPreDepthSurfaceData;
			preTask.meshData = meshData;
			preTask.extraData = preCommand.bindings;
		}

		// Deferred geometry pass
		MaterialRenderData* deferredMaterialRenderData = cmd.materialRenderData;
		if (deferredMaterialRenderData == NULL) {
			return false;
		}

		RenderTask geoTask;

		if (hasGeometryPass) {
			geoTask.age = 0;
			geoTask.sceneData = cmd.sceneData;
			geoTask.batchDrawData = cmd.batchDrawData;
			geoTask.surface = defaultGeometrySurfaceData;
			if (!enablePreDepthPass)
				geoTask.surface.clearFlags |= Clear_Depth;
			geoTask.shaderProgram = deferredMaterial->program;
			geoTask.materialVariant = deferredMaterial;
			geoTask.meshData = meshData;
			geoTask.extraData = cmd.bindings;
		}

		// Lighting pass
		IMaterial* lightingMaterial = NULL;

		if (enableVSMDepthPass) {
			Enum<ShaderFeature> lightingShaderFeature = shaderFeature;
			lightingShaderFeature |= Shader_Lighting | Shader_VSM;
			lightingMaterial = cmd.materialRenderData->getVariant(lightingShaderFeature, matchRule);
			if (lightingMaterial && !lightingMaterial->program->shaderType.has(Shader_VSM))
				lightingMaterial = NULL;
		}
		if (lightingMaterial == NULL) {
			Enum<ShaderFeature> lightingShaderFeature = shaderFeature;
			lightingShaderFeature |= Shader_Lighting;
			lightingMaterial = cmd.materialRenderData->getVariant(lightingShaderFeature, matchRule);
		}

		bool hasLightingPass = lightingMaterial && lightingMaterial->init();

		DeferredLightingTask lightingTask;
		lightingTask.sceneData = cmd.sceneData;
		lightingTask.program = lightingMaterial->program;
		lightingTask.materialVariant = lightingMaterial;
		lightingTask.surface = defaultLightingSurfaceData;

		for (auto& cameraRenderData : cmd.sceneData->cameraRenderDatas) {
			DeferredSurfaceBuffer* surfaceBuffer = dynamic_cast<DeferredSurfaceBuffer*>(cameraRenderData->surfaceBuffer);
			if (surfaceBuffer == NULL) {
				throw runtime_error("SurfaceBuffer is not allocated");
			}

			if (hasPreDepth) {
				preTask.renderMode = preCommand.getRenderMode("PreDepth"_N, cameraRenderData);
				preTask.cameraData = cameraRenderData;
				preTask.surface.renderTarget = &surfaceBuffer->renderTarget;
				preDepthPass.commandList.addRenderTask(preCommand, preTask);
			}

			if (hasGeometryPass) {
				geoTask.renderMode = cmd.getRenderMode("Geometry"_N, cameraRenderData);
				geoTask.cameraData = cameraRenderData;
				geoTask.surface.renderTarget = &surfaceBuffer->renderTarget;
				geoTask.surface.clearColors[0] = cameraRenderData->surface.clearColors[0];
				geometryPass.commandList.addRenderTask(cmd, geoTask);
			}

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
	if (preDepthMaterialRenderData)
		renderDataCollectorRenderThread.add(*preDepthMaterialRenderData);
	for (auto sceneData : sceneDatas)
		sceneData->create();
	screenHitPass.prepare();
	preDepthPass.prepare();
	buildProbeGridPass.prepare();
	geometryPass.prepare();
	if (VirtualShadowMapConfig::isEnable())
		vsmDepthPass.prepare();
	else
		shadowDepthPass.prepare();
	lightingPass.prepare();
	hizPass.prepare();
	genMipPass.prepare();
	ssrPass.prepare();
	translucentPass.prepare();
	for (auto pass : passes)
		pass->prepare();
	blitPass.prepare();
	imGuiPass.prepare();
}

// bool captureVSMTrigger = false;

void DeferredRenderGraph::execute(IRenderContext& context, long long renderFrame)
{
	auto clearTexFrameBindings = [&]() {
		context.clearFrameBindings();
		for (int i = 0; i <= 16; i++) {
			context.bindTexture(NULL, Fragment_Shader_Stage, i, -1);
			context.bindTexture(NULL, Compute_Shader_Stage, i, -1);
		}
	};

	{
		RENDER_SCOPE(context, WaitGPU)
		clearTexFrameBindings();
		context.clearVertexBindings();

		context.waitSignalCPU();
	}

	{
		RENDER_SCOPE(context, RenderDataUpload)
		renderDataCollectorRenderThread.updateRenderThread(renderFrame);
	}

	{
		RENDER_SCOPE(context, SceneDataUpload)
		for (auto sceneData : sceneDatas) {
			sceneData->upload();
			sceneData->debugRenderData.initBuffer(context, false);
		}
	}

	{
		RENDER_SCOPE(context, SceneViewCulling)
		for (auto sceneData : sceneDatas) {
			sceneData->executeViewCulling(context);
		}
	}

	{
		RENDER_SCOPE(context, ScreenHit)
		screenHitPass.execute(context);
	}

	{
		RENDER_SCOPE(context, PreDepth)
		preDepthPass.execute(context);
	}

	{
		RENDER_SCOPE(context, BuildProbeGrid)
		buildProbeGridPass.execute(context);
	}

	{
		RENDER_SCOPE(context, Base)
		geometryPass.execute(context);
	}

	{
		RENDER_SCOPE(context, ShadowDepth)
		// if (captureVSMTrigger)
		// {
		// 	ProfilerManager::instance().beginScope("VSM");
		// }

		if (VirtualShadowMapConfig::isEnable())
			vsmDepthPass.execute(context);
		else
			shadowDepthPass.execute(context);
	}

	{
		RENDER_SCOPE(context, Lighting)
		context.clearFrameBindings();

		lightingPass.execute(context);
	}

	{
		RENDER_SCOPE(context, HiZ)
		// if (captureVSMTrigger)
		// {
		// 	ProfilerManager::instance().endScope();
		// 	captureVSMTrigger = false;
		// }

		clearTexFrameBindings();

		hizPass.execute(context);
	}

	{
		RENDER_SCOPE(context, GenMip)
		genMipPass.execute(context);
	}

	{
		RENDER_SCOPE(context, SSR)
		ssrPass.execute(context);
	}

	{
		RENDER_SCOPE(context, DebugDraw)
		clearTexFrameBindings();

		for (auto sceneData : sceneDatas) {
			for (auto& cameraRenderData : sceneData->cameraRenderDatas) {
				if (!cameraRenderData->flags.has(CameraRender_SceneCapture)) {
					if (cameraRenderData->flags.has(CameraRender_DebugDraw))
						sceneData->debugRenderData.debugDraw(context, *cameraRenderData);
					if (cameraRenderData->flags.has(CameraRender_GizmoDraw))
						sceneData->debugRenderData.gizmoDraw(context, *cameraRenderData);
				}
			}
		}
	}

	{
		RENDER_SCOPE(context, Transparent)
		translucentPass.execute(context);

		context.setGPUSignal();
		context.clearFrameBindings();
	}

	{
		RENDER_SCOPE(context, SceneFetch)
		for (auto sceneData : sceneDatas) {
			for (auto& cameraRenderData : sceneData->cameraRenderDatas)
				context.resolveMultisampleFrame(cameraRenderData->surface.renderTarget->getVendorRenderTarget());
			sceneData->reflectionProbeDataPack.cubeMapPool.refreshCubePool(context);
			sceneData->envLightProbeDataPack.computeEnvLights(context);
		}
	}

	{
		RENDER_SCOPE(context, PostProcess)
		for (auto pass : passes)
			pass->execute(context);
		blitPass.execute(context);
	}

	{
		RENDER_SCOPE(context, ImGui)
		imGuiPass.execute(context);
	}

	{
		RENDER_SCOPE(context, Swap)
		Engine::getMainDeviceSurface()->swapBuffer(Engine::engineConfig.vsnyc, Engine::engineConfig.maxFPS);
	}
}

void DeferredRenderGraph::reset()
{
	renderDataCollectorMainThread.clear();
	renderDataCollectorRenderThread.clear();
	for (auto sceneData : sceneDatas)
		sceneData->reset();
	sceneDatas.clear();

	screenHitPass.reset();
	preDepthPass.reset();
	buildProbeGridPass.reset();
	geometryPass.reset();
	if (VirtualShadowMapConfig::isEnable())
		vsmDepthPass.reset();
	else
		shadowDepthPass.reset();
	lightingPass.reset();
	hizPass.reset();
	genMipPass.reset();
	ssrPass.reset();
	translucentPass.reset();
	for (auto pass : passes) {
		pass->reset();
		pass->renderGraph = NULL;
	}
	blitPass.reset();
	imGuiPass.reset();
	passes.clear();
}

void DeferredRenderGraph::triggerPersistentDebugDraw()
{
	for (auto sceneData : sceneDatas) {
		sceneData->debugRenderData.triggerPersistentDebugDraw();
	}
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
	passes.push_back(make_pair("GenMip", &genMipPass));
	passes.push_back(make_pair("SSR", &ssrPass));
	passes.push_back(make_pair("Translucent", &translucentPass));
	passes.push_back(make_pair("Blit", &blitPass));
	passes.push_back(make_pair("ImGui", &imGuiPass));
}

IRenderDataCollector* DeferredRenderGraph::getRenderDataCollectorMainThread()
{
	return &renderDataCollectorMainThread;
}

IRenderDataCollector* DeferredRenderGraph::getRenderDataCollectorRenderThread()
{
	return &renderDataCollectorRenderThread;
}

Serializable* DeferredRenderGraph::instantiate(const SerializationInfo& from)
{
	return new DeferredRenderGraph();
}
