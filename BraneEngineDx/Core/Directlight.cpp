#include "Directlight.h"
#include "Engine.h"

#define DEPTHSIZE 8192

DirectLight::DirectLight(const string& name, Color color, float intensity) : Light::Light(name, color, intensity, 0, Box()), shadowCamera({ 8132, 8132 }, name + "_ShadowCamera")
{
	shadowCamera.setMode(Camera::Orthotropic);
	shadowCamera.setSize({ DEPTHSIZE, DEPTHSIZE }); 
	shadowCamera.left = -DEPTHSIZE / 16;
	shadowCamera.right = DEPTHSIZE / 16;
	shadowCamera.bottom = -DEPTHSIZE / 16;
	shadowCamera.top = DEPTHSIZE / 16;
	shadowCamera.zNear = 10000;
	shadowCamera.zFar = -10000;
	events.registerFunc("shadowCamDiv", [](void* obj, float v) {
		ShadowCamera& shadowCamera = ((DirectLight*)obj)->shadowCamera;
		shadowCamera.left = -DEPTHSIZE / v;
		shadowCamera.right = DEPTHSIZE / v;
		shadowCamera.bottom = -DEPTHSIZE / v;
		shadowCamera.top = DEPTHSIZE / v;
	});
}

Matrix4f DirectLight::getLightSpaceMatrix() const
{
	return shadowCamera.shadowCameraRender.projectionViewMat;
}

RenderTarget * DirectLight::getShadowRenderTarget() const
{
	return (RenderTarget*)&(shadowCamera.shadowCameraRender.depthRenderTarget);
}

void DirectLight::afterTick()
{
	Transform::afterTick();
	World* world = Engine::getCurrentWorld();
	Vector3f dirPos = getPosition(WORLD);
	Quaternionf dirRot = getRotation(WORLD);
	Vector3f dirFW = getForward(WORLD);
	Vector3f dirRW = getRightward(WORLD);
	Vector3f dirUW = getUpward(WORLD);
	if (world != NULL) {
		::Camera& cam = world->getCurrentCamera();
		Vector3f camRW = cam.getRightward(WORLD);
		Vector3f camFW = cam.getForward(WORLD);
		Vector3f dirFW = getForward(WORLD);
		Vector3f projv = dirFW.cross(camRW.cross(dirFW));
		projv.normalize();
		Vector3f dirRW = getRightward(WORLD);
		Quaternionf quat = Quaternionf::FromTwoVectors(dirRW, projv);

		dirPos = cam.getPosition(WORLD);
		dirRot = quat * dirRot;
		dirRW = quat * dirRW;
		dirUW = quat * dirUW;

		Vector3f fw = Vector3f(camFW.x(), camFW.y(), 0).normalized();
		dirPos += fw * (shadowCamera.bottom / dirUW.dot(fw) * 0.5);
		Vector3f rw = Vector3f(camRW.x(), camRW.y(), 0).normalized();
		float cosd = dirRW.dot(rw);
		dirPos += rw * ((shadowCamera.left / cosd - shadowCamera.left * cosd) * 0.5);
	}
	shadowCamera.position = dirPos;
	shadowCamera.rotation = dirRot;
	shadowCamera.scale = scale;
	shadowCamera.forward = dirFW;
	shadowCamera.rightward = dirRW;
	shadowCamera.upward = dirUW;
	//shadowCamera.afterTick();
	shadowCamera.cameraRender.transformMat = Transform::transformMat;
	shadowCamera.cameraRender.cameraLoc = shadowCamera.position;
	shadowCamera.cameraRender.cameraDir = shadowCamera.forward;
	shadowCamera.cameraRender.cameraUp = shadowCamera.upward;
	shadowCamera.projectionViewMat = shadowCamera.getProjectionMatrix() * shadowCamera.getViewMatrix();
	shadowCamera.cameraRender.projectionViewMat = shadowCamera.projectionViewMat;
}

void DirectLight::preRender()
{
}

void DirectLight::render(RenderInfo & info)
{
	if (info.cmdList == NULL) {
		shadowCamera.shadowCameraRender.preRender();
		shadowCamera.shadowCameraRender.render(info);
	}
	else if (!((Render*)info.tempRender->getRender())->hidden && info.tempRender->getCanCastShadow()) {
		Mesh* mesh = dynamic_cast<Mesh*>(info.tempRender->getShape());
		if (mesh != NULL) {
			Material* mat = &shadowCamera.shadowCameraRender.material;
			unsigned instanceID = info.tempRender->getInstanceID();
			if (instanceID != -1)
				for (int i = 0; i < mesh->meshParts.size(); i++) {
					Material* pm = info.tempRender->getMaterial(i);
					if (pm == NULL || !pm->canCastShadow)
						continue;
					for (int j = 0; j < info.tempRender->getInstanceCount(); j++)
						RenderCommandList::setMeshPartTransform(&mesh->meshParts[i], mat, instanceID + j);
					info.cmdList->setRenderCommand({
					mat,
					&shadowCamera,
					&mesh->meshParts[i]
					});
				}
		}
	}
}
