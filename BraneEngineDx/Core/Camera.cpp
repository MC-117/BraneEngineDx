#include "Camera.h"
#include "Geometry.h"

Camera::Camera(string name) : ::Transform::Transform(name), _cameraRender(new CameraRender()),cameraRender(*_cameraRender)
{
}

Camera::Camera(CameraRender & cameraRender, string name) : ::Transform::Transform(name), cameraRender(cameraRender)
{
}

Camera::Camera(RenderTarget & renderTarget, Material & material, string name) : ::Transform::Transform(name), _cameraRender(new CameraRender(renderTarget, material)), cameraRender(*_cameraRender)
{
}

Camera::~Camera()
{
	if (_cameraRender != NULL)
		delete _cameraRender;
}

void Camera::setAnimationClip(AnimationClipData & data)
{
	animationClip.setAnimationClipData(data);
	for (int i = 0; i < animationClip.animationPlayer.size(); i++)
		animationClip.animationPlayer[i].setTargetTransform(*this);
}

Matrix4f Camera::getProjectionMatrix() const
{
	switch (mode)
	{
	case Perspective:
		return Camera::perspective(fov, aspect, zNear, zFar);
	case Orthotropic:
		return Camera::orthotropic(left, right, bottom, top, zNear, zFar);
	default:
		return Matrix4f::Identity();
	}
}

Matrix4f Camera::getViewMatrix() const
{
	return Camera::lookAt(cameraRender.cameraLoc, cameraRender.cameraLoc + cameraRender.cameraDir, cameraRender.cameraUp);
}

void Camera::tick(float deltaTime)
{
	::Transform::tick(deltaTime);
	if (animationClip.update(deltaTime)) {
		map<unsigned int, float>* curveData = animationClip.getCurveCurrentValue();
		if (curveData != NULL && !curveData->empty()) {
			auto iter = curveData->find(0);
			if (iter != curveData->end())
				fov = iter->second;
			iter = curveData->find(1);
			if (iter != curveData->end())
				distance = iter->second;
		}
	}
}

void Camera::afterTick()
{
	::Transform::afterTick();
	cameraRender.cameraDir = getForward(WORLD);
	cameraRender.cameraUp = getUpward(WORLD);
	cameraRender.cameraLeft = getRightward(WORLD);

	if (distance != 0) {
		Vector3f offset = cameraRender.cameraDir * -distance;
		Matrix4f T = Matrix4f::Identity();
		T(0, 3) = position.x();
		T(1, 3) = position.y();
		T(2, 3) = position.z();
		cameraRender.cameraLoc = getPosition(WORLD) + offset;
		cameraRender.transformMat = T * transformMat;
	}
	else {
		cameraRender.cameraLoc = getPosition(WORLD);
		cameraRender.transformMat = transformMat;
	}
	projectionViewMat = getProjectionMatrix() * getViewMatrix();
	cameraRender.projectionViewMat = projectionViewMat;
}

void Camera::setSize(Unit2Di size)
{
	if (this->size != size) {
		this->size = size;
		cameraRender.setSize(size);
		aspect = size.x / (float)size.y;
	}
}

void Camera::setMode(CameraMode mode)
{
	this->mode = mode;
}

void Camera::setActive(bool active)
{
	this->active = active;
}

bool Camera::isActive()
{
	return active;
}

void Camera::uploadCameraData()
{
	CameraData data;
	data.projectionViewMat = cameraRender.projectionViewMat;
	data.projectionMat = getProjectionMatrix();
	data.viewMat = getViewMatrix();
	data.cameraLoc = cameraRender.cameraLoc;
	data.cameraDir = cameraRender.cameraDir;
	data.cameraUp = cameraRender.cameraUp;
	data.cameraLeft = cameraRender.cameraLeft;
	data.viewSize = Vector2f(size.x, size.y);
	data.zNear = zNear;
	data.zFar = zFar;
	data.fovy = fov;
	cameraDataBuffer.uploadData(1, &data);
}

void Camera::bindCameraData()
{
	cameraDataBuffer.bindBase(CAM_BIND_INDEX);
}

Matrix4f Camera::perspective(float fovy, float aspect, float zNear, float zFar)
{
	float tanHalfFovy = tan(fovy * PI / 360.0);

	Matrix4f Result = Matrix4f::Zero();
	Result(0, 0) = 1 / (aspect * tanHalfFovy);
	Result(1, 1) = 1 / (tanHalfFovy);
	Result(2, 2) = -(zFar + zNear) / (zFar - zNear);
	Result(3, 2) = -1.0;
	Result(2, 3) = -(2 * zFar * zNear) / (zFar - zNear);
	return Result;
}

Matrix4f Camera::orthotropic(float left, float right, float bottom, float top, float zNear, float zFar)
{
	Matrix4f Result = Matrix4f::Identity();
	Result(0, 0) = 2 / (right - left);
	Result(1, 1) = 2 / (top - bottom);
	Result(2, 2) = -2 / (zFar - zNear);
	Result(0, 3) = -(right + left) / (right - left);
	Result(1, 3) = -(top + bottom) / (top - bottom);
	Result(2, 3) = -(zFar + zNear) / (zFar - zNear);
	return Result;
}

Matrix4f Camera::lookAt(Vector3f const & eye, Vector3f const & center, Vector3f const & up)
{
	Vector3f W(center - eye);
	Vector3f U(W.cross(up));
	Vector3f V(U.cross(W));
	W.normalize();
	U.normalize();
	V.normalize();


	Matrix4f Result = Matrix4f::Identity();
	Result(0, 0) = U.x();
	Result(0, 1) = U.y();
	Result(0, 2) = U.z();
	Result(1, 0) = V.x();
	Result(1, 1) = V.y();
	Result(1, 2) = V.z();
	Result(2, 0) = -W.x();
	Result(2, 1) = -W.y();
	Result(2, 2) = -W.z();
	Result(0, 3) = -U.dot(eye);
	Result(1, 3) = -V.dot(eye);
	Result(2, 3) = W.dot(eye);
	return Result;
}

Matrix4f Camera::viewport(int x, int y, int width, int height, float zNear, float zFar)
{
	Matrix4f Result = Matrix4f::Zero();
	Result(0, 0) = width / 2.0;
	Result(0, 3) = x + width / 2.0;
	Result(1, 1) = height / 2.0;
	Result(1, 3) = y + height / 2.0;
	Result(2, 2) = (zFar - zNear) / 2.0;
	Result(2, 3) = (zFar + zNear) / 2.0;
	Result(3, 3) = 1;
	return Result;
}
