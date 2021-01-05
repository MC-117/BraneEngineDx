#include "SSAOPass.h"
#include "Asset.h"

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

		program->bind();
		info.camera->bindCameraData();

		ssaoRenderTarget.resize(ssaoSize.x, ssaoSize.y);
		screenRenderTarget.resize(ssaoSize.x, ssaoSize.y);

		DrawElementsIndirectCommand cmd = { 4, 1, 0, 0 };

		material->setPass(0);
		material->processBaseData();
		material->processInstanceData();

		ssaoRenderTarget.bindFrame();

		glViewport(0, 0, ssaoSize.x, ssaoSize.y);
		glDrawArraysIndirect(GL_TRIANGLE_STRIP, &cmd);

		material->setPass(1);
		material->processBaseData();

		*pSsaoMap = &ssaoMap;
		material->processTextureData();

		screenRenderTarget.bindFrame();

		glViewport(0, 0, ssaoSize.x, ssaoSize.y);
		glDrawArraysIndirect(GL_TRIANGLE_STRIP, &cmd);

		material->setPass(2);
		material->processBaseData();

		*pSsaoMap = &screenMap;
		material->processTextureData();

		ssaoRenderTarget.bindFrame();

		glViewport(0, 0, ssaoSize.x, ssaoSize.y);
		glDrawArraysIndirect(GL_TRIANGLE_STRIP, &cmd);

		resource->ssaoTexture = &ssaoMap;
	}
}
