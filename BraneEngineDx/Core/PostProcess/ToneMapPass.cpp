#include "ToneMapPass.h"
#include "../Asset.h"
#include "../Console.h"

ToneMapPass::ToneMapPass(const string & name, Material * material)
	: PostProcessPass(name, material)
{
}

bool ToneMapPass::mapMaterialParameter(RenderInfo & info)
{
	if (material == NULL)
		material = getAssetByPath<Material>("Engine/Shaders/PostProcess/ToneMapPass.mat");
	if (material == NULL || resource == NULL || resource->screenTexture == NULL)
		return false;
	Image img;
	img.texture = resource->screenTexture;
	material->setImage("imageMap", img);
	if (resource->ssaoTexture == NULL)
		material->setTexture("ssaoMap", Texture2D::whiteRGBADefaultTex);
	else
		material->setTexture("ssaoMap", *resource->ssaoTexture);
	return true;
}

void ToneMapPass::render(RenderInfo & info)
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
	if (program->isComputable()) {
		program->bind();
		Unit2Du localSize = material->getLocalSize();
		localSize.x = ceilf(size.x / (float)localSize.x);
		localSize.y = ceilf(size.y / (float)localSize.y);
		material->setPass(0);
		material->processInstanceData();
		program->dispatchCompute(localSize.x, localSize.y, 1);
	}
}
