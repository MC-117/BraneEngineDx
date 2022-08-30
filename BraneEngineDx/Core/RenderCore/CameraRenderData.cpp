#include "CameraRenderData.h"
#include "../Camera.h"

void CameraRenderData::create()
{
	if (camera == NULL)
		return;
	data.projectionViewMat = MATRIX_UPLOAD_OP(camera->cameraRender.projectionViewMat);
	Matrix4f promat = camera->getProjectionMatrix();
	Matrix4f promatInv = promat.inverse();
	data.projectionMat = MATRIX_UPLOAD_OP(promat);
	data.projectionMatInv = MATRIX_UPLOAD_OP(promatInv);
	Matrix4f vmat = camera->getViewMatrix();
	Matrix4f vmatInv = vmat.inverse();
	data.viewMat = MATRIX_UPLOAD_OP(vmat);
	data.viewMatInv = MATRIX_UPLOAD_OP(vmatInv);
	data.cameraLoc = camera->cameraRender.cameraLoc;
	data.cameraDir = camera->cameraRender.cameraDir;
	data.cameraUp = camera->cameraRender.cameraUp;
	data.cameraLeft = camera->cameraRender.cameraLeft;
	data.viewSize = Vector2f(camera->size.x, camera->size.y);
	data.zNear = camera->zNear;
	data.zFar = camera->zFar;
	data.fovy = camera->fov;
	data.aspect = camera->aspect;

	renderOrder = camera->cameraRender.renderOrder;
	renderTarget = &camera->cameraRender.renderTarget;

	renderTarget->init();
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
	context.bindBufferBase(buffer.getVendorGPUBuffer(), CAM_BIND_INDEX);
}