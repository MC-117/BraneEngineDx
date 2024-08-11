#pragma once
#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "Transform.h"
#include "CameraRender.h"
#include "Animation/AnimationClip.h"
#include "GPUBuffer.h"
#include "RenderTarget.h"
#include "RenderCore/CameraData.h"

class Camera;
class GUISurface;

namespace CameraTag
{
	static const Name main = "main";
}

class ENGINE_API ICameraManager
{
public:
	virtual void setCamera(Camera* camera, const Name& tag) = 0;
	virtual Camera* getCamera(const Name& tag) const = 0;
	virtual Name getCameraTag(Camera* camera) const = 0;

	virtual void removeCamera(Camera* camera) = 0;

	virtual void setGUISurface(GUISurface* guiSurface) = 0;
	virtual GUISurface* getGUISurface() const = 0;
};

class ENGINE_API Camera : public Transform
{
	friend class CameraManager;
public:
	Serialize(Camera, Transform);
	enum CameraMode {
		Perspective, Orthotropic
	} mode = Perspective;

	Unit2Di size = { 1280, 720 };
	Color clearColor = { 0, 0, 0 };
	float fov = 90;
	float aspect = 1280.0f / 720.0f;
	float left = 0, right = 0, top = 0, bottom = 0;
	float zNear = 0.1, zFar = 1000000;
	float distance = 0;
	bool active = false;

	Matrix4f projectionViewMat;

	CameraRender cameraRender;

	AnimationClip animationClip;

	Camera(string name = "Camera");
	Camera(RenderTarget& renderTarget, string name = "Camera");
	virtual ~Camera();
	
	ICameraManager* getCameraManager() const;

	void setAnimationClip(AnimationClipData& data);

	Matrix4f getProjectionMatrix() const;
	Matrix4f getViewMatrix() const;
	Matrix4f getViewOriginMatrix() const;

	Vector3f getFinalWorldPosition();

	bool culling(const BoundBox& bound, const Matrix4f& mat);

	virtual void onAttacted(Object& parent);

	virtual void tick(float deltaTime);
	virtual void afterTick();
	virtual void prerender(SceneRenderData& sceneData);

	virtual Render* getRender();
	virtual unsigned int getRenders(vector<Render*>& renders);

	virtual void setSize(Unit2Di size);
	void setMode(CameraMode mode);
	void setActive(bool active);

	bool isActive();

	static Matrix4f perspective(float fovy, float aspect, float zNear, float zFar);
	static Matrix4f orthotropic(float left, float right, float bottom, float top, float zNear, float zFar);
	static Matrix4f lookAt(Vector3f const& eye, Vector3f const& center, Vector3f const& up);
	static Matrix4f viewport(float x, float y, float width, float height, float zNear, float zFar);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	ICameraManager* manager = nullptr;
};

#endif // !_CAMERA_H_
