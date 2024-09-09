#include "CameraRender.h"

#include "Camera.h"
#include "GUI/GUISurface.h"
#include "RenderCore/RenderCore.h"
#include "RenderCore/RenderCoreUtility.h"

int CameraRender::cameraRenderNextID = 1;

CameraRender::CameraRender()
	: flags(CameraRender_GizmoDraw)
{
	cameraRenderID = cameraRenderNextID;
	cameraRenderNextID++;
	createInternalRenderTarget();
}

CameraRender::CameraRender(RenderTarget& renderTarget)
	: renderTarget(&renderTarget)
	, flags(CameraRender_GizmoDraw)
{
}

CameraRender::~CameraRender()
{
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

void CameraRender::setCameraTag(const Name& tag)
{
	this->tag = tag;
}

bool CameraRender::isMainCameraRender() const
{
	return tag == CameraTag::main;
}

void CameraRender::setCameraRenderFlags(Enum<CameraRenderFlags> flags)
{
	this->flags = flags;
}

Enum<CameraRenderFlags> CameraRender::getCameraRenderFlags() const
{
	return flags;
}

void CameraRender::createDefaultPostProcessGraph()
{
	if (graph)
		return;
	graph = new PostProcessGraph();
	graph->addDefaultPasses();
	graph->resize(size);
	graph->resource.depthTexture = renderTarget->getInternalDepthTexture();
	graph->resource.screenTexture = renderTarget->getTexture(0);
	graph->resource.screenRenderTarget = renderTarget;
}

void CameraRender::triggerScreenHit(const Vector2u& hitPosition)
{
	createInternalHitData();
	hitData->triggerFrame = Time::frames();
	hitData->hitFrame = 0;
	hitData->hitInfo.hitDepth = 0;
	hitData->hitInfo.hitObjectIDHigh = 0;
	hitData->hitInfo.hitObjectIDLow = 0;
	hitData->hitInfo.hitNormal = Vector3f::Zero();
	hitData->hitInfo.hitPosition = hitPosition;
}

bool CameraRender::fetchScreenHit(ScreenHitInfo& hitInfo) const
{
	if (hitData == NULL || hitData->hitFrame != Time::frames())
		return false;
	hitInfo = hitData->hitInfo;
	return true;
}

Texture* CameraRender::getSceneTexture()
{
	return graph ? graph->resource.screenTexture : (renderTarget ? renderTarget->getTexture(0) : NULL);
}

RenderTarget& CameraRender::getRenderTarget()
{
	return *renderTarget;
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
	renderTarget->resize(size.x, size.y);
	if (graph) {
		graph->resize(size);
		graph->resource.depthTexture = renderTarget->getInternalDepthTexture();
		graph->resource.screenTexture = renderTarget->getTexture(0);
		graph->resource.screenRenderTarget = renderTarget;
	}
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

void CameraRender::preRender(PreRenderInfo& info)
{
}

void CameraRender::render(RenderInfo& info)
{
	if (isValid() && !hidden && isMainCameraRender()) {
		CameraRenderData* cameraRenderData = getRenderData();
		if (graph) {
			graph->resource.reset();
			graph->resource.cameraRenderData = cameraRenderData;
			graph->resource.depthTexture = renderTarget->getInternalDepthTexture();
			graph->resource.screenTexture = renderTarget->getTexture(0);
			graph->resource.screenRenderTarget = renderTarget;
			//graph->postRenderTarget.addTexture("screenMap", *graph->resource.screenTexture);
			graph->render(info);
		}
		RENDER_THREAD_ENQUEUE_TASK(CameraRenderUpdate, ([cameraRenderData] (RenderThreadContext& context)
		{
			cameraRenderData->updateSurfaceBuffer(context.renderGraph);
			updateRenderDataMainThread(cameraRenderData, Time::frames());
			context.sceneRenderData->setCamera(cameraRenderData);
		}));
	}
}

CameraRenderData* CameraRender::getRenderData()
{
	if (renderData)
		return renderData;
	renderData = new CameraRenderData();
	renderData->cameraRenderID = cameraRenderID;
	renderData->cameraRender = this;
	renderData->setDebugProbeIndex(debugProbeIndex);
	return renderData;
}

void CameraRender::setDebugProbeIndex(int probeIndex)
{
	debugProbeIndex = probeIndex;
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
