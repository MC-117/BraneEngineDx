#pragma once
#include "SceneCapture.h"
#include "TextureCube.h"

class ENGINE_API SceneCaptureCube : public SceneCapture
{
public:
	unsigned int width = 0;
	Color clearColor = { 0, 0, 0 };
	float zNear = 0.1, zFar = 1000000;
	Vector3f worldPosition;

	CameraRender* const cameraRenders[CF_Faces] =
	{
		&frontCameraRender,
		&backCameraRender,
		&leftCameraRender,
		&rightCameraRender,
		&topCameraRender,
		&bottomCameraRender
	};
	RenderTarget* const renderTargets[CF_Faces] =
	{
		&frontRenderTarget,
		&backRenderTarget,
		&leftRenderTarget,
		&rightRenderTarget,
		&topRenderTarget,
		&bottomRenderTarget
	};

	SceneCaptureCube();
	SceneCaptureCube(TextureCube& texture);

	void setTexture(TextureCube& texture);

	virtual void setSize(Unit2Di size);

	virtual void render(RenderInfo& info);
protected:
	TextureCube* texture = NULL;

	CameraRender frontCameraRender;
	CameraRender backCameraRender;
	CameraRender leftCameraRender;
	CameraRender rightCameraRender;
	CameraRender topCameraRender;
	CameraRender bottomCameraRender;
	RenderTarget frontRenderTarget;
	RenderTarget backRenderTarget;
	RenderTarget leftRenderTarget;
	RenderTarget rightRenderTarget;
	RenderTarget topRenderTarget;
	RenderTarget bottomRenderTarget;

	CameraRenderData* cameraRenderDatas[CF_Faces];

	const Matrix4f& getProjectionMatrix();
	const Matrix4f& getViewMatrix(CubeFace face);
	const Matrix4f& getViewOriginMatrix(CubeFace face);
};