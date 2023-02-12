#include "SceneRenderData.h"
#include "../Camera.h"
#include "../Render.h"
#include "../DirectLight.h"
#include "../PointLight.h"
#include "../ProbeSystem/ReflectionCaptureProbeRender.h"
#include "../ProbeSystem/EnvLightCaptureProbeRender.h"
#include "../Utility/RenderUtility.h"

SceneRenderData::SceneRenderData()
{
	staticMeshTransformDataPack.setFrequentUpdate(false);
}

void SceneRenderData::setCamera(Render* cameraRender)
{
	CameraRender* _cameraRender = dynamic_cast<CameraRender*>(cameraRender);
	if (_cameraRender) {
		CameraRenderData* cameraRenderData = _cameraRender->getRenderData();
		if (_cameraRender->isMainCameraRender())
			cameraRenderDatas.insert(cameraRenderDatas.begin(), cameraRenderData);
		else
			cameraRenderDatas.push_back(cameraRenderData);
		cameraRenderData->create();
	}
}

void SceneRenderData::setLight(Render* lightRender)
{
	if (lightRender->getRenderType() == IRendering::Light)
		lightDataPack.setLight((Light*)lightRender);
}

int SceneRenderData::setReflectionCapture(Render* captureRender)
{
	return reflectionProbeDataPack.setProbe(dynamic_cast<ReflectionCaptureProbeRender*>(captureRender));
}

int SceneRenderData::setEnvLightCapture(Render* captureRender)
{
	return envLightProbeDataPack.setLightCapture(dynamic_cast<EnvLightCaptureProbeRender*>(captureRender));
}

int SceneRenderData::setEnvLightData(Render* captureRender)
{
	return envLightDataPack.setLightData(dynamic_cast<EnvLightCaptureProbeRender*>(captureRender));
}

unsigned int SceneRenderData::setMeshTransform(const Matrix4f& transformMat)
{
	return meshTransformDataPack.setMeshTransform(transformMat);
}

unsigned int SceneRenderData::setMeshTransform(const vector<Matrix4f>& transformMats)
{
	return meshTransformDataPack.setMeshTransform(transformMats);
}

inline Guid makeGuid(void* ptr0, void* ptr1)
{
	Guid guid;
	guid.Word0 = (unsigned long long)ptr0;
	guid.Word1 = (unsigned long long)ptr1;
	return guid;
}

MeshTransformIndex* SceneRenderData::getMeshPartTransform(MeshPart* meshPart, Material* material)
{
	return meshTransformDataPack.getMeshPartTransform(meshPart, material);
}

MeshTransformIndex* SceneRenderData::setMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex, unsigned int transformCount)
{
	return meshTransformDataPack.setMeshPartTransform(meshPart, material, transformIndex, transformCount);
}

MeshTransformIndex* SceneRenderData::setMeshPartTransform(MeshPart* meshPart, Material* material, void* transformIndex)
{
	return meshTransformDataPack.setMeshPartTransform(meshPart, material, (MeshTransformIndex*)transformIndex);
}

unsigned int SceneRenderData::setStaticMeshTransform(const Matrix4f& transformMat)
{
	return staticMeshTransformDataPack.setMeshTransform(transformMat);
}

unsigned int SceneRenderData::setStaticMeshTransform(const vector<Matrix4f>& transformMats)
{
	return staticMeshTransformDataPack.setMeshTransform(transformMats);
}

MeshTransformIndex* SceneRenderData::getStaticMeshPartTransform(MeshPart* meshPart, Material* material)
{
	return staticMeshTransformDataPack.getMeshPartTransform(meshPart, material);
}

MeshTransformIndex* SceneRenderData::setStaticMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex)
{
	return staticMeshTransformDataPack.setMeshPartTransform(meshPart, material, transformIndex);
}

MeshTransformIndex* SceneRenderData::setStaticMeshPartTransform(MeshPart* meshPart, Material* material, void* transformIndex)
{
	return staticMeshTransformDataPack.setMeshPartTransform(meshPart, material, (MeshTransformIndex*)transformIndex);
}

void SceneRenderData::cleanStaticMeshTransform(unsigned int base, unsigned int count)
{
	staticMeshTransformDataPack.cleanTransform(base, count);
}

void SceneRenderData::cleanStaticMeshPartTransform(MeshPart* meshPart, Material* material)
{
	staticMeshTransformDataPack.cleanPart(meshPart, material);
}

void SceneRenderData::setUpdateStatic()
{
	staticMeshTransformDataPack.setDelayUpdate();
}

bool SceneRenderData::willUpdateStatic()
{
	return staticMeshTransformDataPack.getNeedUpdate();
}

bool SceneRenderData::frustumCulling(const Range<Vector3f>& bound, const Matrix4f& mat) const
{
	for (CameraRenderData* data : cameraRenderDatas) {
		if (::frustumCulling(data->data, bound, mat))
			return true;
	}
	return false;
}

void SceneRenderData::create()
{
	meshTransformDataPack.create();
	staticMeshTransformDataPack.create();
	particleDataPack.create();
	lightDataPack.create();
	reflectionProbeDataPack.create();
	envLightProbeDataPack.create();
	envLightDataPack.create();
}

void SceneRenderData::reset()
{
	meshTransformDataPack.clean();
	staticMeshTransformDataPack.clean();
	particleDataPack.clean();
	lightDataPack.clean();
	reflectionProbeDataPack.clean();
	envLightProbeDataPack.clean();
	envLightDataPack.clean();
	cameraRenderDatas.clear();
}

void SceneRenderData::release()
{
	meshTransformDataPack.release();
	staticMeshTransformDataPack.release();
	particleDataPack.release();
	lightDataPack.release();
	reflectionProbeDataPack.release();
	envLightProbeDataPack.release();
	envLightDataPack.release();
}

void SceneRenderData::upload()
{
	meshTransformDataPack.upload();
	staticMeshTransformDataPack.upload();
	particleDataPack.upload();
	lightDataPack.upload();
	reflectionProbeDataPack.upload();
	envLightDataPack.upload();
	for (auto& cameraRenderData : cameraRenderDatas) {
		if (cameraRenderData->usedFrame < (long long)Time::frames()) {
			cameraRenderData->upload();
			cameraRenderData->usedFrame = Time::frames();
		}
	}
}

void SceneRenderData::bind(IRenderContext& context)
{
	particleDataPack.bind(context);
	lightDataPack.bind(context);
	reflectionProbeDataPack.bind(context);
	envLightDataPack.bind(context);
}