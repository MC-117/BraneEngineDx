#include "DeferredRenderGraph.h"
#include "../Engine.h"

SerializeInstance(DeferredRenderGraph);

DeferredRenderGraph::GBufferRT::GBufferRT()
{
	gBufferA.setAutoGenMip(false);
	gBufferB.setAutoGenMip(false);
	gBufferC.setAutoGenMip(false);
	gBufferD.setAutoGenMip(false);
	gBufferE.setAutoGenMip(false);
	renderTarget.addTexture("gBufferA", gBufferA);
	renderTarget.addTexture("gBufferB", gBufferB);
	renderTarget.addTexture("gBufferC", gBufferC);
	renderTarget.addTexture("gBufferD", gBufferD);
	renderTarget.addTexture("gBufferE", gBufferE);
}

void DeferredRenderGraph::GBufferRT::prepare()
{
	renderTarget.resize(camera->size.x, camera->size.y);
}

DeferredRenderGraph::DeferredRenderGraph()
{
	transparentPass.commandList = &transparentList;
	transparentPass.requireClearFrame = false;
}

DeferredRenderGraph::GBufferRT* DeferredRenderGraph::getGBufferRT(Camera* camera)
{
	GBufferRT* rt = NULL;
	auto iter = gBufferRTs.find(camera);
	if (iter == gBufferRTs.end()) {
		rt = new GBufferRT();
		rt->camera = camera;
		rt->cameraData.camera = camera;
		gBufferRTs.insert(make_pair(camera, rt));
	}
	else {
		iter->second->age = 0;
		rt = iter->second;
	}
	if (rt->cameraData.usedFrame < (long long)Time::frames()) {
		rt->cameraData.create();
		rt->cameraData.renderTarget = &rt->renderTarget;
		rt->cameraData.clearColors.resize(5, { 0, 0, 0, 0 });
		rt->cameraData.clearColors[0] = camera->clearColor;
		rt->cameraData.clearColors[1] = { 1.0f, 1.0f, 1.0f, 1.0f };
		rt->cameraData.usedFrame = Time::frames();
	}
	return rt;
}

string getShaderFeatureName(Enum<ShaderFeature> feature)
{
	if (feature.enumValue == Shader_Default)
		return "[default]";
	string name = "";
	if (feature.has(Shader_Custom)) {
		if (feature.has(Shader_Custom_1))
			name += "[custom1]";
		if (feature.has(Shader_Custom_2))
			name += "[custom2]";
		if (feature.has(Shader_Custom_3))
			name += "[custom3]";
		if (feature.has(Shader_Custom_4))
			name += "[custom4]";
		if (feature.has(Shader_Custom_5))
			name += "[custom5]";
		if (feature.has(Shader_Custom_6))
			name += "[custom6]";
		if (feature.has(Shader_Custom_7))
			name += "[custom7]";
		if (feature.has(Shader_Custom_8))
			name += "[custom8]";
	}
	else {
		if (feature.has(Shader_Deferred))
			name += "[deferred]";
		if (feature.has(Shader_Lighting))
			name += "[lighting]";
		if (feature.has(Shader_Postprocess))
			name += "[postprocess]";
		if (feature.has(Shader_Skeleton))
			name += "[skeleton]";
		if (feature.has(Shader_Morph))
			name += "[morph]";
		if (feature.has(Shader_Particle))
			name += "[particle]";
		if (feature.has(Shader_Modifier))
			name += "[modifier]";
		if (feature.has(Shader_Terrain))
			name += "[terrain]";
	}
	return name;
}

bool DeferredRenderGraph::setRenderCommand(const IRenderCommand& cmd)
{
	if (!cmd.isValid())
		return false;

	if (cmd.sceneData)
		sceneDatas.insert(cmd.sceneData);

	const MeshRenderCommand* meshRenderCommand = dynamic_cast<const MeshRenderCommand*>(&cmd);
	uint16_t renderStage = cmd.getRenderMode().getRenderStage();

	if (renderStage >= RS_Transparent) {
		transparentList.setRenderCommand(cmd);
	}
	else if (meshRenderCommand == NULL) {
		geometryPass.commandList->setRenderCommand(cmd);
	}
	else {
		GBufferRT* gBufferRT = getGBufferRT(cmd.camera);

		CameraRenderData* cameraRenderData = dynamic_cast<CameraRenderData*>(cmd.camera->getRenderData());
		if (cameraRenderData == NULL)
			return false;
		if (cameraRenderData->usedFrame < (long long)Time::frames()) {
			cameraRenderData->create();
			cameraRenderData->usedFrame = Time::frames();
		}

		for (auto binding : cmd.bindings) {
			if (binding->usedFrame < (long long)Time::frames()) {
				binding->create();
				binding->usedFrame = Time::frames();
			}
		}

		MeshData* meshData = cmd.mesh == NULL ? NULL : cmd.mesh->meshData;
		if (meshData)
			meshData->init();

		Enum<ShaderFeature> shaderFeature = cmd.getShaderFeature();
		Enum<ShaderFeature> deferredShaderFeature = shaderFeature;
		deferredShaderFeature |= Shader_Deferred;
		ShaderProgram* deferredShader = cmd.material->getShader()->getProgram(deferredShaderFeature);
		if (deferredShader == NULL) {
			Console::warn("Shader %s don't have mode %s",
				cmd.material->getShaderName().c_str(),
				getShaderFeatureName(deferredShaderFeature.enumValue).c_str());
			return false;
		}

		Enum<ShaderFeature> lightingShaderFeature = shaderFeature;
		lightingShaderFeature |= Shader_Lighting;
		ShaderProgram* lightingShader = cmd.material->getShader()->getProgram(lightingShaderFeature);
		if (lightingShader == NULL) {
			Console::warn("Shader %s don't have mode %s",
				cmd.material->getShaderName().c_str(),
				getShaderFeatureName(deferredShaderFeature.enumValue).c_str());
			return false;
		}

		if (!deferredShader->init()) {
			return false;
		}

		if (!lightingShader->init()) {
			return false;
		}

		MaterialRenderData* deferredMaterialRenderData = dynamic_cast<MaterialRenderData*>(cmd.material->getRenderData());
		if (deferredMaterialRenderData == NULL) {
			return false;
		}
		if (deferredMaterialRenderData->usedFrame < (long long)Time::frames()) {
			deferredMaterialRenderData->program = deferredShader;
			deferredMaterialRenderData->create();
			deferredMaterialRenderData->usedFrame = Time::frames();
		}

		RenderTask task;
		task.age = 0;
		task.sceneData = cmd.sceneData;
		task.shaderProgram = deferredShader;
		task.renderMode = cmd.getRenderMode();
		task.cameraData = &gBufferRT->cameraData;
		task.materialData = deferredMaterialRenderData;
		task.meshData = meshData;
		task.extraData = cmd.bindings;

		geometryPass.commandList->addRenderTask(cmd, task);

		DeferredLightingTask lightingTask;
		lightingTask.sceneData = cmd.sceneData;
		lightingTask.program = lightingShader;
		lightingTask.gBufferRT = &gBufferRT->renderTarget;
		lightingTask.cameraRenderData = cameraRenderData;
		lightingTask.material = cmd.material;

		lightingPass.addTask(lightingTask);
	}
	return true;
}

void DeferredRenderGraph::setRenderCommandList(RenderCommandList& commandList)
{
	geometryPass.commandList = &commandList;
}

void DeferredRenderGraph::setMainRenderTarget(RenderTarget& renderTarget)
{
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
	for (auto& rt : gBufferRTs)
		rt.second->prepare();
	geometryPass.prepare();
	lightingPass.prepare();
	transparentPass.prepare();
	for (auto pass : passes)
		pass->prepare();
	imGuiPass.prepare();
}

void DeferredRenderGraph::execute(IRenderContext& context)
{
	for (auto sceneData : sceneDatas)
		sceneData->upload();
	context.bindTexture(NULL, Fragment_Shader_Stage, 6);
	context.bindTexture(NULL, Fragment_Shader_Stage, 7);
	context.bindTexture(NULL, Fragment_Shader_Stage, 8);
	context.bindTexture(NULL, Fragment_Shader_Stage, 9);
	context.bindTexture(NULL, Fragment_Shader_Stage, 10);
	geometryPass.execute(context);
	lightingPass.execute(context);
	transparentPass.execute(context);
	context.clearFrameBindings();
	for (auto pass : passes)
		pass->execute(context);
	imGuiPass.execute(context);

	/*----- Vendor swap -----*/
	{
		if (!VendorManager::getInstance().getVendor().swap(Engine::engineConfig, Engine::windowContext))
			throw runtime_error("Vendor swap failed");
	}
}

void DeferredRenderGraph::reset()
{
	for (auto b = gBufferRTs.begin(), e = gBufferRTs.end(); b != e;) {
		b->second->age++;
		if (b->second->age >= 2) {
			delete b->second;
			b = gBufferRTs.erase(b);
		}
		else b++;
	}

	for (auto sceneData : sceneDatas)
		sceneData->reset();
	sceneDatas.clear();

	geometryPass.reset();
	lightingPass.reset();
	transparentList.resetCommand();
	transparentPass.reset();
	for (auto pass : passes) {
		pass->reset();
		pass->renderGraph = NULL;
	}
	imGuiPass.reset();
	passes.clear();
}

void DeferredRenderGraph::getPasses(vector<pair<string, RenderPass*>>& passes)
{
	passes.push_back(make_pair("geometry", &geometryPass));
	passes.push_back(make_pair("lighting", &lightingPass));
	passes.push_back(make_pair("transparent", &transparentPass));
	int i = 0;
	for (auto& pass : this->passes) {
		passes.push_back(make_pair("pass_" + to_string(i), pass));
	}
	passes.push_back(make_pair("imGui", &imGuiPass));
}

Serializable* DeferredRenderGraph::instantiate(const SerializationInfo& from)
{
	return new DeferredRenderGraph();
}
