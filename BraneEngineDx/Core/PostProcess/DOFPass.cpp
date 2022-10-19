#include "DOFPass.h"
#include "../Asset.h"
#include "../Console.h"
#include "../Camera.h"
#include "../GUI/UIControl.h"

DOFPass::DOFPass(const string & name, Material * material)
	: PostProcessPass(name, material)
{
	dofRenderTarget.addTexture("dofMap", dofMap);
}

void DOFPass::prepare()
{
	dofRenderTarget.init();
	MaterialRenderData* materialRenderData = (MaterialRenderData*)this->materialRenderData;
	materialRenderData->program = program;
	materialRenderData->create();
}

void DOFPass::execute(IRenderContext& context)
{
	materialRenderData->upload();

	context.bindShaderProgram(program);

	context.bindMaterialBuffer(((MaterialRenderData*)materialRenderData)->vendorMaterial);
	cameraRenderData->bind(context);

	context.bindFrame(dofRenderTarget.getVendorRenderTarget());

	context.bindTexture((ITexture*)resource->screenTexture->getVendorTexture(), Fragment_Shader_Stage, screenMapSlot);
	context.bindTexture((ITexture*)resource->depthTexture->getVendorTexture(), Fragment_Shader_Stage, depthMapSlot);

	context.setDrawInfo(0, 1, 0);

	context.setViewport(0, 0, size.x, size.y);

	context.postProcessCall();

	context.clearFrameBindings();

	resource->screenTexture = &dofMap;
	resource->screenRenderTarget = &dofRenderTarget;
}

bool DOFPass::mapMaterialParameter(RenderInfo & info)
{
	if (material == NULL)
		material = getAssetByPath<Material>("Engine/Shaders/PostProcess/DOFPassFS.mat");
	if (material == NULL || resource == NULL ||
		resource->screenTexture == NULL || resource->depthTexture == NULL)
		return false;
	materialRenderData = material->getRenderData();
	return materialRenderData;
}

void DOFPass::render(RenderInfo & info)
{
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

		screenMapSlot = program->getAttributeOffset("screenMap").offset;
		depthMapSlot = program->getAttributeOffset("depthMap").offset;
		if (screenMapSlot == -1 || depthMapSlot == -1)
			return;

		cameraRenderData = info.camera->getRenderData();

		info.renderGraph->addPass(*this);
	}
}

void DOFPass::resize(const Unit2Di & size)
{
	PostProcessPass::resize(size);
	dofRenderTarget.resize(size.x, size.y);
}