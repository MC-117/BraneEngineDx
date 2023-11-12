#include "CameraRenderData.h"
#include "../CameraRender.h"

size_t SurfaceData::Hasher::operator()(const SurfaceData& s) const
{
	return (size_t)s.renderTarget;
}

size_t SurfaceData::Hasher::operator()(const SurfaceData* s) const
{
	return (size_t)s->renderTarget;
}

void SurfaceData::bind(IRenderContext& context, Enum<ClearFlags> plusClearFlags, Enum<ClearFlags> minusClearFlags)
{
	context.bindFrame(renderTarget->getVendorRenderTarget());
	Enum<ClearFlags> flag = (clearFlags | plusClearFlags) & ~minusClearFlags;
	if (flag.has(Clear_Colors))
		context.clearFrameColors(clearColors);
	if (flag.has(Clear_Depth))
		context.clearFrameDepth(clearDepth);
	if (flag.has(Clear_Stencil))
		context.clearFrameStencil(clearStencil);
}

void CameraRenderData::setDebugProbeIndex(int probeIndex)
{
	probeGridInfo.debugProbeIndex = probeIndex;
}

int CameraRenderData::getDebugProbeIndex() const
{
	return probeGridInfo.debugProbeIndex;
}

void CameraRenderData::create()
{
	if (cameraRender == NULL) {
		return;
	}
	data = cameraRender->cameraData;

	hitData = cameraRender->getTriggeredScreenHitData();

	setDebugProbeIndex(cameraRender->debugProbeIndex);
	cameraRender->debugProbeIndex = -1;
	flags = cameraRender->getCameraRenderFlags();

	renderOrder = cameraRender->renderOrder;
	surface.clearFlags = Clear_All;
	surface.renderTarget = &cameraRender->getRenderTarget();

	if (surface.clearColors.size() != surface.renderTarget->getTextureCount()) {
		surface.clearColors.resize(surface.renderTarget->getTextureCount(), Color());
	}
	if (surface.clearColors.size() > 0)
		surface.clearColors[0] = cameraRender->clearColor;

	surface.renderTarget->resize(data.viewSize.x(), data.viewSize.y());
	if (hitData) {
		hitData->resize(data.viewSize.x(), data.viewSize.y());
		hitData->create();
	}

	if (surfaceBuffer) {
		surfaceBuffer->create(cameraRender);
	}

	cullingContext.init();
	staticCullingContext.init();
}

void CameraRenderData::release()
{
	buffer.resize(0);
}

void CameraRenderData::upload()
{
	CameraUploadData uploadData;
	uploadData.cameraData = data;
	uploadData.cameraData.projectionViewMat = MATRIX_UPLOAD_OP(uploadData.cameraData.projectionViewMat);
	uploadData.cameraData.projectionMat = MATRIX_UPLOAD_OP(uploadData.cameraData.projectionMat);
	uploadData.cameraData.projectionMatInv = MATRIX_UPLOAD_OP(uploadData.cameraData.projectionMatInv);
	uploadData.cameraData.viewMat = MATRIX_UPLOAD_OP(uploadData.cameraData.viewMat);
	uploadData.cameraData.viewMatInv = MATRIX_UPLOAD_OP(uploadData.cameraData.viewMatInv);
	uploadData.cameraData.viewOriginMat = MATRIX_UPLOAD_OP(uploadData.cameraData.viewOriginMat);
	uploadData.cameraData.viewOriginMatInv = MATRIX_UPLOAD_OP(uploadData.cameraData.viewOriginMatInv);

	if (probeGrid.probePool)
	{
		probeGridInfo.init(data.viewSize.x(), data.viewSize.y(), data.zNear, data.zFar, *probeGrid.probePool);
		uploadData.probeGridInfo = probeGridInfo;
	}

	buffer.uploadData(1, &uploadData);
	if (hitData)
		hitData->upload();
}

void CameraRenderData::bind(IRenderContext& context)
{
	bindCameraBuffOnly(context);
	probeGrid.bind(context);
}

void CameraRenderData::bindCameraBuffOnly(IRenderContext& context)
{
	static const ShaderPropertyName CameraDataBufName = "CameraDataBuf";
	context.bindBufferBase(buffer.getVendorGPUBuffer(), CameraDataBufName); // CAM_BIND_INDEX
}
