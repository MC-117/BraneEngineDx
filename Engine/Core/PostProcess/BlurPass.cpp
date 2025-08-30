#include "BlurPass.h"
#include "../Asset.h"
#include "../Console.h"
#include "../GUI/UIControl.h"

BlurPass::BlurPass(const Name& name, Material* material)
    : PostProcessPass(name, material)
{
}

bool BlurPass::loadDefaultResource()
{
	if (material == NULL)
		material = getAssetByPath<Material>("Engine/Shaders/PostProcess/BlurPass.mat");
	if (material == NULL || resource == NULL || resource->screenTexture == NULL)
		return false;
	return true;
}

void BlurPass::render(RenderInfo& info)
{
	if (!enable)
		return;
	if (!loadDefaultResource())
		return;
	if (size.x == 0 || size.y == 0)
		return;
	ShaderProgram* program = material->getShader()->getProgram(Shader_Postprocess);
	if (program == NULL || !program->init()) {
		Console::error("PostProcessPass: Shader_Postprocess not found in shader '%s'", material->getShaderName().c_str());
		return;
	}
	if (program->isComputable()) {
		program->bind();
		Vector3u localSize = material->getLocalSize();
		localSize.x() = ceilf(size.x / (float)localSize.x());
		localSize.y() = ceilf(size.y / (float)localSize.y());
		
		// *screenMapSlot = resource->screenTexture;
		// blurMapSlot->texture = &blurXMap;

		material->setPass(0);
		material->processInstanceData();
		program->dispatchCompute(localSize.x(), localSize.y(), 1);

		// *screenMapSlot = &blurXMap;
		// blurMapSlot->texture = &blurYMap;

		material->setPass(1);
		material->processInstanceData();
		program->dispatchCompute(localSize.x(), localSize.y(), 1);

		resource->blurTexture = &blurYMap;
	}
}

void BlurPass::resize(const Unit2Di& size)
{
	PostProcessPass::resize(size);
	blurXMap.resize(size.x, size.y);
	blurYMap.resize(size.x, size.y);
}
