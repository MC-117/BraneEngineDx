#include "SceneCaptureCube.h"
#include "RenderCore/RenderCore.h"

SceneCaptureCube::SceneCaptureCube()
	: SceneCapture(),
	frontCameraRender(frontRenderTarget),
	backCameraRender(backRenderTarget),
	leftCameraRender(leftRenderTarget),
	rightCameraRender(rightRenderTarget),
	topCameraRender(topRenderTarget),
	bottomCameraRender(bottomRenderTarget)
{
}

SceneCaptureCube::SceneCaptureCube(TextureCube& texture)
	: SceneCapture(),
	frontCameraRender(frontRenderTarget),
	backCameraRender(backRenderTarget),
	leftCameraRender(leftRenderTarget),
	rightCameraRender(rightRenderTarget),
	topCameraRender(topRenderTarget),
	bottomCameraRender(bottomRenderTarget)
{
	setTexture(texture);
}

void SceneCaptureCube::setTexture(TextureCube& texture)
{
	if (this->texture == &texture)
		return;
	this->texture = &texture;
	for (int i = 0; i < CF_Faces; i++) {
		renderTargets[i]->addTexture("screenMap", texture, 0, i);
	}
}

Matrix4f SceneCaptureCube::getProjectionMatrix()
{
	return Camera::perspective(90, 1, zNear, zFar);
}

Matrix4f SceneCaptureCube::getViewMatrix(CubeFace face)
{
	return Camera::lookAt(worldPosition, worldPosition + faceForwardVector[face], faceUpwardVector[face]);
}

Matrix4f SceneCaptureCube::getViewOriginMatrix(CubeFace face)
{
	return Camera::lookAt(Vector3f::Zero(), faceForwardVector[face], faceUpwardVector[face]);
}

void SceneCaptureCube::setSize(Unit2Di size)
{
	if (width != size.x) {
		width = size.x;
		for (int i = 0; i < CF_Faces; i++) {
			cameraRenders[i]->setSize({ (int)width, (int)width });
		}
	}
}

void SceneCaptureCube::render(RenderInfo& info)
{
	Matrix4f promat = getProjectionMatrix();
	Matrix4f promatInv = promat.inverse();
	for (int i = 0; i < CF_Faces; i++) {
		CameraRender* cameraRender = cameraRenders[i];
		cameraRender->cameraData.projectionMat = promat;
		cameraRender->cameraData.projectionMatInv = promatInv;
		Matrix4f vmat = getViewMatrix((CubeFace)i);
		Matrix4f vmatInv = vmat.inverse();
		cameraRender->cameraData.viewMat = vmat;
		cameraRender->cameraData.viewMatInv = vmatInv;
		Matrix4f vomat = getViewOriginMatrix((CubeFace)i);
		Matrix4f vomatInv = vomat.inverse();
		cameraRender->cameraData.viewOriginMat = vomat;
		cameraRender->cameraData.viewOriginMatInv = vomatInv;
		Matrix4f pmat = promat * vmat;
		cameraRender->cameraData.projectionViewMat = pmat;
		cameraRender->cameraData.cameraLoc = worldPosition;
		cameraRender->cameraData.cameraDir = faceForwardVector[i];
		cameraRender->cameraData.cameraUp = faceUpwardVector[i];
		cameraRender->cameraData.cameraLeft = faceLeftwardVector[i];
		cameraRender->cameraData.viewSize = Vector2f(width, width);
		cameraRender->cameraData.zNear = zNear;
		cameraRender->cameraData.zFar = zFar;
		cameraRender->cameraData.fovy = 90;
		cameraRender->cameraData.aspect = 1;
		cameraRender->clearColor = clearColor;
		CameraRenderData* cameraRenderData = cameraRender->getRenderData();
		if (cameraRenderData->surfaceBuffer == NULL) {
			cameraRenderData->surfaceBuffer = info.renderGraph->newSurfaceBuffer();
			cameraRenderData->surfaceBuffer->resize(cameraRender->size.x, cameraRender->size.y);
		}
		info.sceneData->setCamera(cameraRender);
	}
}
