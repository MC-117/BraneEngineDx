#include "VolumetricLightPass.h"
#include "../Asset.h"
#include "../Console.h"
#include "../Camera.h"
#include "../GUI/UIControl.h"
#include "../RenderCore/RenderCore.h"

VolumetricLightPass::VolumetricLightPass(const string& name, Material* material)
	: PostProcessPass(name, material)
{
	passAMap.setAutoGenMip(false);
	passBMap.setAutoGenMip(false);
	passARenderTarget.addTexture("temp1Map", passAMap);
	passBRenderTarget.addTexture("temp1Map", passBMap);
}

void VolumetricLightPass::prepare()
{
	materialVaraint = materialRenderData->getVariant(Shader_Postprocess);
	if (materialVaraint == NULL) {
		Console::error("PostProcessPass: Shader_Postprocess not found in shader '%s'", materialRenderData->getShaderName().c_str());
		throw runtime_error("ShaderVariant not found");
		return;
	}
	if (materialVaraint->isComputable()) {
		throw runtime_error("Shader type mismatch");
		return;
	}
	
	materialVaraint->init();
	cameraRenderData = resource->cameraRenderData;
	if (cameraRenderData == NULL) {
		throw runtime_error("cameraRenderData is invalid");
		return;
	}
	passARenderTarget.init();
	passBRenderTarget.init();
}

void VolumetricLightPass::execute(IRenderContext& context)
{
	if (directShadowMap == NULL)
		return;
	
	static const ShaderPropertyName temp1MapName = "temp1Map";
	static const ShaderPropertyName temp2MapName = "temp2Map";

	Unit2Di screenSize = { int(size.x * screenScale), int(size.y * screenScale) };

	materialRenderData->upload();

	context.bindShaderProgram(materialVaraint->program);

	context.bindMaterialBuffer(materialVaraint);
	cameraRenderData->bind(context);

	// Pass 0 VolumetricLight
	context.bindFrame(passARenderTarget.getVendorRenderTarget());

	context.bindTexture((ITexture*)resource->depthTexture->getVendorTexture(), temp1MapName);
	context.bindTexture((ITexture*)directShadowMap->getVendorTexture(), temp2MapName);

	context.setDrawInfo(0, 4, 0);

	context.setViewport(0, 0, screenSize.x, screenSize.y);
	context.setRenderPostState();
	context.postProcessCall();

	// Pass 1 BlurX
	context.bindFrame(passBRenderTarget.getVendorRenderTarget());

	context.bindTexture((ITexture*)passAMap.getVendorTexture(), temp1MapName);
	context.bindTexture((ITexture*)Texture2D::blackRGBADefaultTex.getVendorTexture(), temp2MapName);

	context.setDrawInfo(1, 4, 0);

	context.setViewport(0, 0, screenSize.x, screenSize.y);
	context.setRenderPostState();
	context.postProcessCall();

	// Pass 2 BlurY
	context.bindTexture(NULL, temp1MapName);
	context.bindFrame(passARenderTarget.getVendorRenderTarget());

	context.bindTexture((ITexture*)passBMap.getVendorTexture(), temp1MapName);
	context.bindTexture((ITexture*)Texture2D::blackRGBADefaultTex.getVendorTexture(), temp2MapName);

	context.setDrawInfo(2, 4, 0);

	context.setViewport(0, 0, screenSize.x, screenSize.y);
	context.setRenderPostState();
	context.postProcessCall();

	// Pass 3 Add
	context.bindFrame(resource->screenRenderTarget->getVendorRenderTarget());

	context.bindTexture((ITexture*)passAMap.getVendorTexture(), temp1MapName);
	context.bindTexture((ITexture*)Texture2D::blackRGBADefaultTex.getVendorTexture(), temp2MapName);

	context.setDrawInfo(3, 4, 0);

	context.setViewport(0, 0, size.x, size.y);
	context.setRenderPostAddState();
	context.postProcessCall();

	context.setRenderPostState();

	context.clearFrameBindings();
}

bool VolumetricLightPass::loadDefaultResource()
{
	if (material == NULL)
		material = getAssetByPath<Material>("Engine/Shaders/PostProcess/VolumetricLightFS.mat");
	if (material == NULL || resource == NULL || resource->screenRenderTarget == NULL ||
		resource->depthTexture == NULL)
		return false;
	materialRenderData = material->getMaterialRenderData();
	return materialRenderData;
}

void VolumetricLightPass::render(RenderInfo& info)
{
	if (!enable)
		return;
	if (size.x == 0 || size.y == 0)
		return;
	if (!loadDefaultResource())
		return;

	RENDER_THREAD_ENQUEUE_TASK(AddVolumetricLightPass, ([this, materialRenderData = materialRenderData] (RenderThreadContext& context)
	{
		context.renderGraph->addPass(*this);
		if (materialRenderData)
			renderGraph->getRenderDataCollectorMainThread()->add(*materialRenderData);
		if (context.sceneRenderData->lightDataPack.shadowTarget)
			directShadowMap = context.sceneRenderData->lightDataPack.shadowTarget->getDepthTexture();
	}));
}

void VolumetricLightPass::resize(const Unit2Di& size)
{
	PostProcessPass::resize(size);
	Unit2Di safeSize = size * screenScale;
	safeSize = { max(safeSize.x, 1), max(safeSize.y, 1) };
	passARenderTarget.resize(safeSize.x, safeSize.y);
	passBRenderTarget.resize(safeSize.x, safeSize.y);
}

void VolumetricLightPass::onGUI(EditorInfo& info)
{
	PostProcessPass::onGUI(info);
	float scale = screenScale;
	if (ImGui::InputFloat("ScreenScale", &scale)) {
		setScreenScale(scale);
	}
}

void VolumetricLightPass::setScreenScale(float scale)
{
	if (screenScale != scale && scale > 0) {
		screenScale = scale;
		passARenderTarget.resize(size.x * screenScale, size.y * screenScale);
		passBRenderTarget.resize(size.x * screenScale, size.y * screenScale);
	}
}

float VolumetricLightPass::getScreenScale()
{
	return screenScale;
}
