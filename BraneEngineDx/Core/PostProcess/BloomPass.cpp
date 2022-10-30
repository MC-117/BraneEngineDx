#include "BloomPass.h"
#include "../Camera.h"
#include "../Asset.h"
#include "../Console.h"
#include "../GUI/UIControl.h"

BloomPass::BloomPass(const string & name, Material * material)
	: PostProcessPass(name, material)
{
	int newBloomLevel = 1 + floor(log2(max(size.x, size.y)));
	newBloomLevel = min(newBloomLevel, 9);
	resizeBloomLevel(newBloomLevel);
}

void BloomPass::prepare()
{
	for (int i = 0; i < bloomLevel; i++) {
		bloomRenderTargets[i]->init();
		screenRenderTargets[i]->init();
	}
	MaterialRenderData* materialRenderData = (MaterialRenderData*)this->materialRenderData;
	materialRenderData->program = program;
	materialRenderData->create();
}

void BloomPass::execute(IRenderContext& context)
{
	materialRenderData->upload();
	context.bindShaderProgram(program);
	context.bindMaterialBuffer(((MaterialRenderData*)materialRenderData)->vendorMaterial);

	if (program->isComputable()) {
		Vector3u localSize = material->getLocalSize();

		context.setDrawInfo(0, 4, 0);
		context.bindTexture((ITexture*)resource->screenTexture->getVendorTexture(), Fragment_Shader_Stage, sampleMapSlot);
		Image image;
		image.texture = &bloomMap;
		for (int i = 0; i < bloomLevel; i++) {
			image.level = i;
			context.bindImage(image, imageMapSlot);
			context.dispatchCompute(ceilf(size.x / pow(2, i) / (float)localSize.x()), ceilf(size.y / pow(2, i) / (float)localSize.y()), 1);
		}

		context.setDrawInfo(1, 4, 0);
		for (int i = 0; i < bloomLevel; i++) {
			image.level = i;
			context.bindImage(image, imageMapSlot);
			context.dispatchCompute(ceilf(size.x / pow(2, i) / (float)localSize.x()), ceilf(size.y / pow(2, i) / (float)localSize.y()), 1);
		}

		context.setDrawInfo(2, 4, 0);
		for (int i = 0; i < bloomLevel; i++) {
			image.level = i;
			context.bindImage(image, imageMapSlot);
			context.dispatchCompute(ceilf(size.x / pow(2, i) / (float)localSize.x()), ceilf(size.y / pow(2, i) / (float)localSize.y()), 1);
		}

		context.setDrawInfo(3, 4, 0);
		context.bindTexture((ITexture*)bloomMap.getVendorTexture(), Fragment_Shader_Stage, sampleMapSlot);
		image.texture = resource->screenTexture;
		image.level = 0;
		context.dispatchCompute(ceilf(size.x / (float)localSize.x()), ceilf(size.y / (float)localSize.y()), 1);
	}
	else {
		context.clearFrameBindings();
		context.bindTexture((ITexture*)resource->screenTexture->getVendorTexture(), Fragment_Shader_Stage, sampleMapSlot);
		context.bindTexture((ITexture*)Texture2D::blackRGBADefaultTex.getVendorTexture(), Fragment_Shader_Stage, imageMapSlot);

		context.setDrawInfo(0, 1, 0);
		for (int i = 0; i < bloomLevel; i++) {
			int scalar = pow(2, i);

			context.bindFrame(bloomRenderTargets[i]->getVendorRenderTarget());
			context.clearFrameColor({ 0, 0, 0, 255 });

			context.setViewport(0, 0, size.x / scalar, size.y / scalar);
			context.postProcessCall();
		}

		context.clearFrameBindings();
		context.bindTexture((ITexture*)bloomMap.getVendorTexture(), Fragment_Shader_Stage, sampleMapSlot);

		for (int i = 0; i < bloomLevel; i++) {
			int scalar = pow(2, i);

			context.setDrawInfo(1, i, 0);

			context.bindFrame(screenRenderTargets[i]->getVendorRenderTarget());

			context.setViewport(0, 0, size.x / scalar, size.y / scalar);
			context.postProcessCall();
		}

		context.clearFrameBindings();
		context.bindTexture((ITexture*)screenMap.getVendorTexture(), Fragment_Shader_Stage, sampleMapSlot);

		for (int i = 0; i < bloomLevel; i++) {
			int scalar = pow(2, i);

			context.setDrawInfo(2, i, 0);

			context.bindFrame(bloomRenderTargets[i]->getVendorRenderTarget());

			context.setViewport(0, 0, size.x / scalar, size.y / scalar);
			context.postProcessCall();
		}

		context.setDrawInfo(3, 1, 0);

		context.bindTexture(NULL, Fragment_Shader_Stage, sampleMapSlot);
		context.bindFrame(screenRenderTargets[0]->getVendorRenderTarget());

		context.bindTexture((ITexture*)bloomMap.getVendorTexture(), Fragment_Shader_Stage, sampleMapSlot);
		context.bindTexture((ITexture*)resource->screenTexture->getVendorTexture(), Fragment_Shader_Stage, imageMapSlot);

		context.setViewport(0, 0, size.x, size.y);
		context.postProcessCall();

		context.clearFrameBindings();

		resource->screenRenderTarget = screenRenderTargets[0];
		resource->screenTexture = &screenMap;
	}
}

bool BloomPass::mapMaterialParameter(RenderInfo & info)
{
	if (material == NULL)
		material = getAssetByPath<Material>("Engine/Shaders/PostProcess/BloomPassFS.mat");
	if (material == NULL || resource == NULL || resource->screenTexture == NULL)
		return false;
	material->setCount("bloomLevel", bloomLevel);
	material->setScalar("width", size.x);
	material->setScalar("height", size.y);
	materialRenderData = material->getRenderData();
	return materialRenderData;
}

void BloomPass::render(RenderInfo& info)
{
	if (!enable)
		return;
	if (!mapMaterialParameter(info))
		return;
	if (size.x == 0 || size.y == 0)
		return;
	program = material->getShader()->getProgram(Shader_Postprocess);
	if (program == NULL) {
		Console::error("PostProcessPass: Shader_Postprocess not found in shader '%s'", material->getShaderName());
		return;
	}

	program->init();

	sampleMapSlot = program->getAttributeOffset("sampleMap").offset;

	if (program->isComputable())
		imageMapSlot = program->getAttributeOffset("imageMap").offset;
	else
		imageMapSlot = program->getAttributeOffset("screenMap").offset;

	if (sampleMapSlot == -1 || imageMapSlot == -1)
		return;

	info.renderGraph->addPass(*this);
}

void BloomPass::resize(const Unit2Di & size)
{
	PostProcessPass::resize(size);
	int newBloomLevel = 1 + floor(log2(max(size.x, size.y)));
	newBloomLevel = min(newBloomLevel, 9);
	resizeBloomLevel(newBloomLevel);
}

void BloomPass::resizeBloomLevel(int levels)
{
	int diffLevel = levels - bloomRenderTargets.size();
	bloomLevel = levels;
	if (diffLevel > 0) {
		bloomRenderTargets.reserve(bloomLevel);
		for (int i = 0; i < diffLevel; i++) {
			RenderTarget* bloomRT = new RenderTarget(size.x, size.y, 4);
			RenderTarget* screenRT = new RenderTarget(size.x, size.y, 4);

			bloomRT->addTexture("sampleMap", bloomMap);
			screenRT->addTexture("screenMap", screenMap);

			bloomRenderTargets.push_back(bloomRT);
			screenRenderTargets.push_back(screenRT);
		}
	}
	else {
		for (int i = 0; i < -diffLevel; i++) {
			delete bloomRenderTargets.back();
		}
		bloomRenderTargets.resize(bloomLevel);
	}
	for (int level = 0; level < bloomLevel; level++) {
		RenderTarget*& bloomRT = bloomRenderTargets[level];
		RenderTarget*& screenRT = screenRenderTargets[level];
		bloomRT->setTextureMipLevel(0, level);
		bloomRT->resize(size.x, size.y);
		screenRT->setTextureMipLevel(0, level);
		screenRT->resize(size.x, size.y);
	}
}
