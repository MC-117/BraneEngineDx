#include "Directlight.h"
#include "World.h"
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

float DirectLight::getShadowResolutionScale() const
{
	return shadowData.cameraData.viewSize.x() / shadowData.right;
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

Matrix4f DirectLight::getWorldToLightViewMatrix() const
{
	return worldToLightViewMatrix;
}

Matrix4f DirectLight::getViewOriginToLightViewMatrix() const
{
	return viewOriginToLightViewMatrix;
}

Matrix4f DirectLight::getViewToLightClipMatrix() const
{
	return viewToLightClipMatrix;
}

Matrix4f DirectLight::getWorldToLightClipMatrix() const
{
	return worldToLightClipMatrix;
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

	Vector3f lightPos;
	Vector3f lightFW = getForward(WORLD);
	Quaternionf lightRot = Quaternionf::FromTwoVectors(Vector3f::UnitX(), lightFW);
	Vector3f lightLW = lightRot * Vector3f::UnitY();
	Vector3f lightUW = lightRot * Vector3f::UnitZ();

	if (World* world = dynamic_cast<World*>(getRoot())) {
		Camera& cam = world->getCurrentCamera();
		lightPos = cam.getPosition(WORLD);
	}
	else {
		lightPos = getPosition(WORLD);
	}

	Matrix4f promat = Camera::orthotropic(shadowData.left, shadowData.right, shadowData.bottom,
		shadowData.top, shadowData.cameraData.zNear, shadowData.cameraData.zFar);
	Matrix4f vmat = Camera::lookAt(lightPos, lightPos + lightFW, lightUW);
	worldToLightViewMatrix = vmat;
	viewToLightClipMatrix = promat;
	worldToLightClipMatrix = promat * vmat;
	shadowData.cameraData.projectionViewMat = worldToLightClipMatrix;
	Matrix4f promatInv = promat.inverse();
	shadowData.cameraData.projectionMat = promat;
	shadowData.cameraData.projectionMatInv = promatInv;
	Matrix4f vmatInv = vmat.inverse();
	shadowData.cameraData.viewMat = vmat;
	shadowData.cameraData.viewMatInv = vmatInv;
	Matrix4f vomat = Camera::lookAt(Vector3f::Zero(), lightFW, lightUW);
	Matrix4f vomatInv = vomat.inverse();
	viewOriginToLightViewMatrix = vomat;
	shadowData.cameraData.viewOriginMat = vomat;
	shadowData.cameraData.viewOriginMatInv = vomatInv;
	shadowData.cameraData.cameraLoc = lightPos;
	shadowData.cameraData.cameraDir = lightFW;
	shadowData.cameraData.cameraUp = lightUW;
	shadowData.cameraData.cameraLeft = lightLW;
	shadowData.cameraData.fovy = 0;
	shadowData.cameraData.aspect = 1;
}

void DirectLight::preRender(PreRenderInfo& info)
{
}

void DirectLight::render(RenderInfo& info)
{
	bool isVSMEnable = VirtualShadowMapConfig::isEnable();
	RENDER_THREAD_ENQUEUE_TASK(DirectLightUpdate, ([this, isVSMEnable] (RenderThreadContext& context)
	{
		context.sceneRenderData->setMainLight(this);
		if (isVSMEnable) {
			if (virtualShadowMapClipmap)
				delete virtualShadowMapClipmap;
			virtualShadowMapClipmap = context.sceneRenderData->virtualShadowMapRenderData.newClipmap(context.cameraRenderData);
			context.sceneRenderData->lightDataPack.addVirtualShadowMapClipmap(*virtualShadowMapClipmap);
		}
	}));
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
