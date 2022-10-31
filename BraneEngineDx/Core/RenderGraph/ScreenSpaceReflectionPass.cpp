#include "ScreenSpaceReflectionPass.h"
#include "../Asset.h"

void ScreenSpaceReflectionPass::prepare()
{
	if (gBufferA == NULL)
		return;
	if (material == NULL) {
		material = getAssetByPath<Material>("Engine/Shaders/Pipeline/SSRPass.mat");
	}
	if (material) {
		program = material->getShader()->getProgram(Shader_Default);
		program->init();
		materialRenderData.material = material;
		materialRenderData.program = program;
		materialRenderData.create();
	}
}

void ScreenSpaceReflectionPass::execute(IRenderContext& context)
{
	if (program == NULL || cameraData == NULL || gBufferA == NULL || gBufferB == NULL ||
		gBufferC == NULL || hiZMap == NULL || hitDataMap == NULL || hitColorMap == NULL)
		return;

	const char* gBufferASlot = "inTexture0";
	const char* gBufferBSlot = "inTexture1";
	const char* gBufferCSlot = "inTexture2";
	const char* hiZMapSlot = "inTexture3";

	const char* inHitColorMapSlot = "inTexture0";

	const char* hitDataMapSlot = "rwTexture0";
	const char* hitColorMapSlot = "rwTexture1";

	const char* outGBufferASlot = "rwTexture1";

	int width = gBufferA->getWidth();
	int height = gBufferA->getHeight();
	int hiZWidth = hiZMap->getWidth();
	int hiZHeight = hiZMap->getHeight();
	Vector3u localSize = material->getLocalSize();
	int dimX = ceilf(width / float(localSize.x())) * localSize.x();
	int dimY = ceilf(height / float(localSize.y())) * localSize.y();

	context.bindShaderProgram(program);

	cameraData->bind(context);

	materialRenderData.vendorMaterial->desc.colorField["hiZUVScale"].val =
		Color(width * 0.5f / hiZWidth, height * 0.5f / hiZHeight, 2.0f / width, 2.0f / height);
	materialRenderData.upload();
	context.bindMaterialBuffer(materialRenderData.vendorMaterial);
	context.bindTexture((ITexture*)gBufferA->getVendorTexture(), gBufferASlot);
	context.bindTexture((ITexture*)gBufferB->getVendorTexture(), gBufferBSlot);
	context.bindTexture((ITexture*)gBufferC->getVendorTexture(), gBufferCSlot);
	context.bindTexture((ITexture*)hiZMap->getVendorTexture(), hiZMapSlot);

	Image image0, image1;
	image0.level = 0;
	image1.level = 0;
	image0.texture = hitDataMap;
	image1.texture = hitColorMap;
	context.bindImage(image0, hitDataMapSlot);
	context.bindImage(image1, hitColorMapSlot);

	context.setDrawInfo(0, 2, 0);
	context.dispatchCompute(dimX, dimY, 1);

	context.bindTexture(NULL, gBufferASlot);
	image1.texture = gBufferA;
	context.bindImage(image1, outGBufferASlot);
	context.bindTexture((ITexture*)hitColorMap->getVendorTexture(), inHitColorMapSlot);

	context.setDrawInfo(1, 2, 0);
	context.dispatchCompute(dimX, dimY, 1);

	image0.texture = NULL;
	image1.texture = NULL;
	context.bindImage(image0, hitDataMapSlot);
	context.bindImage(image1, outGBufferASlot);
}

void ScreenSpaceReflectionPass::reset()
{
	/*gBufferA = NULL;
	gBufferB = NULL;
	gBufferC = NULL;
	hiZMap = NULL;
	hitDataMap = NULL;
	hitColorMap = NULL;*/
}

void ScreenSpaceReflectionPass::getOutputTextures(vector<pair<string, Texture*>>& textures)
{
	if (hitDataMap)
		textures.push_back(make_pair("SSR_HitDataMap", hitDataMap));
	if (hitColorMap)
		textures.push_back(make_pair("SSR_HitColorMap", hitColorMap));
}
