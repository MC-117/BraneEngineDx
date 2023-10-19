#include "SceneRenderData.h"
#include "../Camera.h"
#include "../Render.h"
#include "../DirectLight.h"
#include "../PointLight.h"
#include "../ProbeSystem/ReflectionCaptureProbeRender.h"
#include "../ProbeSystem/EnvLightCaptureProbeRender.h"
#include "../Utility/RenderUtility.h"

SceneRenderData::SceneRenderData()
	: lightDataPack(probePoolPack)
	, reflectionProbeDataPack(probePoolPack)
	, envLightDataPack(probePoolPack)
	, virtualShadowMapRenderData(lightDataPack)
{
	staticMeshTransformRenderData.setFrequentUpdate(false);
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
		cameraRenderData->probeGrid.probePool = &probePoolPack;
		cameraRenderData->create();
		cameraRenderData->cullingContext.setSourceBatchDrawCommandArray(&meshBatchDrawCommandArray);
		cameraRenderData->staticCullingContext.setSourceBatchDrawCommandArray(&staticMeshBatchDrawCommandArray);
	}
}

void SceneRenderData::setMainLight(Render* lightRender)
{
	if (lightRender->getRenderType() == IRendering::Light_Render)
		lightDataPack.setMainLight((Light*)lightRender);
}

int SceneRenderData::setLocalLight(Render* lightRender)
{
	if (lightRender->getRenderType() == IRendering::Light_Render)
		return lightDataPack.setLocalLight((Light*)lightRender);
	return -1;
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

unsigned int SceneRenderData::setMeshTransform(const MeshTransformData& data)
{
	return meshTransformRenderData.setMeshTransform(data);
}

unsigned int SceneRenderData::setMeshTransform(const vector<MeshTransformData>& datas)
{
	return meshTransformRenderData.setMeshTransform(datas);
}

inline Guid makeGuid(void* ptr0, void* ptr1)
{
	Guid guid;
	guid.Word0 = (unsigned long long)ptr0;
	guid.Word1 = (unsigned long long)ptr1;
	return guid;
}

MeshBatchDrawCall* SceneRenderData::getMeshPartTransform(const MeshBatchDrawKey& key)
{
	return meshBatchDrawCommandArray.getMeshBatchDrawCall(key);
}

MeshBatchDrawCall* SceneRenderData::setMeshPartTransform(const MeshBatchDrawKey& key, unsigned int transformIndex, unsigned int transformCount)
{
	return meshBatchDrawCommandArray.setMeshBatchDrawCall(key, transformIndex, transformCount);
}

unsigned int SceneRenderData::setStaticMeshTransform(const MeshTransformData& data)
{
	return staticMeshTransformRenderData.setMeshTransform(data);
}

unsigned int SceneRenderData::setStaticMeshTransform(const vector<MeshTransformData>& datas)
{
	return staticMeshTransformRenderData.setMeshTransform(datas);
}

MeshBatchDrawCall* SceneRenderData::getStaticMeshPartTransform(const MeshBatchDrawKey& key)
{
	return staticMeshBatchDrawCommandArray.getMeshBatchDrawCall(key);
}

MeshBatchDrawCall* SceneRenderData::setStaticMeshPartTransform(const MeshBatchDrawKey& key, unsigned int transformIndex, unsigned int transformCount)
{
	return staticMeshBatchDrawCommandArray.setMeshBatchDrawCall(key, transformIndex, transformCount);
}

void SceneRenderData::cleanStaticMeshTransform(unsigned int base, unsigned int count)
{
	staticMeshTransformRenderData.cleanTransform(base, count);
}

void SceneRenderData::cleanStaticMeshPartTransform(MeshPart* meshPart, Material* material)
{
	staticMeshBatchDrawCommandArray.cleanPart(meshPart, material);
}

void SceneRenderData::setUpdateStatic()
{
	staticMeshTransformRenderData.setDelayUpdate();
}

bool SceneRenderData::willUpdateStatic()
{
	return staticMeshTransformRenderData.getNeedUpdate();
}

MeshBatchDrawData SceneRenderData::getBatchDrawData(bool isStatic)
{
	MeshBatchDrawData batchDrawData;
	batchDrawData.transformData = isStatic ? &staticMeshTransformRenderData : &meshTransformRenderData;
	batchDrawData.batchDrawCommandArray = isStatic ? &staticMeshBatchDrawCommandArray : &meshBatchDrawCommandArray;
	return batchDrawData;
}

ViewCulledMeshBatchDrawData SceneRenderData::getViewCulledBatchDrawData(Render* cameraRender, bool isStatic)
{
	ViewCulledMeshBatchDrawData batchDrawData;
	CameraRender* _cameraRender = dynamic_cast<CameraRender*>(cameraRender);
	if (_cameraRender) {
		CameraRenderData* cameraRenderData = _cameraRender->getRenderData();
		batchDrawData.transformData = isStatic ? &staticMeshTransformRenderData : &meshTransformRenderData;
		batchDrawData.batchDrawCommandArray = isStatic
			? &cameraRenderData->staticCullingContext
			: &cameraRenderData->cullingContext;
	}
	return batchDrawData;
}

void SceneRenderData::executeViewCulling(IRenderContext& context)
{
	for (CameraRenderData* data : cameraRenderDatas) {
		data->cullingContext.executeCulling(context, *data, meshTransformRenderData);
		data->staticCullingContext.executeCulling(context, *data, staticMeshTransformRenderData);
	}
	//cullingContext.runCulling();
}

bool SceneRenderData::frustumCulling(const BoundBox& bound, const Matrix4f& mat) const
{
	for (CameraRenderData* data : cameraRenderDatas) {
		if (::frustumCulling(data->data, bound, mat))
			return true;
	}
	return false;
}

void SceneRenderData::create()
{
	meshTransformRenderData.create();
	staticMeshTransformRenderData.create();
	meshBatchDrawCommandArray.create();
	staticMeshBatchDrawCommandArray.create();

	particleDataPack.create();
	probePoolPack.create();
	lightDataPack.create();
	reflectionProbeDataPack.create();
	envLightProbeDataPack.create();
	envLightDataPack.create();
	virtualShadowMapRenderData.create();
	debugRenderData.create();
}

void SceneRenderData::reset()
{
	meshTransformRenderData.clean();
	staticMeshTransformRenderData.clean();
	meshBatchDrawCommandArray.clean();
	staticMeshBatchDrawCommandArray.clean();
	
	particleDataPack.clean();
	probePoolPack.clean();
	lightDataPack.clean();
	reflectionProbeDataPack.clean();
	envLightProbeDataPack.clean();
	envLightDataPack.clean();
	virtualShadowMapRenderData.clean();
	debugRenderData.clean();
	cameraRenderDatas.clear();
}

void SceneRenderData::release()
{
	meshTransformRenderData.release();
	staticMeshTransformRenderData.release();
	meshBatchDrawCommandArray.release();
	staticMeshBatchDrawCommandArray.release();
	
	particleDataPack.release();
	probePoolPack.release();
	lightDataPack.release();
	reflectionProbeDataPack.release();
	envLightProbeDataPack.release();
	envLightDataPack.release();
	virtualShadowMapRenderData.release();
	debugRenderData.release();
}

void SceneRenderData::upload()
{
	meshTransformRenderData.upload();
	staticMeshTransformRenderData.upload();
	meshBatchDrawCommandArray.upload();
	staticMeshBatchDrawCommandArray.upload();
	
	particleDataPack.upload();
	probePoolPack.upload();
	lightDataPack.upload();
	reflectionProbeDataPack.upload();
	envLightDataPack.upload();
	for (auto& cameraRenderData : cameraRenderDatas) {
		if (cameraRenderData->usedFrame < (long long)Time::frames()) {
			cameraRenderData->upload();
			cameraRenderData->usedFrame = Time::frames();
		}
	}
	virtualShadowMapRenderData.upload();
	debugRenderData.upload();
}

void SceneRenderData::bind(IRenderContext& context)
{
	particleDataPack.bind(context);
	probePoolPack.bind(context);
	lightDataPack.bind(context);
	reflectionProbeDataPack.bind(context);
	envLightDataPack.bind(context);
	debugRenderData.bind(context);
}