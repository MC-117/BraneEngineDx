#include "SSAOPass.h"
#include "../Asset.h"
#include "../Console.h"
#include "../Camera.h"
#include "../GUI/UIControl.h"

SSAOPass::SSAOPass(const string & name, Material * material)
	: PostProcessPass(name, material)
{
	passAMap.setAutoGenMip(false);
	passBMap.setAutoGenMip(false);
	screenMap.setAutoGenMip(false);
	passARenderTarget.addTexture("ssaoMap", passAMap);
	passBRenderTarget.addTexture("ssaoMap", passBMap);
	screenRenderTarget.addTexture("screenMap", screenMap);
}

bool SSAOPass::mapMaterialParameter(RenderInfo & info)
{
	if (material == NULL)
		material = getAssetByPath<Material>("Engine/Shaders/PostProcess/SSAOPassFS.mat");
	if (material == NULL || resource == NULL ||
		resource->screenTexture == NULL || resource->depthTexture == NULL)
		return false;
	ssaoMapSlot = material->getTexture("ssaoMap");
	if (ssaoMapSlot == NULL)
		return false;
	screenMapSlot = material->getTexture("screenMap");
	if (screenMapSlot == NULL)
		return false;
	material->setTexture("depthMap", *resource->depthTexture);
	return true;
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
	ShaderProgram* program = material->getShader()->getProgram(Shader_Postprocess);
	if (program == NULL) {
		Console::error("PostProcessPass: Shader_Postprocess not found in shader '%s'", material->getShaderName());
		return;
	}
	if (!program->isComputable()) {
		Unit2Di ssaoSize = { size.x * screenScale, size.y * screenScale };
		IVendor& vendor = VendorManager::getInstance().getVendor();

		program->bind();
		info.camera->bindCameraData();

		// Pass 0 SSAO
		*ssaoMapSlot = NULL;
		*screenMapSlot = NULL;
		passARenderTarget.bindFrame();

		material->setPass(0);
		material->processInstanceData();

		vendor.setViewport(0, 0, ssaoSize.x, ssaoSize.y);
		vendor.setRenderPostState();
		vendor.postProcessCall();

		passARenderTarget.clearBind();

		// Pass 1 BlurX
		*ssaoMapSlot = &passAMap;
		*screenMapSlot = NULL;
		passBRenderTarget.bindFrame();

		material->setPass(1);
		material->processBaseData();
		material->processTextureData();

		vendor.setViewport(0, 0, ssaoSize.x, ssaoSize.y);
		vendor.setRenderPostState();
		vendor.postProcessCall();

		passBRenderTarget.clearBind();

		// Pass 2 BlurY
		*ssaoMapSlot = &passBMap;
		*screenMapSlot = NULL;

		passARenderTarget.bindFrame();

		material->setPass(2);
		material->processBaseData();
		material->processTextureData();

		vendor.setViewport(0, 0, ssaoSize.x, ssaoSize.y);
		vendor.setRenderPostState();
		vendor.postProcessCall();

		passARenderTarget.clearBind();

		// Pass 3 Add
		*ssaoMapSlot = &passAMap;
		*screenMapSlot = resource->screenTexture;

		screenRenderTarget.bindFrame();

		material->setPass(3);
		material->processBaseData();
		material->processTextureData();

		vendor.setViewport(0, 0, size.x, size.y);
		vendor.setRenderPostState();
		vendor.postProcessCall();

		screenRenderTarget.clearBind();

		resource->screenRenderTarget = &screenRenderTarget;
		resource->screenTexture = &screenMap;
	}
}

void SSAOPass::resize(const Unit2Di& size)
{
	PostProcessPass::resize(size);
	passARenderTarget.resize(size.x * screenScale, size.y * screenScale);
	passBRenderTarget.resize(size.x * screenScale, size.y * screenScale);
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
		passARenderTarget.resize(size.x * screenScale, size.y * screenScale);
		passBRenderTarget.resize(size.x * screenScale, size.y * screenScale);
	}
}

float SSAOPass::getScreenScale()
{
	return screenScale;
}
