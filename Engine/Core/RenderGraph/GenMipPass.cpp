#include "GenMipPass.h"
#include "../RenderCore/SceneRenderData.h"
#include "../Asset.h"

bool GenMipPass::loadDefaultResource()
{
	if (material == NULL) {
		material = getAssetByPath<Material>("Engine/Shaders/Pipeline/GenMips.mat");
	}
	return material;
}

void GenMipPass::prepare()
{
	if (material) {
		program = material->getShader()->getProgram(Shader_Default);
		program->init();
	}
	outputTextures.clear();
}

void GenMipPass::execute(IRenderContext& context)
{
	if (renderGraph == NULL || program == NULL)
		return;
	for (auto sceneData : renderGraph->sceneDatas) {
		for (auto cameraData : sceneData->cameraRenderDatas) {
			ISceneColorMipsGetter* getter = dynamic_cast<ISceneColorMipsGetter*>(cameraData->surfaceBuffer);
			if (getter == NULL)
				continue;
			Texture* sceneColorTexture = NULL;
			if (cameraData->surface.renderTarget)
				sceneColorTexture = cameraData->surface.renderTarget->getTexture(0);
			Texture* sceneColorMips = getter->getSceneColorMips();
			if (sceneColorTexture == NULL || sceneColorMips == NULL)
				continue;
			int miplevel = sceneColorMips->getMipLevels();
			int width = sceneColorTexture->getWidth();
			int height = sceneColorTexture->getHeight();
			context.bindShaderProgram(program);

			Vector3u localSize = material->getLocalSize();

			static const ShaderPropertyName srcColorName = "srcColor";
			static const ShaderPropertyName dstColorName = "dstColor";

			Image image;
			image.level = 0;
			image.texture = sceneColorMips;

			MipOption mipOption;
			mipOption.detailMip = 0;
			mipOption.mipCount = 1;

			context.bindImage(image, dstColorName);
			context.bindTexture((ITexture*)sceneColorTexture->getVendorTexture(), srcColorName, mipOption);
			context.dispatchCompute(ceil(width / (float)localSize.x()), ceil(height / (float)localSize.x()), 1);

			for (; mipOption.detailMip < miplevel - 1; mipOption.detailMip++) {
				width /= 2;
				height /= 2;
				image.level = mipOption.detailMip + 1;
				context.bindImage(image, dstColorName);
				context.bindTexture((ITexture*)sceneColorMips->getVendorTexture(), srcColorName, mipOption);
				context.dispatchCompute(ceil(width / (float)localSize.x()), ceil(height / (float)localSize.x()), 1);
			}

			context.unbindBufferBase(srcColorName);
			context.unbindBufferBase(dstColorName);
			outputTextures.push_back(make_pair("SceneColorMips", sceneColorTexture));
		}
	}
}

void GenMipPass::reset()
{
}

void GenMipPass::getOutputTextures(vector<pair<string, Texture*>>& textures)
{
	textures = outputTextures;
}
