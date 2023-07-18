#include "ScreenSpaceReflectionPass.h"
#include "../Asset.h"

void ScreenSpaceReflectionPass::prepare()
{
	if (!enable)
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
	outputTextures.clear();
}

void ScreenSpaceReflectionPass::execute(IRenderContext& context)
{
	if (!enable)
		return;
	if (useComputeShader) {
		if (computeProgram == NULL)
			return;

		static const ShaderPropertyName inTexture0Name = "inTexture0";
		static const ShaderPropertyName inTexture1Name = "inTexture1";
		static const ShaderPropertyName inTexture2Name = "inTexture2";
		static const ShaderPropertyName inTexture3Name = "inTexture3";
		static const ShaderPropertyName inTexture4Name = "inTexture4";
		static const ShaderPropertyName inTexture5Name = "inTexture5";

		static const ShaderPropertyName rwTexture0Name = "rwTexture0";
		static const ShaderPropertyName rwTexture1Name = "rwTexture1";

		const ShaderPropertyName& gBufferASlot = inTexture0Name;
		const ShaderPropertyName& gBufferBSlot = inTexture1Name;
		const ShaderPropertyName& gBufferCSlot = inTexture2Name;
		const ShaderPropertyName& gBufferDSlot = inTexture3Name;
		const ShaderPropertyName& gBufferESlot = inTexture4Name;
		const ShaderPropertyName& hiZMapSlot = inTexture5Name;

		const ShaderPropertyName& inHitColorMapSlot = inTexture0Name;

		const ShaderPropertyName& hitDataMapSlot = rwTexture0Name;
		const ShaderPropertyName& hitColorMapSlot = rwTexture1Name;

		const ShaderPropertyName& outGBufferASlot = rwTexture1Name;

		for (auto sceneData : renderGraph->sceneDatas) {
			for (auto cameraData : sceneData->cameraRenderDatas) {
				IGBufferGetter* gbufferGetter = dynamic_cast<IGBufferGetter*>(cameraData->surfaceBuffer);
				IHiZBufferGetter* hiZGetter = dynamic_cast<IHiZBufferGetter*>(cameraData->surfaceBuffer);
				IScreenSpaceReflectionBufferGetter* ssrGetter = dynamic_cast<IScreenSpaceReflectionBufferGetter*>(cameraData->surfaceBuffer);
				if (gbufferGetter == NULL || ssrGetter == NULL)
					continue;

				Texture* gBufferA = gbufferGetter->getGBufferA();
				Texture* gBufferB = gbufferGetter->getGBufferB();
				Texture* gBufferC = gbufferGetter->getGBufferC();
				Texture* gBufferD = gbufferGetter->getGBufferD();
				Texture* gBufferE = gbufferGetter->getGBufferE();

				Texture* hiZMap = hiZGetter->getHiZTexture();

				Texture* hitDataMap = ssrGetter->getHitDataTexture();
				Texture* hitColorMap = ssrGetter->getHitColorTexture();
				RenderTarget* traceRenderTarget = ssrGetter->getTraceRenderTarget();
				RenderTarget* resolveRenderTarget = ssrGetter->getResolveRenderTarget();

				if (gBufferA == NULL || gBufferB == NULL || gBufferC == NULL ||
					hiZMap == NULL || hitDataMap == NULL || hitColorMap == NULL ||
					traceRenderTarget == NULL || resolveRenderTarget == NULL)
					continue;

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
				context.bindTexture((ITexture*)gBufferD->getVendorTexture(), gBufferDSlot);
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

				outputTextures.push_back(make_pair("SSR_HitDataMap", hitDataMap));
				outputTextures.push_back(make_pair("SSR_HitColorMap", hitColorMap));
			}
		}

		context.clearFrameBindings();
	}
	else {
		if (traceProgram == NULL || resolveProgram == NULL)
			return;

		static const ShaderPropertyName gBufferAName = "gBufferA";
		static const ShaderPropertyName gBufferBName = "gBufferB";
		static const ShaderPropertyName gBufferCName = "gBufferC";
		static const ShaderPropertyName gBufferDName = "gBufferD";
		static const ShaderPropertyName gBufferEName = "gBufferE";
		static const ShaderPropertyName hiZMapName = "hiZMap";
		static const ShaderPropertyName hitDataMapName = "hitDataMap";
		static const ShaderPropertyName hitColorMapName = "hitColorMap";
		static const ShaderPropertyName sceneColorMipsName = "sceneColorMips";
		static const ShaderPropertyName brdfLUTName = "brdfLUT";

		for (auto sceneData : renderGraph->sceneDatas) {
			for (auto cameraData : sceneData->cameraRenderDatas) {
				IGBufferGetter* gbufferGetter = dynamic_cast<IGBufferGetter*>(cameraData->surfaceBuffer);
				IHiZBufferGetter* hiZGetter = dynamic_cast<IHiZBufferGetter*>(cameraData->surfaceBuffer);
				ISceneColorMipsGetter* sceneColorMipsGetter = dynamic_cast<ISceneColorMipsGetter*>(cameraData->surfaceBuffer);
				IScreenSpaceReflectionBufferGetter* ssrGetter = dynamic_cast<IScreenSpaceReflectionBufferGetter*>(cameraData->surfaceBuffer);
				if (gbufferGetter == NULL || hiZGetter == NULL ||
					sceneColorMipsGetter == NULL || ssrGetter == NULL)
					continue;

				Texture* gBufferA = gbufferGetter->getGBufferA();
				Texture* gBufferB = gbufferGetter->getGBufferB();
				Texture* gBufferC = gbufferGetter->getGBufferC();
				Texture* gBufferD = gbufferGetter->getGBufferD();
				Texture* gBufferE = gbufferGetter->getGBufferE();

				Texture* hiZMap = hiZGetter->getHiZTexture();
				Texture* sceneColorMips = sceneColorMipsGetter->getSceneColorMips();

				Texture* hitDataMap = ssrGetter->getHitDataTexture();
				Texture* hitColorMap = ssrGetter->getHitColorTexture();
				RenderTarget* traceRenderTarget = ssrGetter->getTraceRenderTarget();
				RenderTarget* resolveRenderTarget = ssrGetter->getResolveRenderTarget();

				if (gBufferA == NULL || gBufferB == NULL || gBufferC == NULL ||
					hiZMap == NULL || hitDataMap == NULL || hitColorMap == NULL ||
					sceneColorMips == NULL || traceRenderTarget == NULL || resolveRenderTarget == NULL)
					continue;

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
				context.bindTexture((ITexture*)gBufferB->getVendorTexture(), gBufferBName);
				context.bindTexture((ITexture*)gBufferC->getVendorTexture(), gBufferCName);
				context.bindTexture((ITexture*)gBufferD->getVendorTexture(), gBufferDName);
				context.bindTexture((ITexture*)gBufferE->getVendorTexture(), gBufferEName);
				context.bindTexture((ITexture*)hiZMap->getVendorTexture(), hiZMapName);

				context.bindFrame(traceRenderTarget->getVendorRenderTarget());

				context.setRenderPostReplaceState();
				context.postProcessCall();

				context.clearFrameBindings();

				// Resolve
				context.bindShaderProgram(resolveProgram);

				sceneData->bind(context);

				cameraData->bind(context);

				float resolveSamples = 9;
				float resolveRadius = 0.005;

				resolveMaterialRenderData.vendorMaterial->desc.scalarField["resolveSamples"].val = resolveSamples;
				resolveMaterialRenderData.vendorMaterial->desc.scalarField["resolveRadius"].val = resolveRadius;
				resolveMaterialRenderData.vendorMaterial->desc.colorField["hiZUVScale"].val = hiZUVScale;
				resolveMaterialRenderData.upload();
				context.bindMaterialBuffer(resolveMaterialRenderData.vendorMaterial);
				context.bindTexture((ITexture*)gBufferA->getVendorTexture(), gBufferAName);
				context.bindTexture((ITexture*)gBufferB->getVendorTexture(), gBufferBName);
				context.bindTexture((ITexture*)gBufferC->getVendorTexture(), gBufferCName);
				context.bindTexture((ITexture*)gBufferD->getVendorTexture(), gBufferDName);
				context.bindTexture((ITexture*)hitDataMap->getVendorTexture(), hitDataMapName);
				context.bindTexture((ITexture*)hitColorMap->getVendorTexture(), hitColorMapName);
				context.bindTexture((ITexture*)sceneColorMips->getVendorTexture(), sceneColorMipsName);
				context.bindTexture((ITexture*)Texture2D::brdfLUTTex.getVendorTexture(), brdfLUTName);

				context.bindFrame(resolveRenderTarget->getVendorRenderTarget());

				context.setRenderPostAddState();
				context.postProcessCall();

				outputTextures.push_back(make_pair("SSR_HitDataMap", hitDataMap));
				outputTextures.push_back(make_pair("SSR_HitColorMap", hitColorMap));
			}
		}

		context.clearFrameBindings();
	}
}

void ScreenSpaceReflectionPass::reset()
{
}

void ScreenSpaceReflectionPass::getOutputTextures(vector<pair<string, Texture*>>& textures)
{
	if (!enable)
		return;
	textures = outputTextures;
}
