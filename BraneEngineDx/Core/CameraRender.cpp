#include "CameraRender.h"
#include "RenderCore/RenderCore.h"

int CameraRender::cameraRenderNextID = 1;
CameraRender* CameraRender::mainCameraRender = NULL;

CameraRender::CameraRender()
{
	cameraRenderID = cameraRenderNextID;
	cameraRenderNextID++;
	createInternalRenderTarget();
}

CameraRender::CameraRender(RenderTarget& renderTarget) : renderTarget(&renderTarget)
{
}

CameraRender::~CameraRender()
{
	if (mainCameraRender == this)
		mainCameraRender = NULL;
	if (graph) {
		delete graph;
		graph = NULL;
	}
	if (renderData) {
		renderData->release();
		delete renderData;
		renderData = NULL;
	}
	if (hitData) {
		delete hitData;
		hitData = NULL;
	}
	if (internalRenderTarget) {
		delete internalRenderTarget;
		internalRenderTarget = NULL;
	}
	if (internalTexture) {
		delete internalTexture;
		internalTexture = NULL;
	}
}

bool CameraRender::isMainCameraRender() const
{
	return mainCameraRender == this;
}

void CameraRender::setMainCameraRender()
{
	mainCameraRender = this;
}

void CameraRender::createDefaultPostProcessGraph()
{
	if (graph)
		return;
	graph = new PostProcessGraph();
	graph->addDefaultPasses();
	graph->resize(size);
}

void CameraRender::triggerScreenHit(const Vector2u& hitPosition)
{
	createInternalHitData();
	hitData->triggerFrame = Time::frames();
	hitData->hitFrame = 0;
	hitData->hitInfo.hitDepth = 0;
	hitData->hitInfo.hitInstanceID = -1;
	hitData->hitInfo.hitPosition = hitPosition;
}

bool CameraRender::fetchScreenHit(ScreenHitInfo& hitInfo) const
{
	if (hitData == NULL || hitData->hitFrame != Time::frames())
		return false;
	hitInfo = hitData->hitInfo;
	return true;
}

Texture* CameraRender::getSceneMap()
{
	return graph ? graph->resource.screenTexture : (renderTarget ? renderTarget->getTexture(0) : NULL);
}

RenderTarget& CameraRender::getRenderTarget()
{
	return renderTarget ? *renderTarget : RenderTarget::defaultRenderTarget;
}

Texture2D* CameraRender::getSceneBlurTex()
{
	return graph ? dynamic_cast<Texture2D*>(graph->resource.blurTexture) : NULL;
}

void CameraRender::setSize(Unit2Di size)
{
	if (size == this->size)
		return;
	IRenderContext& context = *VendorManager::getInstance().getVendor().getDefaultRenderContext();
	context.clearFrameBindings();
	if (!getRenderTarget().isDefault()) {
		renderTarget->resize(size.x, size.y);
	}
	if (graph)
		graph->resize(size);
	this->size = size;
}

bool CameraRender::isValid()
{
	return renderTarget;
}

IRendering::RenderType CameraRender::getRenderType() const
{
	return IRendering::RenderType::Camera_Render;
}

void CameraRender::preRender()
{
}

void CameraRender::render(RenderInfo& info)
{
	if (isValid() && !hidden && isMainCameraRender()) {
		CameraRenderData* cameraRenderData = getRenderData();
		if (graph) {
			graph->resource.reset();
			graph->resource.sceneRenderData = info.sceneData;
			graph->resource.cameraRenderData = cameraRenderData;
			graph->resource.depthTexture = renderTarget->getInternalDepthTexture();
			graph->resource.screenTexture = renderTarget->getTexture(0);
			graph->resource.screenRenderTarget = renderTarget;
			//graph->postRenderTarget.addTexture("screenMap", *graph->resource.screenTexture);
			graph->render(info);
		}
		if (cameraRenderData->surfaceBuffer == NULL) {
			cameraRenderData->surfaceBuffer = info.renderGraph->newSurfaceBuffer();
			cameraRenderData->surfaceBuffer->resize(size.x, size.y);
		}
		info.sceneData->setCamera(this);
	}
}

CameraRenderData* CameraRender::getRenderData()
{
	if (renderData)
		return renderData;
	CameraRenderData* cameraRenderData = new CameraRenderData();
	cameraRenderData->cameraRenderID = cameraRenderID;
	cameraRenderData->cameraRender = this;
	renderData = cameraRenderData;
	return renderData;
}

CameraRender* CameraRender::getMainCameraRender()
{
	return mainCameraRender;
}

void CameraRender::createInternalRenderTarget()
{
	internalTexture = new Texture2D(size.x, size.y, 4, false, { TW_Clamp, TW_Clamp, TF_Linear, TF_Linear, TIT_RGBA16_FF });
	internalRenderTarget = new RenderTarget(size.x, size.y, 4, true);
	internalRenderTarget->addTexture("screenMap", *internalTexture);
	renderTarget = internalRenderTarget;
}

void CameraRender::createInternalHitData()
{
	if (hitData)
		return;
	hitData = new ScreenHitData();
}

ScreenHitData* CameraRender::getTriggeredScreenHitData()
{
	return hitData && hitData->triggerFrame == Time::frames() ? hitData : NULL;
}
