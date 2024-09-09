#include "SMAAPass.h"
#include "SMAA_AreaTex.h"
#include "SMAA_SearchTex.h"
#include "../Asset.h"
#include "../Console.h"
#include "../RenderCore/RenderCore.h"

Texture2D SMAAPass::areaTexture = Texture2D((unsigned char*)areaTexBytes,
	AREATEX_WIDTH, AREATEX_HEIGHT, AREATEX_CHANNEL, false,
	{ TW_Clamp, TW_Clamp, TF_Linear_Mip_Linear, TF_Linear_Mip_Linear, TIT_RG8_UF }, true);

Texture2D SMAAPass::searchTexture = Texture2D((unsigned char*)searchTexBytes,
	SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, SEARCHTEX_CHANNEL, false,
	{ TW_Clamp, TW_Clamp, TF_Linear_Mip_Linear, TF_Linear_Mip_Linear, TIT_R8_UF }, true);

SMAAPass::SMAAPass(const string& name, Material* material) : PostProcessPass(name, material)
{
	areaTexture.setAutoGenMip(false);
	searchTexture.setAutoGenMip(false);
	edgesTexture.setAutoGenMip(false);
	blendTexture.setAutoGenMip(false);
	screenTexture.setAutoGenMip(false);
	edgesRenderTarget.addTexture("edgesTex", edgesTexture);
	blendRenderTarget.addTexture("blendTex", blendTexture);
	screenRenderTarget.addTexture("colorTex", screenTexture);
}

void SMAAPass::prepare()
{
	edgeDetectionProgram = edgeDetectionMaterial->getShader()->getProgram(Shader_Default);
	blendingWeightCalculationProgram = blendingWeightCalculationMaterial->getShader()->getProgram(Shader_Default);
	neighborhoodBlendingProgram = neighborhoodBlendingMaterial->getShader()->getProgram(Shader_Default);
	
	if (edgeDetectionProgram == NULL ||
		blendingWeightCalculationProgram == NULL ||
		neighborhoodBlendingProgram == NULL) {
		Console::error("PostProcessPass: Shader_Default not found in SMAA shaders");
		throw runtime_error("ShaderVariant not found");
		return;
	}

	if (!edgeDetectionProgram->isComputable() &&
		!blendingWeightCalculationProgram->isComputable() &&
		!neighborhoodBlendingProgram->isComputable()) {
		edgeDetectionProgram->init();
		blendingWeightCalculationProgram->init();
		neighborhoodBlendingProgram->init();
	}
	else {
		throw runtime_error("Shader type mismatch");
		return;
	}
	
	areaTexture.bind();
	searchTexture.bind();
	screenTexture.bind();
	edgesRenderTarget.init();
	blendRenderTarget.init();
	screenRenderTarget.init();
}

void SMAAPass::execute(IRenderContext& context)
{
	Texture2D* sceneMap = dynamic_cast<Texture2D*>(resource->screenTexture);
	if (sceneMap) {
		screenTexture.setTextureInfo(sceneMap->getTextureInfo());
		screenTexture.bind();
		context.copySubTexture2D(sceneMap->getVendorTexture(), 0, screenTexture.getVendorTexture(), 0);
	}

	static const ShaderPropertyName colorTexGammaName = "colorTexGamma";
	static const ShaderPropertyName areaTexName = "areaTex";
	static const ShaderPropertyName searchTexName = "searchTex";
	static const ShaderPropertyName edgesTexName = "edgesTex";
	static const ShaderPropertyName colorTexName = "colorTex";
	static const ShaderPropertyName blendTexName = "blendTex";

	// Edge detection
	context.bindShaderProgram(edgeDetectionProgram);
	cameraRenderData->bind(context);
	context.bindFrame(edgesRenderTarget.getVendorRenderTarget());
	context.clearFrameColor({ 0, 0, 0, 0 });
	context.bindTexture(screenTexture.getVendorTexture(), colorTexGammaName);
	context.setRenderPostReplaceState();
	context.setViewport(0, 0, size.x, size.y);
	context.postProcessCall();

	// Blending weight calculation
	context.bindShaderProgram(blendingWeightCalculationProgram);
	context.bindFrame(blendRenderTarget.getVendorRenderTarget());
	context.clearFrameColor({ 0, 0, 0, 0 });
	context.bindTexture(areaTexture.getVendorTexture(), areaTexName);
	context.bindTexture(searchTexture.getVendorTexture(), searchTexName);
	context.bindTexture(edgesTexture.getVendorTexture(), edgesTexName);
	context.postProcessCall();

	// Neighborhood blending
	context.bindShaderProgram(neighborhoodBlendingProgram);
	context.bindFrame(resource->screenRenderTarget->getVendorRenderTarget());
	context.bindTexture(screenTexture.getVendorTexture(), colorTexName);
	context.bindTexture(blendTexture.getVendorTexture(), blendTexName);
	context.postProcessCall();

	context.clearFrameBindings();
}

bool SMAAPass::loadDefaultResource()
{
	if (edgeDetectionMaterial == NULL)
		edgeDetectionMaterial = getAssetByPath<Material>("Engine/Shaders/PostProcess/SMAA/SMAA_EdgeDetection.mat");
	if (blendingWeightCalculationMaterial == NULL)
		blendingWeightCalculationMaterial = getAssetByPath<Material>("Engine/Shaders/PostProcess/SMAA/SMAA_BlendingWeightCalculation.mat");
	if (neighborhoodBlendingMaterial == NULL)
		neighborhoodBlendingMaterial = getAssetByPath<Material>("Engine/Shaders/PostProcess/SMAA/SMAA_NeighborhoodBlending.mat");

	cameraRenderData = resource->cameraRenderData;

	if (edgeDetectionMaterial == NULL || blendingWeightCalculationMaterial == NULL ||
		neighborhoodBlendingMaterial == NULL || resource == NULL ||
		resource->screenTexture == NULL || cameraRenderData == NULL)
		return false;
	return true;
}

void SMAAPass::render(RenderInfo& info)
{
	if (!enable)
		return;
	if (size.x == 0 || size.y == 0)
		return;
	if (!loadDefaultResource())
		return;

	RENDER_THREAD_ENQUEUE_TASK(AddSMAAPass, ([this] (RenderThreadContext& context)
	{
		context.renderGraph->addPass(*this);
	}));
}

void SMAAPass::onGUI(EditorInfo& info)
{
}

void SMAAPass::resize(const Unit2Di& size)
{
	PostProcessPass::resize(size);
	edgesRenderTarget.resize(size.x, size.y);
	blendRenderTarget.resize(size.x, size.y);
	screenRenderTarget.resize(size.x, size.y);
}
