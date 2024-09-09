#include "SSAOPass.h"
#include "../Asset.h"
#include "../Console.h"
#include "../Camera.h"
#include "../GUI/UIControl.h"
#include "../RenderCore/RenderTask.h"
#include "../RenderCore/RenderThread.h"

SSAOPass::SSAOPass(const string & name, Material * material)
	: PostProcessPass(name, material)
{
	gtaoMap.setAutoGenMip(false);
	screenMap.setAutoGenMip(false);
	gtaoRenderTarget.addTexture("ssaoMap", gtaoMap);
	screenRenderTarget.addTexture("screenMap", screenMap);
}

void SSAOPass::prepare()
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
	Texture2D* sceneMap = dynamic_cast<Texture2D*>(resource->screenTexture);
	if (sceneMap)
		screenMap.setTextureInfo(sceneMap->getTextureInfo());
	gtaoRenderTarget.init();
	screenRenderTarget.init();
	Texture2D::whiteRGBADefaultTex.bind();
}

void SSAOPass::execute(IRenderContext& context)
{
	static const ShaderPropertyName depthMapName = "depthMap";
	static const ShaderPropertyName ssaoMapName = "ssaoMap";
	static const ShaderPropertyName screenMapName = "screenMap";

	Unit2Di ssaoSize = { (int)(size.x * screenScale), (int)(size.y * screenScale) };

	materialRenderData->upload();

	context.bindShaderProgram(materialVaraint->program);
	cameraRenderData->bind(context);

	context.bindMaterialBuffer(materialVaraint);

	context.bindTexture((ITexture*)resource->depthTexture->getVendorTexture(), depthMapName);

	// Pass 0 GTAO
	context.bindTexture((ITexture*)Texture2D::whiteRGBADefaultTex.getVendorTexture(), ssaoMapName);
	context.bindTexture((ITexture*)Texture2D::whiteRGBADefaultTex.getVendorTexture(), screenMapName);
	context.bindFrame(gtaoRenderTarget.getVendorRenderTarget());

	context.setDrawInfo(0, 2, 0);

	context.setViewport(0, 0, ssaoSize.x, ssaoSize.y);
	context.setRenderPostState();
	context.postProcessCall();

	context.clearFrameBindings();

	// Pass 1 Add
	context.bindTexture((ITexture*)gtaoMap.getVendorTexture(), ssaoMapName);
	context.bindTexture((ITexture*)resource->screenTexture->getVendorTexture(), screenMapName);

	context.bindFrame(screenRenderTarget.getVendorRenderTarget());

	context.setDrawInfo(1, 2, 0);

	context.setViewport(0, 0, size.x, size.y);
	context.setRenderPostState();
	context.postProcessCall();

	context.clearFrameBindings();

	resource->screenRenderTarget = &screenRenderTarget;
	resource->screenTexture = &screenMap;
}

bool SSAOPass::loadDefaultResource()
{
	if (material == NULL)
		material = getAssetByPath<Material>("Engine/Shaders/PostProcess/SSAOPassFS.mat");
	if (material == NULL || resource == NULL ||
		resource->screenTexture == NULL || resource->depthTexture == NULL)
		return false;
	materialRenderData = material->getMaterialRenderData();
	return materialRenderData;
}

void SSAOPass::render(RenderInfo & info)
{
	resource->ssaoTexture = NULL;
	if (!enable)
		return;
	if (size.x == 0 || size.y == 0)
		return;
	if (!loadDefaultResource())
		return;

	RENDER_THREAD_ENQUEUE_TASK(AddSSAOPass, ([this, materialRenderData = materialRenderData] (RenderThreadContext& context)
	{
		context.renderGraph->addPass(*this);
		if (materialRenderData)
			renderGraph->getRenderDataCollectorMainThread()->add(*materialRenderData);
	}));
}

void SSAOPass::resize(const Unit2Di& size)
{
	PostProcessPass::resize(size);
	gtaoRenderTarget.resize(size.x * screenScale, size.y * screenScale);
	screenRenderTarget.resize(size.x, size.y);
}

void SSAOPass::onGUI(EditorInfo& info)
{
	PostProcessPass::onGUI(info);
	float scale = screenScale;
	if (ImGui::InputFloat("ScreenScale", &scale)) {
		setScreenScale(scale);
	}
}

void SSAOPass::setScreenScale(float scale)
{
	if (screenScale != scale && scale > 0) {
		screenScale = scale;
		gtaoRenderTarget.resize(size.x * screenScale, size.y * screenScale);
	}
}

float SSAOPass::getScreenScale()
{
	return screenScale;
}
