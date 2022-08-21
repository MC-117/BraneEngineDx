#include "RenderCommandList.h"
#include "../Camera.h"
#include "../Render.h"
#include "../DirectLight.h"
#include "../PointLight.h"
#include "../Console.h"
#include "../IVendor.h"
#include "../Engine.h"

void RenderCommandList::setLight(Render* lightRender)
{
	if (lightRender->getRenderType() == IRendering::Light)
		lightDataPack.setLight((Light*)lightRender);
}

unsigned int RenderCommandList::setMeshTransform(const Matrix4f& transformMat)
{
	return meshTransformDataPack.setMeshTransform(transformMat);
}

unsigned int RenderCommandList::setMeshTransform(const vector<Matrix4f>& transformMats)
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

void* RenderCommandList::getMeshPartTransform(MeshPart* meshPart, Material* material)
{
	return meshTransformDataPack.getMeshPartTransform(meshPart, material);
}

void* RenderCommandList::setMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex)
{
	void* re = meshTransformDataPack.setMeshPartTransform(meshPart, material, transformIndex);
	/*if (re != NULL && material->getRenderOrder() >= 1000 && material->getRenderOrder() < 2450 && material->canCastShadow)
		meshTransformDataPack.setMeshPartTransform(meshPart, &Material::defaultDepthMaterial, transformIndex);*/
	return re;
}

void* RenderCommandList::setMeshPartTransform(MeshPart* meshPart, Material* material, void* transformIndex)
{
	void* re = meshTransformDataPack.setMeshPartTransform(meshPart, material, (MeshTransformIndex*)transformIndex);
	/*if (re != NULL && material->getRenderOrder() >= 1000 && material->getRenderOrder() < 2450 && material->canCastShadow)
		meshTransformDataPack.setMeshPartTransform(meshPart, &Material::defaultDepthMaterial, (MeshTransformIndex*)transformIndex);*/
	return re;
}

unsigned int RenderCommandList::setStaticMeshTransform(const Matrix4f& transformMat)
{
	return meshTransformDataPack.setStaticMeshTransform(transformMat);
}

unsigned int RenderCommandList::setStaticMeshTransform(const vector<Matrix4f>& transformMats)
{
	return meshTransformDataPack.setStaticMeshTransform(transformMats);
}

void* RenderCommandList::getStaticMeshPartTransform(MeshPart* meshPart, Material* material)
{
	return meshTransformDataPack.getStaticMeshPartTransform(meshPart, material);
}

void* RenderCommandList::setStaticMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex)
{
	void* re = meshTransformDataPack.setStaticMeshPartTransform(meshPart, material, transformIndex);
	if (re != NULL && material->getRenderOrder() >= 1000 && material->getRenderOrder() < 2450 && material->canCastShadow)
		meshTransformDataPack.setStaticMeshPartTransform(meshPart, &Material::defaultDepthMaterial, transformIndex);
	return re;
}

void* RenderCommandList::setStaticMeshPartTransform(MeshPart* meshPart, Material* material, void* transformIndex)
{
	void* re = meshTransformDataPack.setStaticMeshPartTransform(meshPart, material, (MeshTransformIndex*)transformIndex);
	if (re != NULL && material->getRenderOrder() >= 1000 && material->getRenderOrder() < 2450 && material->canCastShadow)
		meshTransformDataPack.setStaticMeshPartTransform(meshPart, &Material::defaultDepthMaterial, (MeshTransformIndex*)transformIndex);
	return re;
}

void RenderCommandList::cleanStaticMeshTransform(unsigned int base, unsigned int count)
{
	meshTransformDataPack.cleanStatic(base, count);
}

void RenderCommandList::cleanStaticMeshPartTransform(MeshPart* meshPart, Material* material)
{
	meshTransformDataPack.cleanPartStatic(meshPart, material);
}

bool RenderCommandList::setRenderCommand(const RenderCommand& cmd, bool isStatic)
{
	return setRenderCommand(cmd, isStatic, true);
}

void RenderCommandList::setUpdateStatic()
{
	meshTransformDataPack.setUpdateStatic();
}

bool RenderCommandList::willUpdateStatic()
{
	return meshTransformDataPack.staticUpdate;
}

bool RenderCommandList::setRenderCommand(const RenderCommand& cmd, bool isStatic, bool autoFill)
{
	if (cmd.material == NULL || cmd.material->isNull() || cmd.camera == NULL || (cmd.mesh == NULL && cmd.particles == NULL) ||
		(cmd.mesh == NULL && cmd.particles != NULL && cmd.particles->empty()) || (cmd.mesh != NULL && !cmd.mesh->isValid()))
		return false;
	Enum<ShaderFeature> shaderFeature;
	if (cmd.mesh == NULL)
		shaderFeature |= Shader_Particle;
	else {
		if (cmd.material->isDeferred)
			shaderFeature |= Shader_Deferred;
		if (cmd.mesh->meshData->type == MT_Terrain) {
			shaderFeature |= Shader_Terrain;
		}
		else {
			if (cmd.mesh->meshData->type == MT_SkeletonMesh) {
				shaderFeature |= Shader_Skeleton;
				if (cmd.mesh->isMorph())
					shaderFeature |= Shader_Morph;
			}
			if (cmd.particles != NULL)
				shaderFeature |= Shader_Modifier;
		}
	}
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

	RenderTask task;
	task.age = 0;
	task.shaderProgram = shader;
	task.cameraData = cameraRenderData;
	task.materialData = materialRenderData;
	task.extraData = cmd.bindings;

	MeshData* meshData = cmd.mesh == NULL ? NULL : cmd.mesh->meshData;
	task.meshData = meshData;
	if (meshData)
		meshData->init();

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
		if (cmd.mesh == NULL)
			pTask->renderPack = new ParticleRenderPack();
		else
			pTask->renderPack = new MeshDataRenderPack();
	}

	if (cmd.mesh == NULL) {
		if (cmd.particles != NULL) {
			ParticleRenderPack* prp = dynamic_cast<ParticleRenderPack*>(pTask->renderPack);
			prp->particleData = particleDataPack.setParticles(cmd.material, *cmd.particles);
		}
		else
			return false;
	}
	else {
		MeshDataRenderPack* meshRenderPack = dynamic_cast<MeshDataRenderPack*>(pTask->renderPack);
		if (isStatic) {
			auto meshTDIter = meshTransformDataPack.staticMeshTransformIndex.find(makeGuid(cmd.mesh, cmd.material));
			if (meshTDIter != meshTransformDataPack.staticMeshTransformIndex.end())
				meshRenderPack->setRenderData(cmd.mesh, &meshTDIter->second);
			else
				return false;
		}
		else {
			auto meshTDIter = meshTransformDataPack.meshTransformIndex.find(makeGuid(cmd.mesh, cmd.material));
			if (meshTDIter != meshTransformDataPack.meshTransformIndex.end())
				meshRenderPack->setRenderData(cmd.mesh, &meshTDIter->second);
			else
				return false;
		}
	}

	pTask->age = 0;
	/*if (autoFill && shader->renderOrder >= 1000 && shader->renderOrder < 2450) {
		RenderCommand _cmd = cmd;
		_cmd.material = &Material::defaultDepthMaterial;
		return setRenderCommand(_cmd, isStatic, false);
	}*/
	return true;
}

/*
		 |   0   |       10       |   500   |      750      |  1000  |    2450    |     2500    |  5000   |
 Forward | Light | Depth Pre-Pass |         Geomtry         | Opaque | Alpha Mask | Transparent | Overlay |
Deferred | Light | Depth Pre-Pass | Geomtry | Alpha Geomtry | Pixel  |            |             |         |
*/

void RenderCommandList::prepareCommand()
{
	meshTransformDataPack.create();
	particleDataPack.create();
	lightDataPack.create();
}

void RenderCommandList::excuteCommand()
{
	IVendor& vendor = VendorManager::getInstance().getVendor();
	Timer timer;
	meshTransformDataPack.upload();
	particleDataPack.upload();
	lightDataPack.upload();

	timer.record("Upload");
	RenderTask taskContext;
	IRenderContext& context = *vendor.getDefaultRenderContext();
	Time setupTime, uploadBaseTime, uploadInsTime, execTime;
	for (auto item : taskSet) {
		RenderTask& task = *item;
		task.age++;
		if (task.renderPack == NULL)
			continue;
		Time t = Time::now();
		if (taskContext.cameraData != task.cameraData) {
			taskContext.cameraData = task.cameraData;

			IRenderTarget* renderTarget = task.cameraData->renderTarget->getVendorRenderTarget();
			context.bindFrame(renderTarget);
			context.clearFrameColor(task.cameraData->clearColor);
			context.clearFrameDepth(1);
			context.setViewport(0, 0, task.cameraData->data.viewSize.x(), task.cameraData->data.viewSize.y());

			setupTime = setupTime + Time::now() - t;
		}

		if (taskContext.shaderProgram != task.shaderProgram) {
			t = Time::now();
			taskContext.shaderProgram = task.shaderProgram;
			if (task.shaderProgram->renderOrder < 500)
				context.setRenderPreState();
			else if (task.shaderProgram->renderOrder < 1000)
				context.setRenderGeomtryState();
			else if (task.shaderProgram->renderOrder < 2450)
				context.setRenderOpaqueState();
			else if (task.shaderProgram->renderOrder < 2500)
				context.setRenderAlphaState();
			else if (task.shaderProgram->renderOrder < 5000)
				context.setRenderTransparentState();
			else
				context.setRenderOverlayState();
			context.bindShaderProgram(task.shaderProgram);

			task.cameraData->upload();
			task.cameraData->bind(context);
			meshTransformDataPack.bind(context);
			particleDataPack.bind(context);
			lightDataPack.bind(context);

			uploadBaseTime = uploadBaseTime + Time::now() - t;
		}

		if (taskContext.materialData != task.materialData) {
			t = Time::now();
			taskContext.materialData = task.materialData;
			task.materialData->upload();
			task.materialData->bind(context);
			if (lightDataPack.shadowTarget == NULL)
				context.bindTexture((ITexture*)Texture2D::whiteRGBADefaultTex.getVendorTexture(), "depthMap");
			else
				context.bindTexture((ITexture*)lightDataPack.shadowTarget->getDepthTexture()->getVendorTexture(), "depthMap");
			uploadInsTime = uploadInsTime + Time::now() - t;
		}

		t = Time::now();
		if (taskContext.meshData != task.meshData) {
			taskContext.meshData = task.meshData;
			if (task.meshData)
				context.bindMeshData(task.meshData);
		}

		for (auto data : task.extraData) {
			data->upload();
			data->bind(context);
		}

		for (int passIndex = 0; passIndex < task.materialData->desc.passNum; passIndex++) {
			task.materialData->desc.currentPass = passIndex;
			context.setDrawInfo(passIndex, task.materialData->desc.passNum);
			task.renderPack->excute(context);
		}

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
	meshTransformDataPack.clean();
	particleDataPack.clean();
	lightDataPack.clean();
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