#include "DOFPass.h"
#include "../Asset.h"
#include "../Console.h"
#include "../Camera.h"
#include "../GUI/UIControl.h"
#include "../RenderCore/RenderCore.h"
#include "../RenderCore/RenderCoreUtility.h"

DOFPass::DOFPass(const Name & name, Material * material)
	: PostProcessPass(name, material)
{
	dofRenderTarget.addTexture("dofMap", dofMap);
}

void DOFPass::prepare()
{
	materialVaraint = materialRenderData->getVariant(Shader_Postprocess);
	if (materialVaraint == NULL) {
		Console::error("PostProcessPass: Shader_Postprocess not found in shader '%s'", materialRenderData->getShaderName().c_str());
		throw runtime_error("ShaderVariant not found");
		return;
	}

	if (materialVaraint->isComputable()) {
		throw runtime_error("Shader type mismatch");
		return;
	}

	materialVaraint->init();
	
	Texture2D* sceneMap = dynamic_cast<Texture2D*>(resource->screenTexture);
	if (sceneMap)
		dofMap.setTextureInfo(sceneMap->getTextureInfo());
	dofRenderTarget.init();

	GraphicsPipelineStateDesc desc = GraphicsPipelineStateDesc::forScreen(
		materialVaraint->program, &dofRenderTarget, BM_Default);
	pipelineState = fetchPSOIfDescChangedThenInit(pipelineState, desc);
}

void DOFPass::execute(IRenderContext& context)
{
	static const ShaderPropertyName screenMapName = "screenMap";
	static const ShaderPropertyName depthMapName = "depthMap";

	materialRenderData->upload();

	context.bindPipelineState(pipelineState);

	context.bindMaterialBuffer(materialVaraint);
	cameraRenderData->bind(context);

	context.bindFrame(dofRenderTarget.getVendorRenderTarget());

	context.bindTexture((ITexture*)resource->screenTexture->getVendorTexture(), screenMapName);
	context.bindTexture((ITexture*)resource->depthTexture->getVendorTexture(), depthMapName);

	context.setDrawInfo(0, 1, 0);

	context.setViewport(0, 0, size.x, size.y);

	context.postProcessCall();

	context.clearFrameBindings();

	resource->screenTexture = &dofMap;
	resource->screenRenderTarget = &dofRenderTarget;
}

bool DOFPass::loadDefaultResource()
{
	if (material == NULL)
		material = getAssetByPath<Material>("Engine/Shaders/PostProcess/DOFPassFS.mat");
	cameraRenderData = resource->cameraRenderData;
	if (material == NULL || resource == NULL || cameraRenderData == NULL ||
		resource->screenTexture == NULL || resource->depthTexture == NULL)
		return false;
	materialRenderData = material->getMaterialRenderData();
	if (autoFocus) {
		CameraRenderData* data = dynamic_cast<CameraRenderData*>(cameraRenderData);
		material->setScalar("focusDistance", data->data.distance + focusLengthOffset);
	}
	return materialRenderData;
}

void DOFPass::render(RenderInfo & info)
{
	if (!enable)
		return;
	if (size.x == 0 || size.y == 0)
		return;
	if (!loadDefaultResource())
		return;

	RENDER_THREAD_ENQUEUE_TASK(AddDOFPass, ([this, materialRenderData = materialRenderData] (RenderThreadContext& context)
	{
		context.renderGraph->addPass(*this);
		if (materialRenderData)
			renderGraph->getRenderDataCollectorMainThread()->add(*materialRenderData);
	}));
}

void DOFPass::onGUI(EditorInfo& info)
{
	ImGui::Checkbox("AutoFocus", &autoFocus);
	ImGui::DragFloat("FocusLengthOffset", &focusLengthOffset);
}

void DOFPass::resize(const Unit2Di & size)
{
	PostProcessPass::resize(size);
	dofRenderTarget.resize(size.x, size.y);
}