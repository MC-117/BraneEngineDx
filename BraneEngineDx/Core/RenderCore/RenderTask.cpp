#include "RenderTask.h"

size_t RenderTask::Hasher::operator()(const RenderTask* t) const
{
	return (*this)(*t);
}

size_t RenderTask::Hasher::operator()(const RenderTask& t) const
{
	size_t hash = (size_t)t.cameraData->cameraRender;
	hash_combine(hash, (size_t)t.shaderProgram);
	hash_combine(hash, (size_t)t.sceneData);
	hash_combine(hash, (size_t)t.materialData->material);
	hash_combine(hash, (size_t)t.meshData);
	for (auto data : t.extraData)
		hash_combine(hash, (size_t)data);
	return hash;
}

bool RenderTask::ExecutionOrder::operator()(const RenderTask* t0, const RenderTask* t1) const
{
	return (*this)(*t0, *t1);
}

bool RenderTask::ExecutionOrder::operator()(const RenderTask& t0, const RenderTask& t1) const
{
	if (t0.sceneData < t1.sceneData)
		return true;
	if (t0.sceneData == t1.sceneData) {
		if (t0.cameraData->renderOrder < t1.cameraData->renderOrder)
			return true;
		if (t0.cameraData->renderOrder == t1.cameraData->renderOrder) {
			if (t0.cameraData < t1.cameraData)
				return true;
			if (t0.cameraData == t1.cameraData) {
				uint32_t order0 = t0.renderMode.getOrder();
				uint32_t order1 = t1.renderMode.getOrder();
				if (order0 < order1)
					return true;
				if (order0 == order1) {
					if (t0.shaderProgram < t1.shaderProgram)
						return true;
					if (t0.shaderProgram == t1.shaderProgram) {
						if (t0.meshData < t1.meshData)
							return true;
						if (t0.meshData == t1.meshData)
							return t0.materialData < t1.materialData;
					}
				}
			}
		}
	}
	return false;
}
