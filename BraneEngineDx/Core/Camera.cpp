#include "Camera.h"
#include "Geometry.h"

Camera::Camera(string name) : Transform(name), _cameraRender(new CameraRender()),cameraRender(*_cameraRender)
{
}

Camera::Camera(CameraRender & cameraRender, string name) : Transform(name), cameraRender(cameraRender)
{
}

Camera::Camera(RenderTarget & renderTarget, Material & material, string name) : Transform(name), _cameraRender(new CameraRender(renderTarget, material)), cameraRender(*_cameraRender)
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

Color hsv2rgb(float h, float s, float v, float a = 1)
{
	float      hh, p, q, t, ff;
	long        i;
	Color         out;
	out.a = a;
	if (s <= 0.0) {       // < is bogus, just shuts up warnings
		out.r = v;
		out.g = v;
		out.b = v;
		return out;
	}
	hh = h;
	if (hh >= 360.0) hh = 0.0;
	hh /= 60.0;
	i = (long)hh;
	ff = hh - i;
	p = v * (1.0 - s);
	q = v * (1.0 - (s * ff));
	t = v * (1.0 - (s * (1.0 - ff)));

	switch (i) {
	case 0:
		out.r = v;
		out.g = t;
		out.b = p;
		break;
	case 1:
		out.r = q;
		out.g = v;
		out.b = p;
		break;
	case 2:
		out.r = p;
		out.g = v;
		out.b = t;
		break;

	case 3:
		out.r = p;
		out.g = q;
		out.b = v;
		break;
	case 4:
		out.r = t;
		out.g = p;
		out.b = v;
		break;
	case 5:
	default:
		out.r = v;
		out.g = p;
		out.b = q;
		break;
	}
	return out;
}

void Camera::tick(float deltaTime)
{
	Transform::tick(deltaTime);
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
	Transform::afterTick();
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
	this->size = size;
	cameraRender.setSize(size);
	aspect = size.x / (float)size.y;
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
	data.projectionViewMat = MATRIX_UPLOAD_OP(cameraRender.projectionViewMat);
	data.projectionMat = MATRIX_UPLOAD_OP(getProjectionMatrix());
	data.viewMat = MATRIX_UPLOAD_OP(getViewMatrix());
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
	DirectX::XMFLOAT4X4 xmf44;
	DirectX::XMStoreFloat4x4(&xmf44, DirectX::XMMatrixPerspectiveFovRH(fovy * PI / 180.0, aspect, zNear, zFar));
	return xmf44;
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
	DirectX::XMFLOAT4X4 xmf44;
	DirectX::XMStoreFloat4x4(&xmf44, DirectX::XMMatrixOrthographicOffCenterRH(left, right, bottom, top, zNear, zFar));
	return xmf44;
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
	DirectX::XMFLOAT4X4 xmf44;
	DirectX::XMStoreFloat4x4(&xmf44, DirectX::XMMatrixLookAtRH(DirectX::XMLoadFloat3((DirectX::XMFLOAT3*)&eye),
		DirectX::XMLoadFloat3((DirectX::XMFLOAT3*)&center), DirectX::XMLoadFloat3((DirectX::XMFLOAT3*)&up)));
	return xmf44;
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
