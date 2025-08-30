#include "SceneRenderData.h"

#include "RenderCoreUtility.h"
#include "RenderThread.h"
#include "../Camera.h"
#include "../Render.h"
#include "../DirectLight.h"
#include "../PointLight.h"
#include "../ProbeSystem/ReflectionCaptureProbeRender.h"
#include "../ProbeSystem/EnvLightCaptureProbeRender.h"
#include "../Utility/RenderUtility.h"

bool SceneWarmupData::needWarmup() const
{
	return warmupFrameRemained > 0;
}

void SceneWarmupData::update()
{
	if (warmupFrameRemained > 0) {
		--warmupFrameRemained;
	}
}

SceneRenderData::SceneRenderData()
	: lightDataPack(probePoolPack)
	, reflectionProbeDataPack(probePoolPack)
	, envLightDataPack(probePoolPack)
	, virtualShadowMapRenderData(lightDataPack)
{
	staticMeshTransformRenderData.setFrequentUpdate(false);
}

void SceneRenderData::setCamera(CameraRenderData* cameraRenderData)
{
	if (cameraRenderData) {
		if (cameraRenderData->isMainCamera)
			cameraRenderDatas.insert(cameraRenderDatas.begin(), cameraRenderData);
		else
			cameraRenderDatas.push_back(cameraRenderData);
		cameraRenderData->probeGrid.probePool = &probePoolPack;
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

int SceneRenderData::setReflectionCapture(const ReflectionProbeUpdateData& updateData)
{
	return reflectionProbeDataPack.setProbe(updateData);
}

int SceneRenderData::setEnvLightCapture(const EnvLightProbeData& envLightProbeData)
{
	return envLightProbeDataPack.setLightCapture(envLightProbeData);
}

int SceneRenderData::setEnvLightData(const EnvLightUpdateData& updateData)
{
	return envLightDataPack.setLightData(updateData);
}

unsigned int SceneRenderData::setMeshTransform(const MeshTransformData& data)
{
	return meshTransformRenderData.setMeshTransform(data);
}

MeshTransformDataArray::ReservedData SceneRenderData::addMeshTransform(unsigned int count)
{
	return meshTransformRenderData.addMeshTransform(count);
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

ViewCulledMeshBatchDrawData SceneRenderData::getViewCulledBatchDrawData(CameraRenderData* cameraRenderData, bool isStatic)
{
	ViewCulledMeshBatchDrawData batchDrawData;
	if (cameraRenderData) {
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
	WarmupData.update();
	
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
		updateRenderDataRenderThread(cameraRenderData, RenderThread::get().getRenderFrame());
		if (cameraRenderData->usedFrameRenderThread < (long long)Time::frames()) {
			cameraRenderData->upload();
			cameraRenderData->usedFrameRenderThread = Time::frames();
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