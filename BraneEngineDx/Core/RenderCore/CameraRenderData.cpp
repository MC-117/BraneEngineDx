#include "CameraRenderData.h"
#include "../CameraRender.h"

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

	renderOrder = cameraRender->renderOrder;
	renderTarget = &cameraRender->getRenderTarget();

	if (clearColors.size() != renderTarget->getTextureCount()) {
		clearColors.resize(renderTarget->getTextureCount(), Color());
	}
	if (clearColors.size() > 0)
		clearColors[0] = cameraRender->clearColor;

	renderTarget->resize(data.viewSize.x(), data.viewSize.y());
}

void CameraRenderData::release()
{
	buffer.resize(0);
}

void CameraRenderData::upload()
{
	buffer.uploadData(1, &data);
}

void CameraRenderData::bind(IRenderContext& context)
{
	context.bindBufferBase(buffer.getVendorGPUBuffer(), "CameraDataBuf"); // CAM_BIND_INDEX
}