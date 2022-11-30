#pragma once
#include "SceneCapture.h"
#include "TextureCube.h"

class SceneCaptureCube : public SceneCapture
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

	Matrix4f getProjectionMatrix();
	Matrix4f getViewMatrix(CubeFace face);
	Matrix4f getViewOriginMatrix(CubeFace face);

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

	const Vector3f faceForwardVector[CF_Faces] = {
		{ 1, 0, 0 },
		{ -1, 0, 0 },
		{ 0, -1, 0 },
		{ 0, 1, 0 },
		{ 0, 0, 1 },
		{ 0, 0, -1 },
	};

	const Vector3f faceLeftwardVector[CF_Faces] = {
		{ 0, 1, 0 },
		{ 0, -1, 0 },
		{ 1, 0, 0 },
		{ -1, 0, 0 },
		{ 0, 1, 0 },
		{ 0, -1, 0 },
	};

	const Vector3f faceUpwardVector[CF_Faces] = {
		{ 0, -1, 0 },
		{ 0, -1, 0 },
		{ 0, 0, -1 },
		{ 0, 0, 1 },
		{ 0, -1, 0 },
		{ 0, -1, 0 },
	};

	CameraRenderData* cameraRenderDatas[CF_Faces];
};