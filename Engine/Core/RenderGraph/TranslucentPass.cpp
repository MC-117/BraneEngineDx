#include "TranslucentPass.h"
#include "../Utility/RenderUtility.h"
#include "../Console.h"
#include "../RenderCore/SurfaceBufferGetter.h"
#include "../RenderCore/VirtualShadowMap/VirtualShadowMap.h"

TranslucentSSRBinding::TranslucentSSRBinding() : SSRBinding()
{
}

void TranslucentSSRBinding::create()
{
}

void TranslucentSSRBinding::release()
{
	SSRBinding::release();
	gBufferA = NULL;
	gBufferB = NULL;
	hiZTexture = NULL;
}

void TranslucentSSRBinding::bind(IRenderContext& context)
{
	SSRBinding::bind(context);
	static const ShaderPropertyName gBufferAName = "gBufferA";
	static const ShaderPropertyName gBufferBName = "gBufferB";
	static const ShaderPropertyName hiZMapName = "hiZMap";
	static const ShaderPropertyName SSRInfoName = "SSRInfo";

	context.bindTexture(gBufferA ? gBufferA->getVendorTexture() : NULL, gBufferAName);
	context.bindTexture(gBufferB ? gBufferB->getVendorTexture() : NULL, gBufferBName);
	context.bindTexture(hiZTexture ? hiZTexture->getVendorTexture() : NULL, hiZMapName);
}

bool TranslucentPass::setRenderCommand(const IRenderCommand& command)
{
	const bool enableVSMDepthPass = VirtualShadowMapConfig::isEnable();

	MaterialRenderData* materialRenderData = command.materialRenderData;
	if (materialRenderData == NULL)
		return false;

	IMaterial* materialVariant = NULL;
	Enum<ShaderFeature> shaderFeature = command.getShaderFeature();
	if (enableVSMDepthPass) {
		ShaderMatchRule matchRule;
		matchRule.mainFeatureMask = Shader_Skeleton | Shader_Morph | Shader_Particle | Shader_Modifier | Shader_Terrain;
		matchRule.fragmentFlag = ShaderMatchFlag::Best;
		Enum<ShaderFeature> vsmShaderFeature = shaderFeature;
		vsmShaderFeature |= Shader_VSM;
		materialVariant = materialRenderData->getVariant(vsmShaderFeature, matchRule);
		if (materialVariant && !materialVariant->program->shaderType.has(Shader_VSM))
			materialVariant = NULL;
	}
	if (materialVariant == NULL) {
		materialVariant = materialRenderData->getVariant(shaderFeature);
		if (materialVariant == NULL) {
			Console::warn("Shader %s don't have mode %s",
				materialRenderData->getShaderName().c_str(),
				getShaderFeatureNames(shaderFeature).c_str());
			return false;
		}
	}

	if (!materialVariant->init())
		return false;

	static const ShaderPropertyName& ssrName = "SSR";
	const bool needSSR = materialVariant->program->getShaderMacroSet().has(ssrName);

	MeshData* meshData = command.mesh == NULL ? NULL : command.mesh->meshData;
	if (meshData)
		meshData->init();

	RenderTask task;
	task.age = 0;
	task.sceneData = command.sceneData;
	task.batchDrawData = command.batchDrawData;
	task.shaderProgram = materialVariant->program;
	task.materialVariant = materialVariant;
	task.meshData = meshData;
	task.extraData = command.bindings;

	if (enableVSMDepthPass) {
		task.extraData.push_back(&command.sceneData->virtualShadowMapRenderData);
	}

	for (auto& cameraRenderData : command.sceneData->cameraRenderDatas) {
		task.renderMode = command.getRenderMode("Translucent"_N, cameraRenderData);
		task.cameraData = cameraRenderData;
		task.surface = cameraRenderData->surface;
		if (needSSR) {
			TranslucentSSRBinding* ssrBinding = getSSRBinding(*command.sceneData, *cameraRenderData);
			if (ssrBinding) {
				task.extraData.push_back(ssrBinding);
			}
			commandList.addRenderTask(command, task);
		}
	}

	return true;
}

bool TranslucentPass::loadDefaultResource()
{
	return true;
}

void TranslucentPass::prepare()
{
	outputTextures.clear();
}

void TranslucentPass::execute(IRenderContext& context)
{
	RenderCommandExecutionInfo executionInfo(context);
	executionInfo.plusClearFlags = plusClearFlags;
	executionInfo.minusClearFlags = minusClearFlags;
	executionInfo.outputTextures = &outputTextures;
	executionInfo.timer = &timer;
	commandList.excuteCommand(executionInfo);
}

void TranslucentPass::reset()
{
	cachedSSRBindings.resize(ssrBindings.size());
	ssrBindings.clear();
	commandList.resetCommand();
}

void TranslucentPass::getOutputTextures(vector<pair<string, Texture*>>& textures)
{
	textures = outputTextures;
}

TranslucentSSRBinding* TranslucentPass::getSSRBinding(const SceneRenderData& sceneData, const CameraRenderData& cameraData)
{
	IGBufferGetter* gBufferGetter = dynamic_cast<IGBufferGetter*>(cameraData.surfaceBuffer);
	IHiZBufferGetter* hiZBufferGetter = dynamic_cast<IHiZBufferGetter*>(cameraData.surfaceBuffer);
	if (gBufferGetter == NULL || hiZBufferGetter == NULL)
		return NULL;
	Texture* gBufferA = gBufferGetter->getGBufferA();
	Texture* gBufferB = gBufferGetter->getGBufferB();
	Texture* hiZTexture = hiZBufferGetter->getHiZTexture();
	if (gBufferA == NULL || gBufferB == NULL || hiZTexture == NULL)
		return NULL;

	size_t hash = (size_t)&sceneData;
	hash_combine(hash, (size_t)&cameraData);
	auto iter = ssrBindings.find(hash);
	if (iter != ssrBindings.end())
		return iter->second.get();
	shared_ptr<TranslucentSSRBinding> ssrBinding;
	if (ssrBindings.size() < cachedSSRBindings.size())
		ssrBinding = cachedSSRBindings[ssrBindings.size()];
	else
		ssrBinding = cachedSSRBindings.emplace_back(make_shared<TranslucentSSRBinding>());
	ssrBindings.insert(make_pair(hash, ssrBinding));

	int width = gBufferA->getWidth();
	int height = gBufferA->getHeight();
	int hiZWidth = hiZTexture->getWidth();
	int hiZHeight = hiZTexture->getHeight();
	Vector4f hiZUVScale = { width * 0.5f / hiZWidth, height * 0.5f / hiZHeight, 2.0f / width, 2.0f / height };

	ssrBinding->gBufferA = gBufferA;
	ssrBinding->gBufferB = gBufferB;
	ssrBinding->hiZTexture = hiZTexture;
	ssrBinding->ssrInfo.hiZStartLevel = 5;
	ssrBinding->ssrInfo.hiZStopLevel = -1;
	ssrBinding->ssrInfo.hiZMaxStep = 48;
	ssrBinding->ssrInfo.hiZUVScale = hiZUVScale;

	ssrBinding->create();
	renderGraph->getRenderDataCollectorRenderThread()->add(*ssrBinding);
	return ssrBinding.get();
}
