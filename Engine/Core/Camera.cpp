#include "Camera.h"
#include "Geometry.h"
#include "ObjectUltility.h"
#include "GUI/Gizmo.h"
#include "Utility/MathUtility.h"
#include "Utility/RenderUtility.h"
#include "RenderCore/RenderTask.h"

SerializeInstance(Camera);

Camera::Camera(string name) : Transform(name)
{
	cameraRender.setSize(size);
}

Camera::Camera(RenderTarget & renderTarget, string name) : Transform(name), cameraRender(renderTarget)
{
	cameraRender.setSize(size);
}

Camera::~Camera()
{
}

ICameraManager* Camera::getCameraManager() const
{
	return manager;
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
		return Math::perspective(fov, aspect, zNear, zFar);
	case Orthotropic:
		return Math::orthotropic(left, right, bottom, top, zNear, zFar);
	default:
		return Matrix4f::Identity();
	}
}

Matrix4f Camera::getProjectionMatrixReversedZ() const
{
	switch (mode)
	{
	case Perspective:
		return Math::perspectiveReversedZ(fov, aspect, zNear, zFar);
	case Orthotropic:
		return Math::orthotropicReversedZ(left, right, bottom, top, zNear, zFar);
	default:
		return Matrix4f::Identity();
	}
}

Matrix4f Camera::getViewMatrix() const
{
	return Math::lookAt(cameraRender.cameraData.cameraLoc,
		cameraRender.cameraData.cameraLoc + cameraRender.cameraData.cameraDir,
		cameraRender.cameraData.cameraUp);
}

Matrix4f Camera::getViewOriginMatrix() const
{
	return Math::lookAt(Vector3f::Zero(), cameraRender.cameraData.cameraDir, cameraRender.cameraData.cameraUp);
}

Vector3f Camera::getFinalWorldPosition()
{
	Vector3f offset = getForward(WORLD) * -distance;
	return getPosition(WORLD) + offset;
}

bool Camera::culling(const BoundBox& bound, const Matrix4f& mat)
{
	return frustumCulling(cameraRender.cameraData, bound, mat);
}

void Camera::onAttacted(Object& parent)
{
	Transform::onAttacted(parent);
	if (World* world = getRootWorld(parent)) {
		ICameraManager& newManager = world->getCameraManager();
		if (manager != &newManager)
			newManager.setCamera(this, Name::none);
	}
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
	cameraRender.cameraData.distance = distance;
	cameraRender.cameraData.cameraDir = getForward(WORLD);
	cameraRender.cameraData.cameraUp = getUpward(WORLD);
	cameraRender.cameraData.cameraLeft = getLeftward(WORLD);

	if (distance != 0) {
		Vector3f offset = cameraRender.cameraData.cameraDir * -distance;
		Matrix4f T = Matrix4f::Identity();
		T(0, 3) = position.x();
		T(1, 3) = position.y();
		T(2, 3) = position.z();
		cameraRender.cameraData.cameraLoc = getPosition(WORLD) + offset;
		cameraRender.transformMat = T * transformMat;
	}
	else {
		cameraRender.cameraData.cameraLoc = getPosition(WORLD);
		cameraRender.transformMat = transformMat;
	}
	cameraRender.cameraData.projectionMat = getProjectionMatrixReversedZ();
	cameraRender.cameraData.viewMat = getViewMatrix();
	cameraRender.cameraData.viewOriginMat = getViewOriginMatrix();
	projectionViewMat = cameraRender.cameraData.projectionMat * cameraRender.cameraData.viewMat;
	cameraRender.cameraData.projectionViewMat = projectionViewMat;
	cameraRender.cameraData.projectionMatInv = cameraRender.cameraData.projectionMat.inverse();
	cameraRender.cameraData.viewMatInv = cameraRender.cameraData.viewMat.inverse();
	cameraRender.cameraData.viewOriginMatInv = cameraRender.cameraData.viewOriginMat.inverse();
	cameraRender.cameraData.fovy = fov;
	cameraRender.cameraData.aspect = aspect;
	cameraRender.cameraData.zFar = zFar;
	cameraRender.cameraData.zNear = zNear;
	cameraRender.cameraData.viewSize = { (float)size.x, (float)size.y };
	cameraRender.clearColor = clearColor;
}

void Camera::prerender(SceneRenderData& sceneData)
{
	Transform::prerender(sceneData);
	Name tag = Name::none;
	if (manager) {
		tag = manager->getCameraTag(this);
	}
	cameraRender.setCameraTag(tag);
}

Render* Camera::getRender()
{
	return &cameraRender;
}

unsigned int Camera::getRenders(vector<Render*>& renders)
{
	renders.push_back(&cameraRender);
	return 1;
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
	cameraRender.hidden = !active;
}

bool Camera::isActive()
{
	return active;
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
	cameraRender.createDefaultPostProcessGraph();
	if (from.subfeilds.find("postProcessingGraph") != from.subfeilds.end()) {
		from.get("postProcessingGraph", *cameraRender.graph);
	}
	return true;
}

bool Camera::serialize(SerializationInfo& to)
{
	if (!Transform::serialize(to))
		return false;
	to.set("fov", fov);
	to.set("distance", distance);
	to.set("clearColor", SColor(clearColor));
	if (cameraRender.graph)
		to.set("postProcessingGraph", *cameraRender.graph);
	return true;
}
