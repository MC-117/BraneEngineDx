#include "BlitPass.h"
#include "Asset.h"

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
	DrawElementsIndirectCommand cmd = { 4, 1, 0, 0 };
	for (int i = 0; i < material->getPassNum(); i++) {
		material->setPass(i);
		material->processBaseData();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, size.x, size.y);
		glDrawArraysIndirect(GL_TRIANGLE_STRIP, &cmd);
	}
	material->setPass(0);
}
