#include "RenderTask.h"
#include "CameraRenderData.h"
#include "SceneRenderData.h"
#include "../RenderTarget.h"
#include "../Profile/RenderProfile.h"

size_t RenderTask::Hasher::operator()(const RenderTask* t) const
{
	return (*this)(*t);
}

size_t RenderTask::Hasher::operator()(const RenderTask& t) const
{
	size_t hash = (size_t)t.sceneData;
	hash_combine(hash, (size_t)t.batchDrawData.transformData);
	hash_combine(hash, (size_t)t.batchDrawData.batchDrawCommandArray);
	hash_combine(hash, (size_t)t.shaderProgram);
	hash_combine(hash, (size_t)t.surface.renderTarget);
	hash_combine(hash, (size_t)t.cameraData->cameraRenderID);
	hash_combine(hash, (size_t)t.materialVariant);
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
		if (t0.surface.renderTarget < t1.surface.renderTarget)
			return true;
		if (t0.surface.renderTarget == t1.surface.renderTarget) {
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
							if (t0.batchDrawData.transformData < t1.batchDrawData.transformData)
								return true;
							if (t0.batchDrawData.transformData == t1.batchDrawData.transformData) {
								if (t0.batchDrawData.batchDrawCommandArray < t1.batchDrawData.batchDrawCommandArray)
									return true;
								if (t0.batchDrawData.batchDrawCommandArray == t1.batchDrawData.batchDrawCommandArray) {
									if (t0.meshData < t1.meshData)
										return true;
									if (t0.meshData == t1.meshData)
										return t0.materialVariant < t1.materialVariant;
								}
							}
						}
					}
				}
			}
		}
	}
	return false;
}

void RenderTask::execute(RenderTaskParameter& parameter)
{
	IRenderContext& context = *parameter.renderContext;
	RENDER_SCOPE_NO_CONTEXT(RenderTask)
	RenderTaskContext& taskContext = *parameter.taskContext;
	//Time t = Time::now();

	bool shaderSwitch = false;

	if (taskContext.renderTarget != surface.renderTarget) {
		taskContext.renderTarget = surface.renderTarget;

		IRenderTarget* renderTarget = surface.renderTarget->getVendorRenderTarget();
		context.bindFrame(renderTarget);
	}

	if (taskContext.shaderProgram != shaderProgram) {
		taskContext.shaderProgram = shaderProgram;
		context.bindShaderProgram(shaderProgram);

		shaderSwitch = true;
		sceneData->bind(context);
	}

	if (taskContext.cameraData != cameraData || shaderSwitch) {
		taskContext.cameraData = cameraData;

		context.setViewport(0, 0, cameraData->data.viewSize.x(), cameraData->data.viewSize.y());
		cameraData->bind(context);
		if (cameraData->forceStencilTest)
			context.setStencilRef(cameraData->stencilRef);
	}

	if (taskContext.sceneData != sceneData || shaderSwitch) {
		taskContext.sceneData = sceneData;

		sceneData->bind(context);
	}

	if (taskContext.batchDrawData.transformData != batchDrawData.transformData || shaderSwitch) {
		taskContext.batchDrawData.transformData = batchDrawData.transformData;

		if (batchDrawData.transformData)
			batchDrawData.transformData->bind(context);
	}

	if (taskContext.batchDrawData.batchDrawCommandArray != batchDrawData.batchDrawCommandArray || shaderSwitch) {
		taskContext.batchDrawData.batchDrawCommandArray = batchDrawData.batchDrawCommandArray;

		if (batchDrawData.batchDrawCommandArray)
			batchDrawData.batchDrawCommandArray->bindInstanceBuffer(context);
	}

	if (taskContext.renderMode != renderMode) {
		taskContext.renderMode = renderMode;
		
		uint16_t stage = renderMode.getRenderStage();
		if (stage < RenderStage::RS_Opaque)
			context.setRenderPreState(renderMode.getDepthStencilMode());
		else if (stage < RenderStage::RS_Aplha)
			context.setRenderOpaqueState(renderMode.getDepthStencilMode());
		else if (stage < RenderStage::RS_Transparent)
			context.setRenderAlphaState(renderMode.getDepthStencilMode());
		else if (stage < RenderStage::RS_Post)
			context.setRenderTransparentState(renderMode.getDepthStencilMode());
		else {
			BlendMode blendMode = renderMode.getBlendMode();
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

	for (auto data : extraData) {
		data->bind(context);
	}
	//setupTime = setupTime + Time::now() - t;

	//t = Time::now();
	if (taskContext.meshData != meshData) {
		taskContext.meshData = meshData;
		if (meshData)
			context.bindMeshData(meshData);
	}

	if (taskContext.materialVariant != materialVariant) {
		if (!cameraData->forceStencilTest)
			context.setStencilRef(materialVariant->desc.stencilValue);
	}

	renderPack->excute(context, *this, taskContext);

	//execTime = execTime + Time::now() - t;
}
