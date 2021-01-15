#include "SSAOPass.h"
#include "../Asset.h"
#include "../Console.h"

SSAOPass::SSAOPass(const string & name, Material * material)
	: PostProcessPass(name, material)
{
	ssaoMap.setAutoGenMip(false);
	screenMap.setAutoGenMip(false);
	ssaoRenderTarget.addTexture("ssaoMap", ssaoMap);
	screenRenderTarget.addTexture("screenMap", screenMap);
}

bool SSAOPass::mapMaterialParameter(RenderInfo & info)
{
	if (material == NULL)
		material = getAssetByPath<Material>("Engine/Shaders/PostProcess/SSAOPassFS.mat");
	if (material == NULL || resource == NULL ||
		resource->screenTexture == NULL || resource->depthTexture == NULL)
		return false;
	Texture2D* ssaoKernal = getAssetByPath<Texture2D>("Engine/Textures/ssaoRand.png");
	pScreenScale = material->getScaler("screenScale");
	material->setTexture("depthMap", *resource->depthTexture);
	if (ssaoKernal != NULL)
		material->setTexture("ssaoKernalMap", *ssaoKernal);
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
	Texture** pSsaoMap = material->getTexture("ssaoMap");
	if (pSsaoMap == NULL)
		return;
	ShaderProgram* program = material->getShader()->getProgram(Shader_Postprocess);
	if (program == NULL) {
		Console::error("PostProcessPass: Shader_Postprocess not found in shader '%s'", material->getShaderName());
		return;
	}
	if (*pScreenScale > 0 && !program->isComputable()) {
		Unit2Di ssaoSize = { size.x * *pScreenScale, size.y * *pScreenScale };
		IVendor& vendor = VendorManager::getInstance().getVendor();

		program->bind();
		info.camera->bindCameraData();

		ssaoRenderTarget.resize(ssaoSize.x, ssaoSize.y);
		screenRenderTarget.resize(ssaoSize.x, ssaoSize.y);

		ssaoRenderTarget.bindFrame();

		material->setPass(0);
		material->processInstanceData();

		vendor.setViewport(0, 0, ssaoSize.x, ssaoSize.y);
		vendor.postProcessCall();

		screenRenderTarget.bindFrame();

		material->setPass(1);

		*pSsaoMap = &ssaoMap;
		material->processInstanceData();

		vendor.setViewport(0, 0, ssaoSize.x, ssaoSize.y);
		vendor.postProcessCall();

		ssaoRenderTarget.bindFrame();

		material->setPass(2);

		*pSsaoMap = &screenMap;
		material->processInstanceData();

		vendor.setViewport(0, 0, ssaoSize.x, ssaoSize.y);
		vendor.postProcessCall();

		resource->ssaoTexture = &ssaoMap;
	}
}
