#include "HiZPass.h"
#include "../Asset.h"

void HiZPass::prepare()
{
	if (material == NULL) {
		material = getAssetByPath<Material>("Engine/Shaders/Pipeline/GenHiZ.mat");
	}
	if (material) {
		program = material->getShader()->getProgram(Shader_Default);
	}
}

void HiZPass::execute(IRenderContext& context)
{
	if (program == NULL || depthTexture == NULL || hizTexture == NULL)
		return;
	int miplevel = hizTexture->getMipLevels();
	int width = hizTexture->getWidth();
	int height = hizTexture->getHeight();
	context.bindShaderProgram(program);

	Image image;
	image.level = 0;
	image.texture = hizTexture;

	Vector3u localSize = material->getLocalSize();

	MipOption mipOption = { 0, 1 };

	context.bindImage(image, "dstDepth");
	context.bindTexture((ITexture*)depthTexture->getVendorTexture(), "srcDepth");
	context.dispatchCompute(width / localSize.x(), height / localSize.y(), 1);

	for (; mipOption.detailMip < miplevel - 1; mipOption.detailMip++) {
		width /= 2;
		height /= 2;
		image.level = mipOption.detailMip + 1;
		context.bindImage(image, "dstDepth");
		context.bindTexture((ITexture*)hizTexture->getVendorTexture(), "srcDepth", mipOption);
		context.dispatchCompute(width / localSize.x(), height / localSize.y(), 1);
	}
}

void HiZPass::reset()
{
	depthTexture = NULL;
	hizTexture = NULL;
}

void HiZPass::getOutputTextures(vector<pair<string, Texture*>>& textures)
{
	if (hizTexture)
		textures.push_back(make_pair("HiZMap", hizTexture));
}
