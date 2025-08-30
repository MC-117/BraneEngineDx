#include "VolumetricLightPass.h"
#include "../Asset.h"
#include "../Console.h"
#include "../Camera.h"
#include "../GUI/UIControl.h"
#include "../RenderCore/RenderCore.h"
#include "../RenderCore/RenderCoreUtility.h"
#include "../InitializationManager.h"
#include "../Engine.h"
#include "Core/GUI/GUIUtility.h"
#include "Core/RenderCore/SurfaceBufferGetter.h"

VolumetricFogConfig& VolumetricFogConfig::instance()
{
	static VolumetricFogConfig config;
	return config;
}

class VolumetricFogConfigInitializer : public Initialization
{
protected:
	static VolumetricFogConfigInitializer instance;
	VolumetricFogConfigInitializer() : Initialization(
		InitializeStage::BeforeEngineSetup, 0,
		FinalizeStage::BeforeEngineRelease, 0) { }

	virtual bool initialize()
	{
		Engine::engineConfig.configInfo.get("volumetricFog.gridPixels", (int&)VolumetricFogConfig::instance().gridPixels);
		Engine::engineConfig.configInfo.get("volumetricFog.gridZSlices", (int&)VolumetricFogConfig::instance().gridZSlices);
		Engine::engineConfig.configInfo.get("volumetricFog.nearOffsetScale", VolumetricFogConfig::instance().nearOffsetScale);
		Engine::engineConfig.configInfo.get("volumetricFog.depthDistributionScale", VolumetricFogConfig::instance().depthDistributionScale);
		return true;
	}

	virtual bool finalize()
	{
		return true;
	}
};

VolumetricFogConfigInitializer VolumetricFogConfigInitializer::instance;

VolumetricLightPass::VolumetricLightPass(const Name& name, Material* material)
	: PostProcessPass(name, material)
	, maxDepthTexture(160, 90, 4, false, { TW_Clamp_Edge, TW_Clamp_Edge, TF_Linear, TF_Linear, TIT_R32_F })
	, fogScatteringVolume(160, 90, 64, 4, false, { TW_Clamp_Edge, TW_Clamp_Edge, TF_Linear, TF_Linear, TIT_RGBA16_FF })
	, fogIntegratedVolume(160, 90, 64, 4, false, { TW_Clamp_Edge, TW_Clamp_Edge, TF_Linear, TF_Linear, TIT_RGBA16_FF })
	, parameterBuffer(GB_Constant, GBF_Struct, sizeof(FogParameters))
{
	maxDepthTexture.setAutoGenMip(false);
	fogScatteringVolume.setAutoGenMip(false);
	fogIntegratedVolume.setAutoGenMip(false);
}

Vector3f getVolumetricFogGridZParams(float nearPlane, float farPlane, unsigned int startDistance, float depthDistributionScale, int gridSizeZ)
{
	nearPlane = max(nearPlane, float(startDistance));

	float NearOffset = .095 * 100.0;
	// Space out the slices so they aren't all clustered at the near plane
	float S = depthDistributionScale;

	float N = nearPlane + NearOffset;
	float F = farPlane;

	float O = (F - N * exp2((gridSizeZ - 1) / S)) / (F - N);
	float B = (1 - O) / N;

	return Vector3f(B, O, S);
}

void VolumetricLightPass::prepare()
{
	genMaxDepthProgram = genMaxDepthMaterial->getShader()->getProgram(Shader_Default);
	if (genMaxDepthProgram == NULL) {
		Console::error("PostProcessPass: Shader_Postprocess not found in shader '%s'", genMaxDepthMaterial->getShaderName().c_str());
		throw runtime_error("ShaderVariant not found");
		return;
	}
	Enum<ShaderFeature> shaderFeature = Shader_Default;
	if (VirtualShadowMapConfig::isEnable())
		shaderFeature |= Shader_VSM;
	lightScatteringProgram = lightScatteringMaterial->getShader()->getProgram(shaderFeature);
	if (lightScatteringProgram == NULL) {
		Console::error("PostProcessPass: Shader_Postprocess not found in shader '%s'", lightScatteringMaterial->getShaderName().c_str());
		throw runtime_error("ShaderVariant not found");
		return;
	}
	integrationProgram = integrationMaterial->getShader()->getProgram(Shader_Default);
	if (integrationProgram == NULL) {
		Console::error("PostProcessPass: Shader_Postprocess not found in shader '%s'", integrationMaterial->getShaderName().c_str());
		throw runtime_error("ShaderVariant not found");
		return;
	}
	applyProgram = applyMaterial->getShader()->getProgram(Shader_Postprocess);
	if (applyProgram == NULL) {
		Console::error("PostProcessPass: Shader_Postprocess not found in shader '%s'", applyMaterial->getShaderName().c_str());
		throw runtime_error("ShaderVariant not found");
		return;
	}

	genMaxDepthPipelineState = fetchPSOIfDescChangedThenInit(genMaxDepthPipelineState, genMaxDepthProgram);
	lightScatteringPipelineState = fetchPSOIfDescChangedThenInit(lightScatteringPipelineState, lightScatteringProgram);
	integrationPipelineState = fetchPSOIfDescChangedThenInit(integrationPipelineState, integrationProgram);

	GraphicsPipelineStateDesc desc = GraphicsPipelineStateDesc::forScreen(
		applyProgram, resource->screenRenderTarget, BM_MultiplyAlpha);
	desc.renderMode.setDepthStencilMode(DepthStencilMode::DepthNonTestNonWritable());
	applyPipelineState = fetchPSOIfDescChangedThenInit(applyPipelineState, desc);


	const CameraData& cameraData = dynamic_cast<CameraRenderData*>(cameraRenderData)->data;
	VolumetricFogConfig& config = VolumetricFogConfig::instance();
	parameters.fogGridZParams = getVolumetricFogGridZParams(cameraData.zNear, parameters.vfogMaxDistance,
		parameters.vfogStartDistance, config.depthDistributionScale, config.gridZSlices);
	static constexpr float maxWorldViewZ = 65536.0f;
	parameters.fogMaxWorldViewHeight = std::min(FLT_MAX, parameters.efogHeight + maxWorldViewZ);
	parameters.hiZSize = Vector2f { (float)hiZTexture->getWidth(), (float)hiZTexture->getHeight() };

	parameterBuffer.resize(1);

	resize(size);
}

void VolumetricLightPass::execute(IRenderContext& context)
{
	static const ShaderPropertyName FogParametersName = "FogParameters";
	static const ShaderPropertyName hiZTextureName = "hiZTexture";
	static const ShaderPropertyName outMaxDepthTextureName = "outMaxDepthTexture";
	static const ShaderPropertyName maxDepthTextureName = "maxDepthTexture";
	static const ShaderPropertyName depthMapName = "depthMap";
	static const ShaderPropertyName sceneDepthMapName = "sceneDepthMap";
	static const ShaderPropertyName fogScatteringVolumeName = "fogScatteringVolume";
	static const ShaderPropertyName fogIntegratedVolumeName = "fogIntegratedVolume";
	static const ShaderPropertyName outFogScatteringVolumeName = "outFogScatteringVolume";
	static const ShaderPropertyName outFogIntegratedVolumeName = "outFogIntegratedVolume";
	static const ShaderPropertyName outFogTextureName = "outFogTexture";
	static const ShaderPropertyName fogTextureName = "fogTexture";

	Vector2i maxDepthDim = {
		(int)ceil(safeWidth / 8.0f),
		(int)ceil(safeHeight / 8.0f)
	};

	Vector3i dim = {
		(int)ceil(safeWidth / 4.0f),
		(int)ceil(safeHeight / 4.0f),
		(int)ceil(safeDepth / 4.0f)
	};

	Vector2i screenDim = {
		(int)ceil(size.x / 8.0f),
		(int)ceil(size.y / 8.0f)
	};

	context.uploadBufferData(parameterBuffer.getVendorGPUBuffer(), 1, &parameters);

	// Pre Pass GenMinDepth
	context.bindPipelineState(genMaxDepthPipelineState);

	context.bindBufferBase(parameterBuffer.getVendorGPUBuffer(), FogParametersName);

	Image maxDepthImage;
	maxDepthImage.texture = &maxDepthTexture;
	context.bindTexture(hiZTexture->getVendorTexture(), hiZTextureName);
	context.bindImage(maxDepthImage, outMaxDepthTextureName);
	cameraRenderData->bind(context);

	context.dispatchCompute(maxDepthDim.x(), maxDepthDim.y(), 1);
	context.unbindBufferBase(outMaxDepthTextureName);

	// Pass 0 LightScattering
	context.bindPipelineState(lightScatteringPipelineState);
	
	context.bindBufferBase(parameterBuffer.getVendorGPUBuffer(), FogParametersName);
	cameraRenderData->bind(context);
	lightRenderData->bind(context);
	probeRenderData->bind(context);
	
	if (VirtualShadowMapConfig::isEnable())
		vsmRenderData->bind(context);
	else
		context.bindTexture((ITexture*)directShadowMap->getVendorTexture(), depthMapName);
	context.bindTexture(maxDepthTexture.getVendorTexture(), maxDepthTextureName);

	Image scatteringImage;
	scatteringImage.texture = &fogScatteringVolume;
	scatteringImage.dimension = TD_Volume;
	scatteringImage.level = 0;
	scatteringImage.arrayBase = 0;
	scatteringImage.arrayCount = fogScatteringVolume.getArrayCount();
	context.bindImage(scatteringImage, outFogScatteringVolumeName);

	context.dispatchCompute(dim.x(), dim.y(), dim.z());
	context.unbindBufferBase(outFogScatteringVolumeName);
	context.unbindBufferBase(maxDepthTextureName);

	// Pass 1 Integration
	context.bindPipelineState(integrationPipelineState);

	context.bindBufferBase(parameterBuffer.getVendorGPUBuffer(), FogParametersName);
	cameraRenderData->bind(context);
	lightRenderData->bind(context);

	context.bindTexture(fogScatteringVolume.getVendorTexture(), fogScatteringVolumeName);

	Image integrationImage;
	integrationImage.texture = &fogIntegratedVolume;
	integrationImage.dimension = TD_Volume;
	integrationImage.level = 0;
	integrationImage.arrayBase = 0;
	integrationImage.arrayCount = fogIntegratedVolume.getArrayCount();
	context.bindImage(integrationImage, outFogIntegratedVolumeName);
	
	context.dispatchCompute(dim.x(), dim.y(), dim.z());
	context.unbindBufferBase(fogScatteringVolumeName);
	context.unbindBufferBase(outFogIntegratedVolumeName);

	// Pass 3 Apply
	context.bindPipelineState(applyPipelineState);
	
	context.bindFrame(resource->screenRenderTarget->getVendorRenderTarget());
	
	context.bindBufferBase(parameterBuffer.getVendorGPUBuffer(), FogParametersName);
	context.bindTexture(fogIntegratedVolume.getVendorTexture(), fogIntegratedVolumeName);
	context.bindTexture(resource->depthTexture->getVendorTexture(), sceneDepthMapName);

	context.setViewport(0, 0, size.x, size.y);
	context.postProcessCall();

	context.clearFrameBindings();
}

bool VolumetricLightPass::loadDefaultResource()
{
	if (genMaxDepthMaterial == NULL) {
		genMaxDepthMaterial = getAssetByPath<Material>("Engine/Shaders/Pipeline/VolumetricFog/VolumetricFog_GenMaxDepth.mat");
	}
	if (lightScatteringMaterial == NULL) {
		lightScatteringMaterial = getAssetByPath<Material>("Engine/Shaders/Pipeline/VolumetricFog/VolumetricFog_Lighting.mat");
	}
	if (integrationMaterial == NULL) {
		integrationMaterial = getAssetByPath<Material>("Engine/Shaders/Pipeline/VolumetricFog/VolumetricFog_Integration.mat");
	}
	if (applyMaterial == NULL) {
		applyMaterial = getAssetByPath<Material>("Engine/Shaders/Pipeline/VolumetricFog/VolumetricFog_Apply.mat");
	}
	cameraRenderData = resource->cameraRenderData;
	if (IHiZBufferGetter* hiZGetter = dynamic_cast<IHiZBufferGetter*>(resource->cameraRenderData->surfaceBuffer)) {
		hiZTexture = hiZGetter->getHiZTexture();
	}
	else {
		hiZTexture = NULL;
	}
	if (genMaxDepthMaterial == NULL || lightScatteringMaterial == NULL || integrationMaterial == NULL || applyMaterial == NULL
		|| hiZTexture == NULL || resource == NULL || resource->screenRenderTarget == NULL || resource->depthTexture == NULL)
		return false;
	return true;
}

void VolumetricLightPass::render(RenderInfo& info)
{
	if (!enable)
		return;
	if (size.x == 0 || size.y == 0)
		return;
	if (!loadDefaultResource())
		return;

	RENDER_THREAD_ENQUEUE_TASK(AddVolumetricLightPass, ([this, materialRenderData = materialRenderData] (RenderThreadContext& context)
	{
		context.renderGraph->addPass(*this);
		if (materialRenderData)
			renderGraph->getRenderDataCollectorMainThread()->add(*materialRenderData);
		if (context.sceneRenderData->lightDataPack.shadowTarget)
			directShadowMap = context.sceneRenderData->lightDataPack.shadowTarget->getDepthTexture();
		if (VirtualShadowMapConfig::isEnable())
			vsmRenderData = &context.sceneRenderData->virtualShadowMapRenderData;
		else
			vsmRenderData = NULL;
		lightRenderData = &context.sceneRenderData->lightDataPack;
		probeRenderData = &context.sceneRenderData->probePoolPack;
	}));
}

void VolumetricLightPass::resize(const Unit2Di& size)
{
	PostProcessPass::resize(size);
	VolumetricFogConfig& config = VolumetricFogConfig::instance();
	safeWidth = max((int)ceil(size.x / (float)config.gridPixels), 1);
	safeHeight = max((int)ceil(size.y / (float)config.gridPixels), 1);
	safeDepth = max(config.gridZSlices, 1u);
	maxDepthTexture.resize(safeWidth, safeHeight);
	fogScatteringVolume.resize(safeWidth, safeHeight, safeDepth);
	fogIntegratedVolume.resize(safeWidth, safeHeight, safeDepth);
	parameters.fogScreenToUV = Vector2f(1.0f / (safeWidth * config.gridPixels), 1.0f / (safeHeight * config.gridPixels));
	parameters.fogGridSize = Vector3i(safeWidth, safeHeight, safeDepth);
}

void VolumetricLightPass::onGUI(EditorInfo& info)
{
	PostProcessPass::onGUI(info);
	ImGui::BeginGroupPanel("HeightFog");
	ImGui::PushID("HeightFog");
	ImGui::ColorEdit3("Color", parameters.efogColor.data());
	ImGui::DragFloat("Height", &parameters.efogHeight, 0.001, 0);
	float density = parameters.efogDensity * 1000;
	if (ImGui::DragFloat("Density", &density, 0.001, 0, 1)) {
		parameters.efogDensity = density * 0.001f;
	}
	float heightFalloff = parameters.efogHeightFalloff * 1000;
	if (ImGui::DragFloat("HeightFalloff", &heightFalloff, 0.001, 0.001, 2)) {
		parameters.efogHeightFalloff = heightFalloff * 0.001f;
	}
	ImGui::DragFloat("StartDistance", &parameters.efogStartDistance, 0.01, 0, 5000);
	bool useCutoff = parameters.efogCutoffDistance > 0;
	if (ImGui::Checkbox("UseCutoff", &useCutoff)) {
		parameters.efogCutoffDistance = useCutoff ? parameters.efogCutoffDistance : 0;
	}
	if (useCutoff && parameters.efogCutoffDistance == 0) {
		parameters.efogCutoffDistance = 100000;
	}
	ImGui::BeginDisabled(!useCutoff);
	ImGui::DragFloat("CutoffDistance", &parameters.efogCutoffDistance, 0.01, 100000, 20000000);
	ImGui::EndDisabled();
	ImGui::DragFloat("MaxOpacity", &parameters.efogMaxOpacity, 0.001, 0, 1);
	ImGui::PopID();
	ImGui::EndGroupPanel();

	ImGui::BeginGroupPanel("VolumetricFog");
	ImGui::PushID("VolumetricFog");
	ImGui::ColorEdit3("ScatterColor", parameters.vfogColor.data());
	ImGui::DragFloat("ScatteringDistribution", &parameters.vfogScatteringDistribution, 0.001, -0.9, 0.9);
	ImGui::DragFloat("GridSliceZDistributionScale", &VolumetricFogConfig::instance().depthDistributionScale, 0.001, 0.1, 10);
	ImGui::DragFloat("ExtinctionScale", &parameters.vfogExtinctionScale, 0.001, 0.1, 10);
	ImGui::DragFloat("StartDistance", &parameters.vfogStartDistance, 0.001, 0, 5000);
	float distance = parameters.vfogMaxDistance - parameters.vfogStartDistance;
	if (ImGui::DragFloat("Distance", &distance, 0.001, 1000, 10000)) {
		parameters.vfogMaxDistance = distance + parameters.vfogStartDistance;
	}
	ImGui::DragFloat("NoiseScalar", &parameters.vfogNoiseScalar, 0.001, 0, 100);
	ImGui::PopID();
	ImGui::EndGroupPanel();
}
