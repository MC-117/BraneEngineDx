#include "BloomPass.h"
#include "../Camera.h"
#include "../Asset.h"
#include "../Console.h"
#include "../GUI/UIControl.h"
#include "../RenderCore/RenderCore.h"
#include "../RenderCore/RenderCoreUtility.h"
#include "../RenderCore/SurfaceBufferGetter.h"
#include "../Profile/RenderProfile.h"

constexpr int maxBloomLevel = 5;

BloomPass::BloomPass(const Name & name, Material * material)
	: PostProcessPass(name, material)
{
	int newBloomLevel = 1 + floor(log2(max(size.x, size.y)));
	newBloomLevel = min(newBloomLevel, maxBloomLevel);
	resizeBloomLevel(newBloomLevel);
}

void BloomPass::prepare()
{
	materialVaraint = materialRenderData->getVariant(Shader_Postprocess);
	if (materialVaraint == NULL) {
		Console::error("PostProcessPass: Shader_Postprocess not found in shader '%s'", materialRenderData->getShaderName().c_str());
		throw runtime_error("ShaderVariant not found");
		return;
	}

	materialVaraint->init();
	
	for (int i = 0; i < bloomLevel; i++) {
		bloomSampleRenderTargets[i]->init();
	}

	if (materialVaraint->program->isComputable()) {
		pipelineState = fetchPSOIfDescChangedThenInit(pipelineState, materialVaraint->program);
	}
	else {
		GraphicsPipelineStateDesc desc0 = GraphicsPipelineStateDesc::forScreen(
			materialVaraint->program, bloomSampleRenderTargets[0], BM_Default);
		pipelineState = fetchPSOIfDescChangedThenInit(pipelineState, desc0);
		
		GraphicsPipelineStateDesc desc1 = GraphicsPipelineStateDesc::forScreen(
			materialVaraint->program, resource->screenRenderTarget, BM_Additive);
		additivePipelineState = fetchPSOIfDescChangedThenInit(additivePipelineState, desc1);
	}
}

void BloomPass::execute(IRenderContext& context)
{
	static const ShaderPropertyName sampleMapName = "sampleMap";
	static const ShaderPropertyName imageMapName = "imageMap";

	materialRenderData->upload();

	ShaderProgram* program = materialVaraint->program;

	context.bindPipelineState(pipelineState);
	context.bindMaterialBuffer(materialVaraint);

	if (program->isComputable()) {
		Vector3u localSize = material->getLocalSize();

		context.setDrawInfo(0, 4, 0);
		context.bindTexture((ITexture*)resource->screenTexture->getVendorTexture(), sampleMapName);
		Image image;
		image.texture = &bloomSampleMap;
		for (int i = 0; i < bloomLevel; i++) {
			image.level = i;
			context.bindImage(image, imageMapName);
			context.dispatchCompute(ceilf(size.x / pow(2, i) / (float)localSize.x()), ceilf(size.y / pow(2, i) / (float)localSize.y()), 1);
		}

		context.setDrawInfo(1, 4, 0);
		for (int i = 0; i < bloomLevel; i++) {
			image.level = i;
			context.bindImage(image, imageMapName);
			context.dispatchCompute(ceilf(size.x / pow(2, i) / (float)localSize.x()), ceilf(size.y / pow(2, i) / (float)localSize.y()), 1);
		}

		context.setDrawInfo(2, 4, 0);
		for (int i = 0; i < bloomLevel; i++) {
			image.level = i;
			context.bindImage(image, imageMapName);
			context.dispatchCompute(ceilf(size.x / pow(2, i) / (float)localSize.x()), ceilf(size.y / pow(2, i) / (float)localSize.y()), 1);
		}

		context.setDrawInfo(3, 4, 0);
		context.bindTexture((ITexture*)bloomSampleMap.getVendorTexture(), sampleMapName);
		image.texture = resource->screenTexture;
		image.level = 0;
		context.dispatchCompute(ceilf(size.x / (float)localSize.x()), ceilf(size.y / (float)localSize.y()), 1);
	}
	else {
		context.clearFrameBindings();

		{
			RENDER_SCOPE(context, BloomFilterPixel);

			context.bindPipelineState(pipelineState);
			
			context.bindTexture(resource->screenTexture->getVendorTexture(), sampleMapName);
			context.bindFrame(bloomSampleRenderTargets[0]->getVendorRenderTarget());
			context.clearFrameColor({ 0, 0, 0, 255 });
			
			context.setDrawInfo(0, 0, 0);
			context.setViewport(0, 0, size.x, size.y);
			context.postProcessCall();
			
			context.bindTexture(NULL, sampleMapName);
			context.clearFrameBindings();
		}

		{
			RENDER_SCOPE(context, BloomDownSamplePixel);

			context.bindPipelineState(pipelineState);
			
			for (int i = 1; i < bloomLevel; ++i) {
				MipOption mipOption;
				mipOption.mipCount = 1;
				mipOption.detailMip = i - 1;
				context.bindTexture(bloomSampleMap.getVendorTexture(), sampleMapName, mipOption);
			
				context.bindFrame(bloomSampleRenderTargets[i]->getVendorRenderTarget());
				context.clearFrameColor({ 0, 0, 0, 255 });
			
				context.setDrawInfo(i == 1 ? 1 : 2, mipOption.detailMip, 0);
				context.setViewport(0, 0, size.x >> i, size.y >> i);
				context.postProcessCall();
			
				context.bindTexture(NULL, sampleMapName);
				context.clearFrameBindings();
			}
		}

		{
			RENDER_SCOPE(context, BloomUpSamplePixel);

			context.bindPipelineState(additivePipelineState);
			
			for (int i = bloomLevel - 2; i >= 0; --i) {
				MipOption mipOption;
				mipOption.mipCount = 1;
				mipOption.detailMip = i + 1;
				context.bindTexture((ITexture*)bloomSampleMap.getVendorTexture(), sampleMapName, mipOption);

				IRenderTarget* renderTarget = i == 0 ? resource->screenRenderTarget->getVendorRenderTarget() :
					bloomSampleRenderTargets[i]->getVendorRenderTarget();
				
				context.bindFrame(renderTarget);

				context.setDrawInfo(3, mipOption.detailMip, 0);
				context.setViewport(0, 0, size.x >> i, size.y >> i);
				context.postProcessCall();
			
				context.bindTexture(NULL, sampleMapName);
				context.clearFrameBindings();
			}
		}

		context.setDrawInfo(0, 1, 0);
	}
}

bool BloomPass::loadDefaultResource()
{
	if (material == NULL)
		material = getAssetByPath<Material>("Engine/Shaders/PostProcess/BloomPassFS.mat");
	if (material == NULL || resource == NULL || resource->screenTexture == NULL)
		return false;
	material->setCount("bloomLevel", bloomLevel);
	material->setScalar("width", size.x);
	material->setScalar("height", size.y);
	materialRenderData = material->getMaterialRenderData();
	return materialRenderData;
}

void BloomPass::render(RenderInfo& info)
{
	if (!enable)
		return;
	if (size.x == 0 || size.y == 0)
		return;
	if (!loadDefaultResource())
		return;

	RENDER_THREAD_ENQUEUE_TASK(AddBloomPass, ([this, materialRenderData = materialRenderData] (RenderThreadContext& context)
	{
		context.renderGraph->addPass(*this);
		if (materialRenderData)
			renderGraph->getRenderDataCollectorMainThread()->add(*materialRenderData);
	}));
}

void BloomPass::resize(const Unit2Di & size)
{
	PostProcessPass::resize(size);
	int newBloomLevel = 1 + floor(log2(max(size.x, size.y)));
	newBloomLevel = min(newBloomLevel, maxBloomLevel);
	resizeBloomLevel(newBloomLevel);
}

void BloomPass::resizeBloomLevel(int levels)
{
	int diffLevel = levels - bloomSampleRenderTargets.size();
	bloomLevel = levels;
	if (diffLevel > 0) {
		bloomSampleRenderTargets.reserve(bloomLevel);
		for (int i = 0; i < diffLevel; i++) {
			RenderTarget* bloomDownRT = new RenderTarget(size.x, size.y, 4);

			bloomDownRT->addTexture("sampleMap", bloomSampleMap);

			bloomSampleRenderTargets.push_back(bloomDownRT);
		}
	}
	else {
		for (int i = 0; i < -diffLevel; i++) {
			delete bloomSampleRenderTargets[i];
		}
		bloomSampleRenderTargets.resize(bloomLevel);
	}
	for (int level = 0; level < bloomLevel; level++) {
		RenderTarget*& bloomDownRT = bloomSampleRenderTargets[level];
		bloomDownRT->setTextureMipLevel(0, level);
		bloomDownRT->resize(size.x, size.y);
	}
}
