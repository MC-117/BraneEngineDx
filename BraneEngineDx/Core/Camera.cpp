#include "Camera.h"
#include "Geometry.h"
#include "GUI/Gizmo.h"
#include "Utility/MathUtility.h"

SerializeInstance(Camera);

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
	animationClip.setAnimationClipData(&data);
	animationClip.setupDefault();
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

Vector3f Camera::getFinalWorldPosition()
{
	Vector3f offset = getForward(WORLD) * -distance;
	return getPosition(WORLD) + offset;
}

bool Camera::culling(const Range<Vector3f>& bound, const Matrix4f& mat)
{
	/*float vLen = zFar * tan(fov * (0.5f * PI / 180.0f));
	float hLen = vLen * aspect;

	Vector3f pos, sca;
	Quaternionf rot;
	mat.decompose(pos, rot, sca);

	Matrix3f rotMat = rot.toRotationMatrix();
	Matrix4f _mat = Matrix4f::Identity();
	_mat.block(0, 0, 3, 3) = rotMat;
	_mat.block(0, 3, 3, 1) = pos;

	Matrix4f trans = getTransformMat();
	trans = trans * _mat.inverse();

	Vector3f worldPos = trans.block(0, 3, 3, 1);
	Vector3f upVec = (Matrix3f(trans.block(0, 0, 3, 3)) * Vector3f(0, 0, 1)).normalize();
	Vector3f rightVec = (Matrix3f(trans.block(0, 0, 3, 3)) * Vector3f(0, 1, 0)).normalize();
	Vector3f forVec = (Matrix3f(trans.block(0, 0, 3, 3)) * Vector3f(1, 0, 0)).normalize();

	Vector3f vVec = upVec * vLen;
	Vector3f hVec = rightVec * hLen;

	Vector3f farPoint = worldPos + forVec * zFar;
	Vector3f nearPoint = worldPos + forVec * zNear;

	Vector3f corner[4] =
	{
		farPoint - vVec - hVec,
		farPoint - vVec + hVec,
		farPoint + vVec - hVec,
		farPoint + vVec + hVec,
	};

	Vector4f planes[8] =
	{
		Vector4f::Plane(corner[1], corner[0], worldPos),
		Vector4f::Plane(corner[2], corner[3], worldPos),
		Vector4f::Plane(corner[0], corner[2], worldPos),
		Vector4f::Plane(corner[3], corner[1], worldPos),
		Vector4f::Plane(-forVec, farPoint),
		Vector4f::Plane(forVec, nearPoint)
	};

	planes[6] = planes[0];
	planes[7] = planes[0];

	Vector3f extend = (bound.maxVal - bound.minVal).cwiseProduct(sca) * 0.5f;
	Vector3f center = (bound.maxVal + bound.minVal).cwiseProduct(sca) * 0.5f;

	return IntersectAABB8Plane(center, extend, planes);*/

	Matrix4f MVP = getProjectionMatrix() * getViewMatrix() * mat;

	Vector4f corners[8] = {
		{bound.minVal.x(), bound.minVal.y(), bound.minVal.z(), 1.0}, // x y z
		{bound.maxVal.x(), bound.minVal.y(), bound.minVal.z(), 1.0}, // X y z
		{bound.minVal.x(), bound.maxVal.y(), bound.minVal.z(), 1.0}, // x Y z
		{bound.maxVal.x(), bound.maxVal.y(), bound.minVal.z(), 1.0}, // X Y z

		{bound.minVal.x(), bound.minVal.y(), bound.maxVal.z(), 1.0}, // x y Z
		{bound.maxVal.x(), bound.minVal.y(), bound.maxVal.z(), 1.0}, // X y Z
		{bound.minVal.x(), bound.maxVal.y(), bound.maxVal.z(), 1.0}, // x Y Z
		{bound.maxVal.x(), bound.maxVal.y(), bound.maxVal.z(), 1.0}, // X Y Z
	};

	for (size_t corner_idx = 0; corner_idx < 8; corner_idx++) {
		// Transform vertex
		Vector4f corner = MVP * corners[corner_idx];
		// Check vertex against clip space bounds
		if (-corner.w() < corner.x() && corner.x() < corner.w() &&
			-corner.w() < corner.y() && corner.y() < corner.w() &&
			0.0f < corner.z() && corner.z() < corner.w())
			return false;
	}
	return true;
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
		AnimationPose pose = animationClip.getPose();
		TransformData& data = pose.transformData[0];
		setPosition(data.position);
		setRotation(data.rotation);
		fov = pose.morphTargetWeight[0];
		distance = pose.morphTargetWeight[1];
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

	/*float vLen = zFar * tan(fov * PI / 360.0f * 0.5f);
	float hLen = vLen * aspect;
	Vector3f worldPos = getPosition(WORLD);
	Vector3f upVec = getUpward(WORLD);
	Vector3f rightVec = getRightward(WORLD);
	Vector3f forVec = getForward(WORLD);

	Vector3f vVec = upVec * vLen;
	Vector3f hVec = rightVec * hLen;

	Vector3f farPoint = worldPos + forVec * zFar;
	Vector3f nearPoint = worldPos + forVec * zNear;

	Vector3f corner[4] =
	{
		farPoint - vVec - hVec,
		farPoint - vVec + hVec,
		farPoint + vVec - hVec,
		farPoint + vVec + hVec,
	};

	Gizmo::instance().drawLine(corner[0], worldPos, Color(0.0f, 1.0f, 0.0f));
	Gizmo::instance().drawLine(corner[1], worldPos, Color(0.0f, 1.0f, 0.0f));
	Gizmo::instance().drawLine(corner[2], worldPos, Color(0.0f, 1.0f, 0.0f));
	Gizmo::instance().drawLine(corner[3], worldPos, Color(0.0f, 1.0f, 0.0f));
	Gizmo::instance().drawLine(corner[0], corner[1], Color(0.0f, 1.0f, 0.0f));
	Gizmo::instance().drawLine(corner[0], corner[2], Color(0.0f, 1.0f, 0.0f));
	Gizmo::instance().drawLine(corner[3], corner[1], Color(0.0f, 1.0f, 0.0f));
	Gizmo::instance().drawLine(corner[3], corner[2], Color(0.0f, 1.0f, 0.0f));*/
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
	Matrix4f promat = getProjectionMatrix();
	Matrix4f promatInv = promat.inverse();
	data.projectionMat = MATRIX_UPLOAD_OP(promat);
	data.projectionMatInv = MATRIX_UPLOAD_OP(promatInv);
	Matrix4f vmat = getViewMatrix();
	Matrix4f vmatInv = vmat.inverse();
	data.viewMat = MATRIX_UPLOAD_OP(vmat);
	data.viewMatInv = MATRIX_UPLOAD_OP(vmatInv);
	data.cameraLoc = cameraRender.cameraLoc;
	data.cameraDir = cameraRender.cameraDir;
	data.cameraUp = cameraRender.cameraUp;
	data.cameraLeft = cameraRender.cameraLeft;
	data.viewSize = Vector2f(size.x, size.y);
	data.zNear = zNear;
	data.zFar = zFar;
	data.fovy = fov;
	data.aspect = aspect;
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

Matrix4f Camera::viewport(float x, float y, float width, float height, float zNear, float zFar)
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

Serializable* Camera::instantiate(const SerializationInfo& from)
{
	return new Camera(from.name);
}

bool Camera::deserialize(const SerializationInfo& from)
{
	if (!Transform::deserialize(from))
		return false;
	from.get("fov", fov);
	from.get("distance", distance);
	SColor color;
	from.get("clearColor", color);
	clearColor = color;
	return true;
}

bool Camera::serialize(SerializationInfo& to)
{
	if (!Transform::serialize(to))
		return false;
	to.set("fov", fov);
	to.set("distance", distance);
	to.set("clearColor", SColor(clearColor));
	return true;
}
