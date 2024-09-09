#include "SceneCaptureCube.h"
#include "RenderCore/RenderCore.h"
#include "RenderCore/RenderCoreUtility.h"
#include "Utility/RenderUtility.h"

SceneCaptureCube::SceneCaptureCube()
	: SceneCapture(),
	frontCameraRender(frontRenderTarget),
	backCameraRender(backRenderTarget),
	leftCameraRender(leftRenderTarget),
	rightCameraRender(rightRenderTarget),
	topCameraRender(topRenderTarget),
	bottomCameraRender(bottomRenderTarget)
{
	for (int i = 0; i < CF_Faces; i++) {
		CameraRender* cameraRender = cameraRenders[i];
		cameraRender->setCameraRenderFlags(CameraRender_SceneCapture);
	}
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
	for (int i = 0; i < CF_Faces; i++) {
		CameraRender* cameraRender = cameraRenders[i];
		cameraRender->setCameraRenderFlags(CameraRender_SceneCapture);
	}
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
	std::array<CameraRenderData*, CF_Faces> cameraRenderDatas;
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
		cameraRender->cameraData.cameraDir = getCubeFaceForwardVector((CubeFace)i);
		cameraRender->cameraData.cameraUp = getCubeFaceUpwardVector((CubeFace)i);
		cameraRender->cameraData.cameraLeft = getCubeFaceLeftwardVector((CubeFace)i);
		cameraRender->cameraData.viewSize = Vector2f(width, width);
		cameraRender->cameraData.zNear = zNear;
		cameraRender->cameraData.zFar = zFar;
		cameraRender->cameraData.fovy = 90;
		cameraRender->cameraData.aspect = 1;
		cameraRender->clearColor = clearColor;
		cameraRenderDatas[i] = cameraRender->getRenderData();
	}

	RENDER_THREAD_ENQUEUE_TASK(SceneCaptureCubeUpdateCameraRenderData, ([this, cameraRenderDatas, mainFrame = Time::frames()] (RenderThreadContext& context)
	{
		for (int i = 0; i < CF_Faces; i++) {
			CameraRenderData* cameraRenderData = cameraRenderDatas[i];
			cameraRenderData->updateSurfaceBuffer(context.renderGraph);
			updateRenderDataMainThread(cameraRenderData, Time::frames());
			context.sceneRenderData->setCamera(cameraRenderData);
		}
	}));
}

const Matrix4f& SceneCaptureCube::getProjectionMatrix()
{
	return getCubeFaceProjectionMatrix(zNear, zFar);
}

const Matrix4f& SceneCaptureCube::getViewMatrix(CubeFace face)
{
	return getCubeFaceViewMatrix(face, worldPosition);
}

const Matrix4f& SceneCaptureCube::getViewOriginMatrix(CubeFace face)
{
	return getCubeFaceViewMatrix(face, Vector3f::Zero());
}
