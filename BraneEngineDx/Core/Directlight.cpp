#include "Directlight.h"
#include "Engine.h"
#include "GUI/Gizmo.h"
#include "RenderCore/RenderCore.h"

#define DEPTHSIZE 8192

SerializeInstance(DirectLight);

DirectLight::DirectLight(const string& name, Color color, float intensity)
	: Light::Light(name, color, intensity, 0, Box()),
	depthTex(DEPTHSIZE, DEPTHSIZE, 1, false,
		{ TW_Border, TW_Border, TF_Point, TF_Point, TIT_D32_F, { 255, 255, 255, 255 } }),
	depthRenderTarget(DEPTHSIZE, DEPTHSIZE, depthTex)
{
	shadowData.cameraData.viewSize = { DEPTHSIZE, DEPTHSIZE };
	scaleShadowResolution(16);
	shadowData.cameraData.zNear = 10000;
	shadowData.cameraData.zFar = -10000;
}

void DirectLight::resizeShadowMap(Unit2Di size)
{
	depthRenderTarget.resize(size.x, size.y);
	shadowData.cameraData.viewSize = { (float)size.x, (float)size.y };
}

void DirectLight::scaleShadowResolution(float scalar)
{
	shadowData.left = -shadowData.cameraData.viewSize.x() / scalar;
	shadowData.right = shadowData.cameraData.viewSize.x() / scalar;
	shadowData.bottom = -shadowData.cameraData.viewSize.y() / scalar;
	shadowData.top = shadowData.cameraData.viewSize.y() / scalar;
}

void DirectLight::setShadowBiasDepthScale(float scale)
{
	shadowData.shadowBiasDepthScale = scale;
}

void DirectLight::setShadowBiasNormalScale(float scale)
{
	shadowData.shadowBiasNormalScale = scale;
}

float DirectLight::getShadowBiasDepthScale() const
{
	return shadowData.shadowBiasDepthScale;
}

float DirectLight::getShadowBiasNormalScale() const
{
	return shadowData.shadowBiasNormalScale;
}

Matrix4f DirectLight::getLightSpaceMatrix() const
{
	return lightSpaceMatrix;
}

Vector4f DirectLight::getShadowBias() const
{
	float bias = 3 * shadowData.right / DEPTHSIZE;
	return Vector4f(shadowData.shadowBiasDepthScale * bias, shadowData.shadowBiasNormalScale * bias);
}

RenderTarget* DirectLight::getShadowRenderTarget() const
{
	return (RenderTarget*)&depthRenderTarget;
}

void DirectLight::afterTick()
{
	Transform::afterTick();
	World* world = Engine::getCurrentWorld();
	Vector3f dirPos = getPosition(WORLD);
	Quaternionf dirRot = getRotation(WORLD);
	Vector3f dirFW = getForward(WORLD);
	Vector3f dirLW = getLeftward(WORLD);
	Vector3f dirUW = getUpward(WORLD);

	if (world != NULL) {
		::Camera& cam = world->getCurrentCamera();
		Vector3f camLW = cam.getLeftward(WORLD);
		Vector3f camFW = cam.getForward(WORLD);
		Vector3f dirFW = getForward(WORLD);
		Vector3f projv = dirFW.cross(camLW.cross(dirFW));
		projv.normalize();
		Vector3f dirLW = getLeftward(WORLD);
		Quaternionf quat = Quaternionf::FromTwoVectors(dirLW, projv);

		dirPos = cam.getPosition(WORLD);
		dirRot = quat * dirRot;
		dirLW = quat * dirLW;
		dirUW = quat * dirUW;

		Vector3f fw = Vector3f(camFW.x(), camFW.y(), 0).normalized();
		dirPos += fw * (shadowData.bottom / dirUW.dot(fw) * 0.5);
		Vector3f rw = Vector3f(camLW.x(), camLW.y(), 0).normalized();
		float cosd = dirLW.dot(rw);
		dirPos += rw * ((shadowData.left / cosd - shadowData.left * cosd) * 0.5);
	}
	Matrix4f promat = Camera::orthotropic(shadowData.left, shadowData.right, shadowData.bottom,
		shadowData.top, shadowData.cameraData.zNear, shadowData.cameraData.zFar);
	Matrix4f vmat = Camera::lookAt(dirPos, dirPos + dirFW, dirUW);
	lightSpaceMatrix = promat * vmat;
	shadowData.cameraData.projectionViewMat = MATRIX_UPLOAD_OP(lightSpaceMatrix);
	Matrix4f promatInv = promat.inverse();
	shadowData.cameraData.projectionMat = MATRIX_UPLOAD_OP(promat);
	shadowData.cameraData.projectionMatInv = MATRIX_UPLOAD_OP(promatInv);
	Matrix4f vmatInv = vmat.inverse();
	shadowData.cameraData.viewMat = MATRIX_UPLOAD_OP(vmat);
	shadowData.cameraData.viewMatInv = MATRIX_UPLOAD_OP(vmatInv);
	Matrix4f vomat = Camera::lookAt(Vector3f::Zero(), dirFW, dirUW);
	Matrix4f vomatInv = vomat.inverse();
	shadowData.cameraData.viewOriginMat = MATRIX_UPLOAD_OP(vomat);
	shadowData.cameraData.viewOriginMatInv = MATRIX_UPLOAD_OP(vomatInv);
	shadowData.cameraData.cameraLoc = dirPos;
	shadowData.cameraData.cameraDir = dirFW;
	shadowData.cameraData.cameraUp = dirUW;
	shadowData.cameraData.cameraLeft = dirLW;
	shadowData.cameraData.fovy = 0;
	shadowData.cameraData.aspect = 1;
}

void DirectLight::preRender()
{
}

void DirectLight::render(RenderInfo& info)
{
	info.sceneData->setLight(this);
}

Serializable* DirectLight::instantiate(const SerializationInfo& from)
{
	return new DirectLight(from.name);
}

bool DirectLight::deserialize(const SerializationInfo& from)
{
	return Light::deserialize(from);
}

bool DirectLight::serialize(SerializationInfo& to)
{
	return Light::serialize(to);
}
