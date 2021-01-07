#include "BlitPass.h"
#include "../Asset.h"
#include "../Console.h"

BlitPass::BlitPass(const string & name, Material * material)
	: PostProcessPass(name, material)
{
}

bool BlitPass::mapMaterialParameter(RenderInfo & info)
{
	if (resource == NULL || resource->finalRenderTarget == NULL)
		return false;
	if (material == NULL)
		material = getAssetByPath<Material>("Engine/Shaders/PostProcess/BlitPass.mat");
	if (resource == NULL || resource->screenTexture == NULL || resource->finalRenderTarget == NULL)
		return false;
	material->setTexture("screenMap", *resource->screenTexture);
	return true;
}

void BlitPass::render(RenderInfo & info)
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
	program->bind();
	resource->finalRenderTarget->bindFrame();
	material->processInstanceData();
	IVendor& vendor = VendorManager::getInstance().getVendor();
	for (int i = 0; i < material->getPassNum(); i++) {
		material->setPass(i);
		material->processBaseData();
		resource->finalRenderTarget->clearColor({ 0, 0, 0, 0 });
		resource->finalRenderTarget->clearDepth(1);
		vendor.setViewport(0, 0, size.x, size.y);
		vendor.postProcessCall();
	}
	material->setPass(0);
}
