#include "ScreenSpaceReflectionPass.h"
#include "../Asset.h"

void ScreenSpaceReflectionPass::prepare()
{
	if (!enable || gBufferA == NULL)
		return;
	if (useComputeShader) {
		if (computeMaterial == NULL) {
			computeMaterial = getAssetByPath<Material>("Engine/Shaders/Pipeline/SSRPass.mat");
		}
		if (computeMaterial) {
			computeProgram = computeMaterial->getShader()->getProgram(Shader_Default);
			computeProgram->init();
			computeMaterialRenderData.material = computeMaterial;
			computeMaterialRenderData.program = computeProgram;
			computeMaterialRenderData.create();
		}
	}
	else {
		if (traceMaterial == NULL) {
			traceMaterial = getAssetByPath<Material>("Engine/Shaders/Pipeline/SSRTracePassFS.mat");
		}
		if (traceMaterial) {
			traceProgram = traceMaterial->getShader()->getProgram(Shader_Postprocess);
			traceProgram->init();
			traceMaterialRenderData.material = traceMaterial;
			traceMaterialRenderData.program = traceProgram;
			traceMaterialRenderData.create();
		}
		if (resolveMaterial == NULL) {
			resolveMaterial = getAssetByPath<Material>("Engine/Shaders/Pipeline/SSRResolvePassFS.mat");
		}
		if (resolveMaterial) {
			resolveProgram = resolveMaterial->getShader()->getProgram(Shader_Postprocess);
			resolveProgram->init();
			resolveMaterialRenderData.material = resolveMaterial;
			resolveMaterialRenderData.program = resolveProgram;
			resolveMaterialRenderData.create();
		}
	}
}

void ScreenSpaceReflectionPass::execute(IRenderContext& context)
{
	if (useComputeShader) {
		if (!enable || computeProgram == NULL || cameraData == NULL || gBufferA == NULL || gBufferB == NULL ||
			gBufferC == NULL || hiZMap == NULL || hitDataMap == NULL || hitColorMap == NULL)
			return;

		static const ShaderPropertyName inTexture0Name = "inTexture0";
		static const ShaderPropertyName inTexture1Name = "inTexture1";
		static const ShaderPropertyName inTexture2Name = "inTexture2";
		static const ShaderPropertyName inTexture3Name = "inTexture3";
		static const ShaderPropertyName inTexture4Name = "inTexture4";

		static const ShaderPropertyName rwTexture0Name = "rwTexture0";
		static const ShaderPropertyName rwTexture1Name = "rwTexture1";

		const ShaderPropertyName& gBufferASlot = inTexture0Name;
		const ShaderPropertyName& gBufferBSlot = inTexture1Name;
		const ShaderPropertyName& gBufferCSlot = inTexture2Name;
		const ShaderPropertyName& gBufferESlot = inTexture3Name;
		const ShaderPropertyName& hiZMapSlot = inTexture4Name;

		const ShaderPropertyName& inHitColorMapSlot = inTexture0Name;

		const ShaderPropertyName& hitDataMapSlot = rwTexture0Name;
		const ShaderPropertyName& hitColorMapSlot = rwTexture1Name;

		const ShaderPropertyName& outGBufferASlot = rwTexture1Name;

		int width = gBufferA->getWidth();
		int height = gBufferA->getHeight();
		int hiZWidth = hiZMap->getWidth();
		int hiZHeight = hiZMap->getHeight();
		Color hiZUVScale = { width * 0.5f / hiZWidth, height * 0.5f / hiZHeight, 2.0f / width, 2.0f / height };
		Vector3u localSize = computeMaterial->getLocalSize();
		int dimX = ceilf(width / float(localSize.x())) * localSize.x();
		int dimY = ceilf(height / float(localSize.y())) * localSize.y();

		context.bindShaderProgram(computeProgram);

		sceneData->bind(context);

		cameraData->bind(context);

		computeMaterialRenderData.vendorMaterial->desc.colorField["hiZUVScale"].val = hiZUVScale;
		computeMaterialRenderData.upload();
		context.bindMaterialBuffer(computeMaterialRenderData.vendorMaterial);
		context.bindTexture((ITexture*)gBufferA->getVendorTexture(), gBufferASlot);
		context.bindTexture((ITexture*)gBufferB->getVendorTexture(), gBufferBSlot);
		context.bindTexture((ITexture*)gBufferC->getVendorTexture(), gBufferCSlot);
		context.bindTexture((ITexture*)gBufferE->getVendorTexture(), gBufferESlot);
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
	else {
		if (!enable || traceProgram == NULL || resolveProgram == NULL || cameraData == NULL || gBufferA == NULL ||
			gBufferB == NULL || gBufferC == NULL || hiZMap == NULL || hitDataMap == NULL || hitColorMap == NULL ||
			traceRenderTarget == NULL || resolveRenderTarget == NULL)
			return;

		static const ShaderPropertyName gBufferAName = "gBufferA";
		static const ShaderPropertyName gBufferBName = "gBufferB";
		static const ShaderPropertyName gBufferCName = "gBufferC";
		static const ShaderPropertyName gBufferEName = "gBufferE";
		static const ShaderPropertyName hiZMapName = "hiZMap";
		static const ShaderPropertyName hitDataMapName = "hitDataMap";
		static const ShaderPropertyName hitColorMapName = "hitColorMap";

		int width = gBufferA->getWidth();
		int height = gBufferA->getHeight();
		int hiZWidth = hiZMap->getWidth();
		int hiZHeight = hiZMap->getHeight();
		Color hiZUVScale = { width * 0.5f / hiZWidth, height * 0.5f / hiZHeight, 2.0f / width, 2.0f / height };
		
		// Trace
		context.bindShaderProgram(traceProgram);

		sceneData->bind(context);

		cameraData->bind(context);

		traceMaterialRenderData.vendorMaterial->desc.colorField["hiZUVScale"].val = hiZUVScale;
		traceMaterialRenderData.upload();
		context.bindMaterialBuffer(traceMaterialRenderData.vendorMaterial);
		context.bindTexture((ITexture*)gBufferA->getVendorTexture(), gBufferAName);
		context.bindTexture((ITexture*)gBufferB->getVendorTexture(), gBufferBName);
		context.bindTexture((ITexture*)gBufferC->getVendorTexture(), gBufferCName);
		context.bindTexture((ITexture*)gBufferE->getVendorTexture(), gBufferEName);
		context.bindTexture((ITexture*)hiZMap->getVendorTexture(), hiZMapName);

		context.bindFrame(traceRenderTarget->getVendorRenderTarget());
		context.clearFrameColors({ Color(), Color() });

		context.setRenderPostState();
		context.postProcessCall();

		context.clearFrameBindings();

		// Resolve
		context.bindShaderProgram(resolveProgram);

		sceneData->bind(context);

		cameraData->bind(context);

		resolveMaterialRenderData.vendorMaterial->desc.colorField["hiZUVScale"].val = hiZUVScale;
		resolveMaterialRenderData.upload();
		context.bindMaterialBuffer(resolveMaterialRenderData.vendorMaterial);
		context.bindTexture((ITexture*)gBufferB->getVendorTexture(), gBufferBName);
		context.bindTexture((ITexture*)gBufferC->getVendorTexture(), gBufferCName);
		context.bindTexture((ITexture*)hitDataMap->getVendorTexture(), hitDataMapName);
		context.bindTexture((ITexture*)hitColorMap->getVendorTexture(), hitColorMapName);

		context.bindFrame(resolveRenderTarget->getVendorRenderTarget());

		context.setRenderPostAddState();
		context.postProcessCall();

		context.clearFrameBindings();
	}
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
	if (!enable)
		return;
	if (hitDataMap)
		textures.push_back(make_pair("SSR_HitDataMap", hitDataMap));
	if (hitColorMap)
		textures.push_back(make_pair("SSR_HitColorMap", hitColorMap));
}
