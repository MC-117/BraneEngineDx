#include "DOFPass.h"
#include "../Asset.h"
#include "../Console.h"

DOFPass::DOFPass(const string & name, Material * material)
	: PostProcessPass(name, material)
{
	dofRenderTarget.addTexture("dofMap", dofMap);
}

bool DOFPass::mapMaterialParameter(RenderInfo & info)
{
	if (material == NULL)
		material = getAssetByPath<Material>("Engine/Shaders/PostProcess/DOFPassFS.mat");
	if (material == NULL || resource == NULL ||
		resource->screenTexture == NULL || resource->depthTexture == NULL)
		return false;
	material->setTexture("depthMap", *resource->depthTexture);
	return true;
}

void DOFPass::render(RenderInfo & info)
{
	if (!enable)
		return;
	if (!mapMaterialParameter(info))
		return;
	if (size.x == 0 || size.y == 0)
		return;
	Texture** pScreenMap = material->getTexture("screenMap");
	if (pScreenMap == NULL)
		return;
	ShaderProgram* program = material->getShader()->getProgram(Shader_Postprocess);
	if (program == NULL) {
		Console::error("PostProcessPass: Shader_Postprocess not found in shader '%s'", material->getShaderName());
		return;
	}
	if (!program->isComputable()) {
		IVendor& vendor = VendorManager::getInstance().getVendor();

		program->bind();
		info.camera->bindCameraData();

		*pScreenMap = resource->screenTexture;

		dofRenderTarget.bindFrame();

		material->setPass(0);
		material->processBaseData();
		material->processInstanceData();

		vendor.setViewport(0, 0, size.x, size.y);

		vendor.postProcessCall();

		resource->screenTexture = &dofMap;
	}
}

void DOFPass::resize(const Unit2Di & size)
{
	PostProcessPass::resize(size);
	dofRenderTarget.resize(size.x, size.y);
}