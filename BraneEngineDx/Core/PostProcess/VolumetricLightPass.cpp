#include "VolumetricLightPass.h"
#include "../Asset.h"
#include "../Console.h"
#include "../Camera.h"
#include "../GUI/UIControl.h"

VolumetricLightPass::VolumetricLightPass(const string& name, Material* material)
	: PostProcessPass(name, material)
{
	passAMap.setAutoGenMip(false);
	passBMap.setAutoGenMip(false);
	passARenderTarget.addTexture("temp1Map", passAMap);
	passBRenderTarget.addTexture("temp1Map", passBMap);
}

bool VolumetricLightPass::mapMaterialParameter(RenderInfo& info)
{
	if (material == NULL)
		material = getAssetByPath<Material>("Engine/Shaders/PostProcess/VolumetricLightFS.mat");
	if (material == NULL || resource == NULL || resource->screenRenderTarget == NULL ||
		resource->depthTexture == NULL || resource->lightDepthTexture == NULL)
		return false;
	temp1Map = material->getTexture("temp1Map");
	temp2Map = material->getTexture("temp2Map");
	if (temp1Map == NULL || temp2Map == NULL)
		return false;
	return true;
}

void VolumetricLightPass::render(RenderInfo& info)
{
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
		IVendor& vendor = VendorManager::getInstance().getVendor();
		Unit2Di screenSize = { size.x * screenScale, size.y * screenScale };

		program->bind();
		info.camera->bindCameraData();

		// Pass 0 VolumetricLight
		*temp1Map = resource->depthTexture;
		*temp2Map = resource->lightDepthTexture;

		passARenderTarget.bindFrame();

		material->setPass(0);
		material->processBaseData();
		material->processInstanceData();

		vendor.setViewport(0, 0, screenSize.x, screenSize.y);
		vendor.setRenderPostState();
		vendor.postProcessCall();

		// Pass 1 BlurX
		*temp1Map = &passAMap;
		*temp2Map = NULL;

		passBRenderTarget.bindFrame();

		material->setPass(1);
		material->processBaseData();
		material->processTextureData();

		vendor.setViewport(0, 0, screenSize.x, screenSize.y);
		vendor.setRenderPostState();
		vendor.postProcessCall();

		// Pass 2 BlurY
		*temp1Map = &passBMap;
		*temp2Map = NULL;

		passARenderTarget.bindFrame();

		material->setPass(2);
		material->processBaseData();
		material->processTextureData();

		vendor.setViewport(0, 0, screenSize.x, screenSize.y);
		vendor.setRenderPostState();
		vendor.postProcessCall();

		// Pass 3 Add
		*temp1Map = &passAMap;
		*temp2Map = NULL;

		resource->screenRenderTarget->bindFrame();

		material->setPass(3);
		material->processBaseData();
		material->processTextureData();

		vendor.setViewport(0, 0, size.x, size.y);
		vendor.setRenderPostAddState();
		vendor.postProcessCall();

		vendor.setRenderPostState();

		resource->screenRenderTarget->clearBind();

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
