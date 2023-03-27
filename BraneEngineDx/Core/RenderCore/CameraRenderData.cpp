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

void CameraRenderData::create()
{
	if (cameraRender == NULL)
		return;
	data = cameraRender->cameraData;

	hitData = cameraRender->getTriggeredScreenHitData();

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
}

void CameraRenderData::release()
{
	buffer.resize(0);
}

void CameraRenderData::upload()
{
	CameraData dataUpload = data;
	dataUpload.projectionViewMat = MATRIX_UPLOAD_OP(dataUpload.projectionViewMat);
	dataUpload.projectionMat = MATRIX_UPLOAD_OP(dataUpload.projectionMat);
	dataUpload.projectionMatInv = MATRIX_UPLOAD_OP(dataUpload.projectionMatInv);
	dataUpload.viewMat = MATRIX_UPLOAD_OP(dataUpload.viewMat);
	dataUpload.viewMatInv = MATRIX_UPLOAD_OP(dataUpload.viewMatInv);
	dataUpload.viewOriginMat = MATRIX_UPLOAD_OP(dataUpload.viewOriginMat);
	dataUpload.viewOriginMatInv = MATRIX_UPLOAD_OP(dataUpload.viewOriginMatInv);
	buffer.uploadData(1, &dataUpload);
	if (hitData)
		hitData->upload();
}

void CameraRenderData::bind(IRenderContext& context)
{
	static const ShaderPropertyName CameraDataBufName = "CameraDataBuf";
	context.bindBufferBase(buffer.getVendorGPUBuffer(), CameraDataBufName); // CAM_BIND_INDEX
}
