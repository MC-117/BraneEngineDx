#include "ToneMapPass.h"
#include "../Asset.h"
#include "../Console.h"
#include "../GUI/UIControl.h"
#include "../RenderCore/RenderCore.h"
#include "../RenderCore/RenderCoreUtility.h"

ToneMapPass::ToneMapPass(const Name & name, Material * material)
	: PostProcessPass(name, material)
{
	screenMap.setAutoGenMip(false);
	screenRenderTarget.addTexture("screenMap", screenMap);
}

void ToneMapPass::prepare()
{
	materialVaraint = materialRenderData->getVariant(Shader_Postprocess);
	if (materialVaraint == NULL) {
		Console::error("PostProcessPass: Shader_Postprocess not found in shader '%s'", materialRenderData->getShaderName().c_str());
		throw runtime_error("ShaderVariant not found");
		return;
	}
	materialVaraint->init();

	if (materialVaraint->program->isComputable()) {
		pipelineState = fetchPSOIfDescChangedThenInit(pipelineState, materialVaraint->program);
	}
	else {
		GraphicsPipelineStateDesc desc = GraphicsPipelineStateDesc::forScreen(
			materialVaraint->program, &screenRenderTarget, BM_Default);
		pipelineState = fetchPSOIfDescChangedThenInit(pipelineState, desc);
	}
}

void ToneMapPass::execute(IRenderContext& context)
{
	materialRenderData->upload();

	context.bindPipelineState(pipelineState);

	context.bindMaterialBuffer(materialVaraint);
	context.bindMaterialTextures(materialVaraint);

	if (materialVaraint->program->isComputable()) {
		static const ShaderPropertyName imageMapName = "imageMap";

		Image image;
		image.texture = resource->screenTexture;

		context.bindImage(image, imageMapName);

		Vector3u localSize = material->getLocalSize();
		localSize.x() = ceilf(size.x / (float)localSize.x());
		localSize.y() = ceilf(size.y / (float)localSize.y());

		context.setDrawInfo(0, 1, 0);
		context.dispatchCompute(localSize.x(), localSize.y(), 1);
	}
	else {
		static const ShaderPropertyName screenMapName = "screenMap";

		context.bindTexture((ITexture*)resource->screenTexture->getVendorTexture(), screenMapName);
		
		context.bindFrame(screenRenderTarget.getVendorRenderTarget());

		context.setDrawInfo(0, 1, 0);
		context.postProcessCall();

		context.clearFrameBindings();

		resource->screenTexture = &screenMap;
		resource->screenRenderTarget = &screenRenderTarget;
	}
}

bool ToneMapPass::loadDefaultResource()
{
	if (material == NULL)
		material = getAssetByPath<Material>("Engine/Shaders/PostProcess/ToneMapPassFS.mat");
	if (material == NULL || resource == NULL || resource->screenTexture == NULL)
		return false;
	materialRenderData = material->getMaterialRenderData();
	return materialRenderData;
}

void ToneMapPass::render(RenderInfo & info)
{
	if (!enable)
		return;
	if (size.x == 0 || size.y == 0)
		return;
	if (!loadDefaultResource())
		return;

	RENDER_THREAD_ENQUEUE_TASK(AddToneMapPass, ([this, materialRenderData = materialRenderData] (RenderThreadContext& context)
	{
		context.renderGraph->addPass(*this);
		if (materialRenderData)
			renderGraph->getRenderDataCollectorMainThread()->add(*materialRenderData);
	}));
}

void ToneMapPass::resize(const Unit2Di& size)
{
	PostProcessPass::resize(size);
	screenRenderTarget.resize(size.x, size.y);
}
