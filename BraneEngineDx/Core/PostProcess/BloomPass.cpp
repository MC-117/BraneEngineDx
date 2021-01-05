#include "BloomPass.h"
#include "Camera.h"
#include "Asset.h"

BloomPass::BloomPass(const string & name, Material * material)
	: PostProcessPass(name, material)
{
	bloomRenderTarget.addTexture("sampleMap", bloomMap);
	screenRenderTarget.addTexture("screenMap", screenMap);
}

bool BloomPass::mapMaterialParameter(RenderInfo & info)
{
	if (material == NULL)
		material = getAssetByPath<Material>("Engine/Shaders/PostProcess/BloomPassFS.mat");
	if (material == NULL || resource == NULL || resource->screenTexture == NULL)
		return false;
	bloomLevel = 1 + floor(log2(max(size.x, size.y)));
	bloomLevel = min(bloomLevel, 9);
	material->setCount("bloomLevel", bloomLevel);
	return true;
}

void BloomPass::render(RenderInfo & info)
{
	if (!enable)
		return;
	if (!mapMaterialParameter(info))
		return;
	if (size.x == 0 || size.y == 0)
		return;
	Texture** sampleMap = material->getTexture("sampleMap");
	if (sampleMap == NULL)
		return;
	ShaderProgram* program = material->getShader()->getProgram(Shader_Postprocess);
	if (program == NULL) {
		Console::error("PostProcessPass: Shader_Postprocess not found in shader '%s'", material->getShaderName());
		return;
	}
	if (program->isComputable()) {
		Image* imageMap = material->getImage("imageMap");
		if (imageMap == NULL)
			return;
		program->bind();
		Unit2Du localSize = material->getLocalSize();

		material->setPass(0);
		*sampleMap = resource->screenTexture;
		imageMap->texture = &bloomMap;
		material->processBaseData();
		material->processInstanceData();
		for (int i = 0; i < bloomLevel; i++) {
			imageMap->level = i;
			material->processImageData();
			program->dispatchCompute(ceilf(size.x / pow(2, i) / (float)localSize.x), ceilf(size.y / pow(2, i) / (float)localSize.y), 1);
		}

		material->setPass(1);
		material->processBaseData();
		for (int i = 0; i < bloomLevel; i++) {
			imageMap->level = i;
			material->processImageData();
			program->dispatchCompute(ceilf(size.x / pow(2, i) / (float)localSize.x), ceilf(size.y / pow(2, i) / (float)localSize.y), 1);
		}

		material->setPass(2);
		material->processBaseData();
		for (int i = 0; i < bloomLevel; i++) {
			imageMap->level = i;
			material->processImageData();
			program->dispatchCompute(ceilf(size.x / pow(2, i) / (float)localSize.x), ceilf(size.y / pow(2, i) / (float)localSize.y), 1);
		}

		material->setPass(3);
		*sampleMap = &bloomMap;
		imageMap->texture = resource->screenTexture;
		imageMap->level = 0;
		material->processBaseData();
		material->processTextureData();
		material->processImageData();
		program->dispatchCompute(ceilf(size.x / (float)localSize.x), ceilf(size.y / (float)localSize.y), 1);

		material->setPass(0);
	}
	else {
		Texture** pScreenMap = material->getTexture("screenMap");
		float* pWidth = material->getScaler("width");
		float* pHeight = material->getScaler("height");
		if (pScreenMap == NULL)
			return;

		*sampleMap = resource->screenTexture;
		*pScreenMap = NULL;

		program->bind();

		DrawElementsIndirectCommand cmd = { 4, 1, 0, 0 };

		material->setPass(0);
		material->processBaseData();
		material->processInstanceData();

		for (int i = 0; i < bloomLevel; i++) {
			int scalar = pow(2, i);

			*pWidth = size.x / scalar;
			*pHeight = size.y / scalar;

			material->processScalarData();

			bloomRenderTarget.setTextureMipLevel(0, i);
			bloomRenderTarget.bindFrame();

			glViewport(0, 0, *pWidth, *pHeight);

			glDrawArraysIndirect(GL_TRIANGLE_STRIP, &cmd);
		}

		material->setPass(1);
		material->processBaseData();

		*sampleMap = &bloomMap;
		material->processTextureData();

		for (int i = 0; i < bloomLevel; i++) {
			int scalar = pow(2, i);

			*pWidth = size.x / scalar;
			*pHeight = size.y / scalar;

			material->processScalarData();

			screenRenderTarget.setTextureMipLevel(0, i);
			screenRenderTarget.bindFrame();

			glViewport(0, 0, *pWidth, *pHeight);

			glDrawArraysIndirect(GL_TRIANGLE_STRIP, &cmd);
		}

		material->setPass(2);
		material->processBaseData();

		*sampleMap = &screenMap;
		material->processTextureData();

		for (int i = 0; i < bloomLevel; i++) {
			int scalar = pow(2, i);

			*pWidth = size.x / scalar;
			*pHeight = size.y / scalar;

			material->processScalarData();

			bloomRenderTarget.setTextureMipLevel(0, i);
			bloomRenderTarget.bindFrame();

			glViewport(0, 0, *pWidth, *pHeight);

			glDrawArraysIndirect(GL_TRIANGLE_STRIP, &cmd);
		}

		material->setPass(3);
		material->processBaseData();

		*sampleMap = &bloomMap;
		*pScreenMap = resource->screenTexture;
		material->processTextureData();

		screenRenderTarget.setTextureMipLevel(0, 0);
		screenRenderTarget.bindFrame();
		glViewport(0, 0, size.x, size.y);
		glDrawArraysIndirect(GL_TRIANGLE_STRIP, &cmd);

		resource->screenTexture = &screenMap;
	}
}

void BloomPass::resize(const Unit2Di & size)
{
	PostProcessPass::resize(size);
	bloomRenderTarget.resize(size.x, size.y);
	screenRenderTarget.resize(size.x, size.y);
}
