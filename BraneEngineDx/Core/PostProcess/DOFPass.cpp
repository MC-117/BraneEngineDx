#include "DOFPass.h"
#include "Asset.h"

DOFPass::DOFPass(const string & name, Material * material)
	: PostProcessPass(name, material)
{
	screenRenderTarget.addTexture("screenMap", screenMap);
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
	Texture** pDofMap = material->getTexture("dofMap");
	Texture** pScreenMap = material->getTexture("screenMap");
	if (pDofMap == NULL || pScreenMap == NULL)
		return;
	ShaderProgram* program = material->getShader()->getProgram(Shader_Postprocess);
	if (program == NULL) {
		Console::error("PostProcessPass: Shader_Postprocess not found in shader '%s'", material->getShaderName());
		return;
	}
	if (!program->isComputable()) {
		DrawElementsIndirectCommand cmd = { 4, 1, 0, 0 };

		program->bind();
		info.camera->bindCameraData();

		*pDofMap = NULL;
		*pScreenMap = resource->screenTexture;

		material->setPass(0);
		material->processBaseData();
		material->processInstanceData();

		dofRenderTarget.bindFrame();
		glViewport(0, 0, size.x, size.y);

		glDrawArraysIndirect(GL_TRIANGLE_STRIP, &cmd);


		*pDofMap = &dofMap;
		*pScreenMap = resource->screenTexture;

		material->setPass(1);
		material->processBaseData();
		material->processTextureData();

		screenRenderTarget.bindFrame();
		glViewport(0, 0, size.x, size.y);

		glDrawArraysIndirect(GL_TRIANGLE_STRIP, &cmd);

		resource->screenTexture = &screenMap;
	}
}

void DOFPass::resize(const Unit2Di & size)
{
	PostProcessPass::resize(size);
	dofRenderTarget.resize(size.x, size.y);
	screenRenderTarget.resize(size.x, size.y);
}