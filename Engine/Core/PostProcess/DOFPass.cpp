#include "DOFPass.h"
#include "../Asset.h"
#include "../Console.h"
#include "../Camera.h"
#include "../GUI/UIControl.h"
#include "../RenderCore/RenderCore.h"

DOFPass::DOFPass(const string & name, Material * material)
	: PostProcessPass(name, material)
{
	dofRenderTarget.addTexture("dofMap", dofMap);
}

void DOFPass::prepare()
{
	Texture2D* sceneMap = dynamic_cast<Texture2D*>(resource->screenTexture);
	if (sceneMap)
		dofMap.setTextureInfo(sceneMap->getTextureInfo());
	dofRenderTarget.init();
	MaterialRenderData* materialRenderData = (MaterialRenderData*)this->materialRenderData;
	materialRenderData->program = program;
	materialRenderData->create();
}

void DOFPass::execute(IRenderContext& context)
{
	static const ShaderPropertyName screenMapName = "screenMap";
	static const ShaderPropertyName depthMapName = "depthMap";

	materialRenderData->upload();

	context.bindShaderProgram(program);

	context.bindMaterialBuffer(((MaterialRenderData*)materialRenderData)->vendorMaterial);
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

bool DOFPass::mapMaterialParameter(RenderInfo & info)
{
	if (material == NULL)
		material = getAssetByPath<Material>("Engine/Shaders/PostProcess/DOFPassFS.mat");
	cameraRenderData = resource->cameraRenderData;
	if (material == NULL || resource == NULL || cameraRenderData == NULL ||
		resource->screenTexture == NULL || resource->depthTexture == NULL)
		return false;
	materialRenderData = material->getRenderData();
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
	if (!mapMaterialParameter(info))
		return;
	if (size.x == 0 || size.y == 0)
		return;
	program = material->getShader()->getProgram(Shader_Postprocess);
	if (program == NULL) {
		Console::error("PostProcessPass: Shader_Postprocess not found in shader '%s'", material->getShaderName().c_str());
		return;
	}

	if (!program->isComputable()) {
		program->init();

		info.renderGraph->addPass(*this);
	}
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