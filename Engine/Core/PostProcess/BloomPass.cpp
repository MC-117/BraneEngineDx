#include "BloomPass.h"
#include "../Camera.h"
#include "../Asset.h"
#include "../Console.h"
#include "../GUI/UIControl.h"
#include "../RenderCore/RenderCore.h"

BloomPass::BloomPass(const string & name, Material * material)
	: PostProcessPass(name, material)
{
	int newBloomLevel = 1 + floor(log2(max(size.x, size.y)));
	newBloomLevel = min(newBloomLevel, 5);
	resizeBloomLevel(newBloomLevel);
}

void BloomPass::prepare()
{
	materialVaraint = materialRenderData->getVariant(Shader_Postprocess);
	if (materialVaraint == NULL) {
		Console::error("PostProcessPass: Shader_Postprocess not found in shader '%s'", materialRenderData->getShaderName().c_str());
		throw runtime_error("ShaderVariant not found");
		return;
	}

	materialVaraint->init();
	
	for (int i = 0; i < bloomLevel; i++) {
		bloomRenderTargets[i]->init();
		screenRenderTargets[i]->init();
	}
}

void BloomPass::execute(IRenderContext& context)
{
	static const ShaderPropertyName sampleMapName = "sampleMap";
	static const ShaderPropertyName imageMapName = "imageMap";

	materialRenderData->upload();

	ShaderProgram* program = materialVaraint->program;
	
	context.bindShaderProgram(program);
	context.bindMaterialBuffer(materialVaraint);

	if (program->isComputable()) {
		Vector3u localSize = material->getLocalSize();

		context.setDrawInfo(0, 4, 0);
		context.bindTexture((ITexture*)resource->screenTexture->getVendorTexture(), sampleMapName);
		Image image;
		image.texture = &bloomMap;
		for (int i = 0; i < bloomLevel; i++) {
			image.level = i;
			context.bindImage(image, imageMapName);
			context.dispatchCompute(ceilf(size.x / pow(2, i) / (float)localSize.x()), ceilf(size.y / pow(2, i) / (float)localSize.y()), 1);
		}

		context.setDrawInfo(1, 4, 0);
		for (int i = 0; i < bloomLevel; i++) {
			image.level = i;
			context.bindImage(image, imageMapName);
			context.dispatchCompute(ceilf(size.x / pow(2, i) / (float)localSize.x()), ceilf(size.y / pow(2, i) / (float)localSize.y()), 1);
		}

		context.setDrawInfo(2, 4, 0);
		for (int i = 0; i < bloomLevel; i++) {
			image.level = i;
			context.bindImage(image, imageMapName);
			context.dispatchCompute(ceilf(size.x / pow(2, i) / (float)localSize.x()), ceilf(size.y / pow(2, i) / (float)localSize.y()), 1);
		}

		context.setDrawInfo(3, 4, 0);
		context.bindTexture((ITexture*)bloomMap.getVendorTexture(), sampleMapName);
		image.texture = resource->screenTexture;
		image.level = 0;
		context.dispatchCompute(ceilf(size.x / (float)localSize.x()), ceilf(size.y / (float)localSize.y()), 1);
	}
	else {
		context.clearFrameBindings();
		context.bindTexture((ITexture*)resource->screenTexture->getVendorTexture(), sampleMapName);

		context.setDrawInfo(0, 1, 0);
		context.setRenderPostState();
		for (int i = 0; i < bloomLevel; i++) {
			int scalar = pow(2, i);

			context.bindFrame(bloomRenderTargets[i]->getVendorRenderTarget());
			context.clearFrameColor({ 0, 0, 0, 255 });

			context.setViewport(0, 0, size.x / scalar, size.y / scalar);
			context.postProcessCall();
		}

		context.clearFrameBindings();
		context.bindTexture((ITexture*)bloomMap.getVendorTexture(), sampleMapName);

		for (int i = 0; i < bloomLevel; i++) {
			int scalar = pow(2, i);

			context.setDrawInfo(1, i, 0);

			context.bindFrame(screenRenderTargets[i]->getVendorRenderTarget());

			context.setViewport(0, 0, size.x / scalar, size.y / scalar);
			context.postProcessCall();
		}

		context.clearFrameBindings();
		context.bindTexture((ITexture*)screenMap.getVendorTexture(), sampleMapName);

		for (int i = 0; i < bloomLevel; i++) {
			int scalar = pow(2, i);

			context.setDrawInfo(2, i, 0);

			context.bindFrame(bloomRenderTargets[i]->getVendorRenderTarget());

			context.setViewport(0, 0, size.x / scalar, size.y / scalar);
			context.postProcessCall();
		}

		context.setDrawInfo(3, 1, 0);

		context.bindTexture(NULL, sampleMapName);
		context.bindFrame(resource->screenRenderTarget->getVendorRenderTarget());

		context.bindTexture((ITexture*)bloomMap.getVendorTexture(), sampleMapName);

		context.setViewport(0, 0, size.x, size.y);
		context.setRenderPostAddState();
		context.postProcessCall();

		context.clearFrameBindings();
	}
}

bool BloomPass::loadDefaultResource()
{
	if (material == NULL)
		material = getAssetByPath<Material>("Engine/Shaders/PostProcess/BloomPassFS.mat");
	if (material == NULL || resource == NULL || resource->screenTexture == NULL)
		return false;
	material->setCount("bloomLevel", bloomLevel);
	material->setScalar("width", size.x);
	material->setScalar("height", size.y);
	materialRenderData = material->getMaterialRenderData();
	return materialRenderData;
}

void BloomPass::render(RenderInfo& info)
{
	if (!enable)
		return;
	if (size.x == 0 || size.y == 0)
		return;
	if (!loadDefaultResource())
		return;

	RENDER_THREAD_ENQUEUE_TASK(AddBloomPass, ([this, materialRenderData = materialRenderData] (RenderThreadContext& context)
	{
		context.renderGraph->addPass(*this);
		if (materialRenderData)
			renderGraph->getRenderDataCollectorMainThread()->add(*materialRenderData);
	}));
}

void BloomPass::resize(const Unit2Di & size)
{
	PostProcessPass::resize(size);
	int newBloomLevel = 1 + floor(log2(max(size.x, size.y)));
	newBloomLevel = min(newBloomLevel, 5);
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
			delete bloomRenderTargets[i];
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
