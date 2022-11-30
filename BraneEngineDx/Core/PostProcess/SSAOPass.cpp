#include "SSAOPass.h"
#include "../Asset.h"
#include "../Console.h"
#include "../Camera.h"
#include "../GUI/UIControl.h"
#include "../RenderCore/RenderTask.h"

SSAOPass::SSAOPass(const string & name, Material * material)
	: PostProcessPass(name, material)
{
	gtaoMap.setAutoGenMip(false);
	screenMap.setAutoGenMip(false);
	gtaoRenderTarget.addTexture("ssaoMap", gtaoMap);
	screenRenderTarget.addTexture("screenMap", screenMap);
}

void SSAOPass::prepare()
{
	gtaoRenderTarget.init();
	screenRenderTarget.init();
	MaterialRenderData* materialRenderData = (MaterialRenderData*)this->materialRenderData;
	materialRenderData->program = program;
	materialRenderData->create();
}

void SSAOPass::execute(IRenderContext& context)
{
	Unit2Di ssaoSize = { size.x * screenScale, size.y * screenScale };

	materialRenderData->upload();

	context.bindShaderProgram(program);
	cameraRenderData->bind(context);

	context.bindMaterialBuffer(((MaterialRenderData*)materialRenderData)->vendorMaterial);

	context.bindTexture((ITexture*)resource->depthTexture->getVendorTexture(), Fragment_Shader_Stage, depthMapSlot, depthMapSamplerSlot);

	// Pass 0 GTAO
	context.bindTexture(NULL, Fragment_Shader_Stage, ssaoMapSlot, ssaoMapSamplerSlot);
	context.bindTexture(NULL, Fragment_Shader_Stage, screenMapSlot, screenMapSamplerSlot);
	context.bindFrame(gtaoRenderTarget.getVendorRenderTarget());

	context.setDrawInfo(0, 2, 0);

	context.setViewport(0, 0, ssaoSize.x, ssaoSize.y);
	context.setRenderPostState();
	context.postProcessCall();

	context.clearFrameBindings();

	// Pass 1 Add
	context.bindTexture((ITexture*)gtaoMap.getVendorTexture(), Fragment_Shader_Stage, ssaoMapSlot, ssaoMapSamplerSlot);
	context.bindTexture((ITexture*)resource->screenTexture->getVendorTexture(), Fragment_Shader_Stage, screenMapSlot, screenMapSamplerSlot);

	context.bindFrame(screenRenderTarget.getVendorRenderTarget());

	context.setDrawInfo(1, 2, 0);

	context.setViewport(0, 0, size.x, size.y);
	context.setRenderPostState();
	context.postProcessCall();

	context.clearFrameBindings();

	resource->screenRenderTarget = &screenRenderTarget;
	resource->screenTexture = &screenMap;
}

bool SSAOPass::mapMaterialParameter(RenderInfo & info)
{
	if (material == NULL)
		material = getAssetByPath<Material>("Engine/Shaders/PostProcess/SSAOPassFS.mat");
	if (material == NULL || resource == NULL ||
		resource->screenTexture == NULL || resource->depthTexture == NULL)
		return false;
	materialRenderData = material->getRenderData();
	return materialRenderData;
}

void SSAOPass::render(RenderInfo & info)
{
	resource->ssaoTexture = NULL;
	if (!enable)
		return;
	if (!mapMaterialParameter(info))
		return;
	if (size.x == 0 || size.y == 0)
		return;
	program = material->getShader()->getProgram(Shader_Postprocess);
	if (program == NULL) {
		Console::error("PostProcessPass: Shader_Postprocess not found in shader '%s'", material->getShaderName());
		return;
	}
	if (!program->isComputable()) {
		program->init();

		depthMapSlot = program->getAttributeOffset("depthMap").offset;
		depthMapSamplerSlot = program->getAttributeOffset("depthMapSampler").offset;
		ssaoMapSlot = program->getAttributeOffset("ssaoMap").offset;
		ssaoMapSamplerSlot = program->getAttributeOffset("ssaoMapSampler").offset;
		screenMapSlot = program->getAttributeOffset("screenMap").offset;
		screenMapSamplerSlot = program->getAttributeOffset("screenMapSampler").offset;

		if (depthMapSlot == -1 || ssaoMapSlot == -1 || screenMapSlot == -1)
			return;

		cameraRenderData = resource->cameraRenderData;
		if (cameraRenderData == NULL)
			return;

		info.renderGraph->addPass(*this);
	}
}

void SSAOPass::resize(const Unit2Di& size)
{
	PostProcessPass::resize(size);
	gtaoRenderTarget.resize(size.x * screenScale, size.y * screenScale);
	screenRenderTarget.resize(size.x, size.y);
}

void SSAOPass::onGUI(GUIRenderInfo& info)
{
	PostProcessPass::onGUI(info);
	float scale = screenScale;
	if (ImGui::InputFloat("ScreenScale", &scale)) {
		setScreenScale(scale);
	}
}

void SSAOPass::setScreenScale(float scale)
{
	if (screenScale != scale && scale > 0) {
		screenScale = scale;
		gtaoRenderTarget.resize(size.x * screenScale, size.y * screenScale);
	}
}

float SSAOPass::getScreenScale()
{
	return screenScale;
}
