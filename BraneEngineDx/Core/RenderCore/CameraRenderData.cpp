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
	data.projectionViewMat = MATRIX_UPLOAD_OP(data.projectionViewMat);
	data.projectionMat = MATRIX_UPLOAD_OP(data.projectionMat);
	data.projectionMatInv = MATRIX_UPLOAD_OP(data.projectionMatInv);
	data.viewMat = MATRIX_UPLOAD_OP(data.viewMat);
	data.viewMatInv = MATRIX_UPLOAD_OP(data.viewMatInv);
	data.viewOriginMat = MATRIX_UPLOAD_OP(data.viewOriginMat);
	data.viewOriginMatInv = MATRIX_UPLOAD_OP(data.viewOriginMatInv);

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
}

void CameraRenderData::release()
{
	buffer.resize(0);
}

void CameraRenderData::upload()
{
	buffer.uploadData(1, &data);
	if (hitData)
		hitData->upload();
}

void CameraRenderData::bind(IRenderContext& context)
{
	static const ShaderPropertyName CameraDataBufName = "CameraDataBuf";
	context.bindBufferBase(buffer.getVendorGPUBuffer(), CameraDataBufName); // CAM_BIND_INDEX
}
