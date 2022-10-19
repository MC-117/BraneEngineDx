#include "RenderCommandList.h"
#include "../Camera.h"
#include "../Render.h"
#include "../DirectLight.h"
#include "../PointLight.h"
#include "../Console.h"
#include "../IVendor.h"
#include "../Engine.h"

void SceneRenderData::setLight(Render* lightRender)
{
	if (lightRender->getRenderType() == IRendering::Light)
		lightDataPack.setLight((Light*)lightRender);
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

void* SceneRenderData::getMeshPartTransform(MeshPart* meshPart, Material* material)
{
	return meshTransformDataPack.getMeshPartTransform(meshPart, material);
}

void* SceneRenderData::setMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex)
{
	void* re = meshTransformDataPack.setMeshPartTransform(meshPart, material, transformIndex);
	/*if (re != NULL && material->getRenderOrder() >= 1000 && material->getRenderOrder() < 2450 && material->canCastShadow)
		meshTransformDataPack.setMeshPartTransform(meshPart, &Material::defaultDepthMaterial, transformIndex);*/
	return re;
}

void* SceneRenderData::setMeshPartTransform(MeshPart* meshPart, Material* material, void* transformIndex)
{
	void* re = meshTransformDataPack.setMeshPartTransform(meshPart, material, (MeshTransformIndex*)transformIndex);
	/*if (re != NULL && material->getRenderOrder() >= 1000 && material->getRenderOrder() < 2450 && material->canCastShadow)
		meshTransformDataPack.setMeshPartTransform(meshPart, &Material::defaultDepthMaterial, (MeshTransformIndex*)transformIndex);*/
	return re;
}

unsigned int SceneRenderData::setStaticMeshTransform(const Matrix4f& transformMat)
{
	return meshTransformDataPack.setStaticMeshTransform(transformMat);
}

unsigned int SceneRenderData::setStaticMeshTransform(const vector<Matrix4f>& transformMats)
{
	return meshTransformDataPack.setStaticMeshTransform(transformMats);
}

void* SceneRenderData::getStaticMeshPartTransform(MeshPart* meshPart, Material* material)
{
	return meshTransformDataPack.getStaticMeshPartTransform(meshPart, material);
}

void* SceneRenderData::setStaticMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex)
{
	void* re = meshTransformDataPack.setStaticMeshPartTransform(meshPart, material, transformIndex);
	//if (re != NULL && material->getRenderOrder() >= 1000 && material->getRenderOrder() < 2450 && material->canCastShadow)
	//	meshTransformDataPack.setStaticMeshPartTransform(meshPart, &Material::defaultDepthMaterial, transformIndex);
	return re;
}

void* SceneRenderData::setStaticMeshPartTransform(MeshPart* meshPart, Material* material, void* transformIndex)
{
	void* re = meshTransformDataPack.setStaticMeshPartTransform(meshPart, material, (MeshTransformIndex*)transformIndex);
	//if (re != NULL && material->getRenderOrder() >= 1000 && material->getRenderOrder() < 2450 && material->canCastShadow)
	//	meshTransformDataPack.setStaticMeshPartTransform(meshPart, &Material::defaultDepthMaterial, (MeshTransformIndex*)transformIndex);
	return re;
}

void SceneRenderData::cleanStaticMeshTransform(unsigned int base, unsigned int count)
{
	meshTransformDataPack.cleanStatic(base, count);
}

void SceneRenderData::cleanStaticMeshPartTransform(MeshPart* meshPart, Material* material)
{
	meshTransformDataPack.cleanPartStatic(meshPart, material);
}

void SceneRenderData::setUpdateStatic()
{
	meshTransformDataPack.setUpdateStatic();
}

bool SceneRenderData::willUpdateStatic()
{
	return meshTransformDataPack.staticUpdate;
}

void SceneRenderData::create()
{
	meshTransformDataPack.create();
	particleDataPack.create();
	lightDataPack.create();
}

void SceneRenderData::reset()
{
	meshTransformDataPack.clean();
	particleDataPack.clean();
	lightDataPack.clean();
}

void SceneRenderData::release()
{
	meshTransformDataPack.release();
	particleDataPack.release();
	lightDataPack.release();
}

void SceneRenderData::upload()
{
	meshTransformDataPack.upload();
	particleDataPack.upload();
	lightDataPack.upload();
}

void SceneRenderData::bind(IRenderContext& context)
{
	meshTransformDataPack.bind(context);
	particleDataPack.bind(context);
	lightDataPack.bind(context);
}

bool RenderCommandList::addRenderTask(const IRenderCommand& cmd, RenderTask& task)
{
	task.hashCode = RenderTask::Hasher()(task);

	RenderTask* pTask = NULL;
	auto taskIter = taskSet.find(&task);
	if (taskIter == taskSet.end()) {
		pTask = new RenderTask(task);
		//taskMap.insert(make_pair(task.hashCode, pTask));
		taskSet.insert(pTask);
	}
	else {
		pTask = *taskIter;
		task.renderPack = pTask->renderPack;
		*pTask = task;
	}

	if (pTask->renderPack == NULL) {
		task.renderPack = pTask->renderPack = cmd.createRenderPack(*cmd.sceneData, *this);
	}

	if (!pTask->renderPack->setRenderCommand(cmd))
		return false;

	pTask->age = 0;
	return true;
}

bool RenderCommandList::setRenderCommand(const IRenderCommand& cmd, ShaderFeature extraFeature)
{
	if (!cmd.isValid())
		return false;
	Enum<ShaderFeature> shaderFeature = cmd.getShaderFeature();
	shaderFeature |= extraFeature;
	ShaderProgram* shader = cmd.material->getShader()->getProgram(shaderFeature);
	if (shader == NULL) {
		Console::warn("Shader %s don't have mode %d", cmd.material->getShaderName().c_str(), shaderFeature.enumValue);
		return false;
	}

	if (!shader->init())
		return false;

	CameraRenderData* cameraRenderData = dynamic_cast<CameraRenderData*>(cmd.camera->getRenderData());
	if (cameraRenderData == NULL)
		return false;
	if (cameraRenderData->usedFrame < (long long)Time::frames()) {
		cameraRenderData->create();
		cameraRenderData->usedFrame = Time::frames();
	}

	MaterialRenderData* materialRenderData = dynamic_cast<MaterialRenderData*>(cmd.material->getRenderData());
	if (materialRenderData == NULL)
		return false;
	if (materialRenderData->usedFrame < (long long)Time::frames()) {
		materialRenderData->program = shader;
		materialRenderData->create();
		materialRenderData->usedFrame = Time::frames();
	}

	for (auto binding : cmd.bindings) {
		if (binding->usedFrame < (long long)Time::frames()) {
			binding->create();
			binding->usedFrame = Time::frames();
		}
	}

	MeshData* meshData = cmd.mesh == NULL ? NULL : cmd.mesh->meshData;
	if (meshData)
		meshData->init();

	RenderTask task;
	task.age = 0;
	task.sceneData = cmd.sceneData;
	task.shaderProgram = shader;
	task.renderMode = cmd.getRenderMode();
	task.cameraData = cameraRenderData;
	task.materialData = materialRenderData;
	task.meshData = meshData;
	task.extraData = cmd.bindings;

	return addRenderTask(cmd, task);
}

bool RenderCommandList::setRenderCommand(const IRenderCommand& cmd, vector<ShaderFeature> extraFeatures)
{
	if (!cmd.isValid())
		return false;

	CameraRenderData* cameraRenderData = dynamic_cast<CameraRenderData*>(cmd.camera->getRenderData());
	if (cameraRenderData == NULL)
		return false;
	if (cameraRenderData->usedFrame < (long long)Time::frames()) {
		cameraRenderData->create();
		cameraRenderData->usedFrame = Time::frames();
	}

	for (auto binding : cmd.bindings) {
		if (binding->usedFrame < (long long)Time::frames()) {
			binding->create();
			binding->usedFrame = Time::frames();
		}
	}

	MeshData* meshData = cmd.mesh == NULL ? NULL : cmd.mesh->meshData;
	if (meshData)
		meshData->init();

	bool success = true;

	for (auto extraFeature : extraFeatures) {
		Enum<ShaderFeature> shaderFeature = cmd.getShaderFeature();
		shaderFeature |= extraFeature;
		ShaderProgram* shader = cmd.material->getShader()->getProgram(shaderFeature);
		if (shader == NULL) {
			Console::warn("Shader %s don't have mode %d", cmd.material->getShaderName().c_str(), shaderFeature.enumValue);
			success &= false;
			continue;
		}

		if (!shader->init()) {
			success &= false;
			continue;
		}

		MaterialRenderData* materialRenderData = dynamic_cast<MaterialRenderData*>(cmd.material->getRenderData());
		if (materialRenderData == NULL) {
			success &= false;
			continue;
		}
		if (materialRenderData->usedFrame < (long long)Time::frames()) {
			materialRenderData->program = shader;
			materialRenderData->create();
			materialRenderData->usedFrame = Time::frames();
		}

		RenderTask task;
		task.age = 0;
		task.sceneData = cmd.sceneData;
		task.shaderProgram = shader;
		task.renderMode = cmd.getRenderMode();
		task.cameraData = cameraRenderData;
		task.materialData = materialRenderData;
		task.meshData = meshData;
		task.extraData = cmd.bindings;

		success &= addRenderTask(cmd, task);
	}

	return success;
}

/*
		 |   0   |       10       |   500   |      750      |  1000  |    2450    |     2500    |  5000   |
 Forward | Light | Depth Pre-Pass |         Geomtry         | Opaque | Alpha Mask | Transparent | Overlay |
Deferred | Light | Depth Pre-Pass | Geomtry | Alpha Geomtry | Pixel  |            |             |         |
*/

void RenderCommandList::prepareCommand()
{
}

void RenderCommandList::excuteCommand()
{
	IVendor& vendor = VendorManager::getInstance().getVendor();
	IRenderContext& context = *vendor.getDefaultRenderContext();

	Timer timer;

	timer.record("Upload");
	RenderTaskContext taskContext;
	Time setupTime, uploadBaseTime, uploadInsTime, execTime;
	for (auto item : taskSet) {
		RenderTask& task = *item;
		task.age++;
		if (task.renderPack == NULL)
			continue;
		Time t = Time::now();

		if (taskContext.sceneData != task.sceneData) {
			taskContext.sceneData = task.sceneData;

			task.sceneData->upload();
			task.sceneData->bind(context);
		}

		if (taskContext.cameraData != task.cameraData) {
			taskContext.cameraData = task.cameraData;

			IRenderTarget* renderTarget = task.cameraData->renderTarget->getVendorRenderTarget();
			context.bindFrame(renderTarget);
			context.clearFrameColors(task.cameraData->clearColors);
			context.clearFrameDepth(1);
			context.setViewport(0, 0, task.cameraData->data.viewSize.x(), task.cameraData->data.viewSize.y());

			task.cameraData->upload();
			task.cameraData->bind(context);

			setupTime = setupTime + Time::now() - t;
		}

		if (taskContext.shaderProgram != task.shaderProgram) {
			t = Time::now();
			taskContext.shaderProgram = task.shaderProgram;
			context.bindShaderProgram(task.shaderProgram);

			uploadBaseTime = uploadBaseTime + Time::now() - t;
		}

		if (taskContext.renderMode != task.renderMode) {
			uint16_t stage = task.renderMode.getRenderStage();
			if (stage < RenderStage::RS_Opaque)
				context.setRenderPreState();
			else if (stage < RenderStage::RS_Aplha)
				context.setRenderOpaqueState();
			else if (stage < RenderStage::RS_Transparent)
				context.setRenderAlphaState();
			else if (stage < RenderStage::RS_Post)
				context.setRenderTransparentState();
			else {
				BlendMode blendMode = task.renderMode.getBlendMode();
				switch (blendMode)
				{
				case BM_Default:
					context.setRenderPostState();
					break;
				case BM_Additive:
					context.setRenderPostAddState();
					break;
				case BM_Multipy:
					context.setRenderPostMultiplyState();
					break;
				case BM_PremultiplyAlpha:
					context.setRenderPostPremultiplyAlphaState();
					break;
				case BM_Mask:
					context.setRenderPostMaskState();
					break;
				default:
					throw runtime_error("Invalid blend mode");
					break;
				}
			}
		}

		for (auto data : task.extraData) {
			data->upload();
			data->bind(context);
		}

		t = Time::now();
		if (taskContext.meshData != task.meshData) {
			taskContext.meshData = task.meshData;
			if (task.meshData)
				context.bindMeshData(task.meshData);
		}

		task.renderPack->excute(context, taskContext);

		execTime = execTime + Time::now() - t;
	}

	timer.record("Execute");
	/*Console::getTimer("RCMDL") = timer;
	Timer& execTimer = Console::getTimer("RCMDL Exec");
	execTimer.setIntervalMode(true);
	execTimer.reset();
	execTimer.record("Setup", setupTime);
	execTimer.record("Base", uploadBaseTime);
	execTimer.record("Instance", uploadInsTime);
	execTimer.record("Exec", execTime);*/
}

void RenderCommandList::resetCommand()
{
	for (auto b = taskSet.begin(); b != taskSet.end();) {
		RenderTask* task = *b;
		delete task->renderPack;
		task->renderPack = NULL;
		if (task->age > 2) {
			//taskMap.erase(task->hashCode);
			b = taskSet.erase(b);
			delete task;
		}
		else b++;
	}
	IRenderContext& context = *VendorManager::getInstance().getVendor().getDefaultRenderContext();
	context.reset();
}
