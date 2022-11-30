#include "VolumetricLightPass.h"
#include "../Asset.h"
#include "../Console.h"
#include "../Camera.h"
#include "../GUI/UIControl.h"
#include "../RenderCore/RenderCore.h"

VolumetricLightPass::VolumetricLightPass(const string& name, Material* material)
	: PostProcessPass(name, material)
{
	passAMap.setAutoGenMip(false);
	passBMap.setAutoGenMip(false);
	passARenderTarget.addTexture("temp1Map", passAMap);
	passBRenderTarget.addTexture("temp1Map", passBMap);
}

void VolumetricLightPass::prepare()
{
	passARenderTarget.init();
	passBRenderTarget.init();

	MaterialRenderData* materialRenderData = (MaterialRenderData*)this->materialRenderData;
	materialRenderData->program = program;
	materialRenderData->create();
}

void VolumetricLightPass::execute(IRenderContext& context)
{
	Unit2Di screenSize = { size.x * screenScale, size.y * screenScale };

	materialRenderData->upload();

	context.bindShaderProgram(program);

	context.bindMaterialBuffer(((MaterialRenderData*)materialRenderData)->vendorMaterial);
	cameraRenderData->bind(context);

	// Pass 0 VolumetricLight
	context.bindFrame(passARenderTarget.getVendorRenderTarget());

	context.bindTexture((ITexture*)resource->depthTexture->getVendorTexture(), Fragment_Shader_Stage, temp1MapSlot, temp1MapSamplerSlot);
	context.bindTexture((ITexture*)directShadowMap->getVendorTexture(), Fragment_Shader_Stage, temp2MapSlot, temp2MapSamplerSlot);

	context.setDrawInfo(0, 4, 0);

	context.setViewport(0, 0, screenSize.x, screenSize.y);
	context.setRenderPostState();
	context.postProcessCall();

	// Pass 1 BlurX
	context.bindFrame(passBRenderTarget.getVendorRenderTarget());

	context.bindTexture((ITexture*)passAMap.getVendorTexture(), Fragment_Shader_Stage, temp1MapSlot, temp1MapSamplerSlot);
	context.bindTexture((ITexture*)Texture2D::blackRGBADefaultTex.getVendorTexture(), Fragment_Shader_Stage, temp2MapSlot, temp2MapSamplerSlot);

	context.setDrawInfo(1, 4, 0);

	context.setViewport(0, 0, screenSize.x, screenSize.y);
	context.setRenderPostState();
	context.postProcessCall();

	// Pass 2 BlurY
	context.bindTexture(NULL, Fragment_Shader_Stage, temp1MapSlot, temp1MapSamplerSlot);
	context.bindFrame(passARenderTarget.getVendorRenderTarget());

	context.bindTexture((ITexture*)passBMap.getVendorTexture(), Fragment_Shader_Stage, temp1MapSlot, temp1MapSamplerSlot);
	context.bindTexture((ITexture*)Texture2D::blackRGBADefaultTex.getVendorTexture(), Fragment_Shader_Stage, temp2MapSlot, temp2MapSamplerSlot);

	context.setDrawInfo(2, 4, 0);

	context.setViewport(0, 0, screenSize.x, screenSize.y);
	context.setRenderPostState();
	context.postProcessCall();

	// Pass 3 Add
	context.bindFrame(resource->screenRenderTarget->getVendorRenderTarget());

	context.bindTexture((ITexture*)passAMap.getVendorTexture(), Fragment_Shader_Stage, temp1MapSlot, temp1MapSamplerSlot);
	context.bindTexture((ITexture*)Texture2D::blackRGBADefaultTex.getVendorTexture(), Fragment_Shader_Stage, temp2MapSlot, temp2MapSamplerSlot);

	context.setDrawInfo(3, 4, 0);

	context.setViewport(0, 0, size.x, size.y);
	context.setRenderPostAddState();
	context.postProcessCall();

	context.setRenderPostState();

	context.clearFrameBindings();
}

bool VolumetricLightPass::mapMaterialParameter(RenderInfo& info)
{
	if (material == NULL)
		material = getAssetByPath<Material>("Engine/Shaders/PostProcess/VolumetricLightFS.mat");
	if (material == NULL || resource == NULL || resource->screenRenderTarget == NULL ||
		resource->depthTexture == NULL || resource->sceneRenderData == NULL ||
		resource->sceneRenderData->lightDataPack.shadowTarget == NULL)
		return false;
	directShadowMap = resource->sceneRenderData->lightDataPack.shadowTarget->getDepthTexture();
	if (directShadowMap == NULL)
		return false;
	materialRenderData = material->getRenderData();
	return materialRenderData;
}

void VolumetricLightPass::render(RenderInfo& info)
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

		temp1MapSlot = program->getAttributeOffset("temp1Map").offset;
		temp1MapSamplerSlot = program->getAttributeOffset("temp1MapSampler").offset;
		temp2MapSlot = program->getAttributeOffset("temp2Map").offset;
		temp2MapSamplerSlot = program->getAttributeOffset("temp2MapSampler").offset;
		cameraRenderData = resource->cameraRenderData;

		if (temp1MapSlot >= 0 && temp2MapSlot >= 0 && cameraRenderData)
			info.renderGraph->addPass(*this);
	}
}

void VolumetricLightPass::resize(const Unit2Di& size)
{
	PostProcessPass::resize(size);
	passARenderTarget.resize(size.x * screenScale, size.y * screenScale);
	passBRenderTarget.resize(size.x * screenScale, size.y * screenScale);
}

void VolumetricLightPass::onGUI(GUIRenderInfo& info)
{
	PostProcessPass::onGUI(info);
	float scale = screenScale;
	if (ImGui::InputFloat("ScreenScale", &scale)) {
		setScreenScale(scale);
	}
}

void VolumetricLightPass::setScreenScale(float scale)
{
	if (screenScale != scale && scale > 0) {
		screenScale = scale;
		passARenderTarget.resize(size.x * screenScale, size.y * screenScale);
		passBRenderTarget.resize(size.x * screenScale, size.y * screenScale);
	}
}

float VolumetricLightPass::getScreenScale()
{
	return screenScale;
}
