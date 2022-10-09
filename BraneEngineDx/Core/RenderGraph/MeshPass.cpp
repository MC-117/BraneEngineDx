#include "MeshPass.h"

void MeshPass::prepare()
{
}

void MeshPass::execute(IRenderContext& context)
{
	if (commandList == NULL)
		return;

	timer.reset();

	timer.record("Upload");
	RenderTaskContext taskContext;
	Time setupTime, uploadBaseTime, uploadInsTime, execTime;
	for (auto item : commandList->taskSet) {
		RenderTask& task = *item;
		task.age++;
		if (task.renderPack == NULL)
			continue;
		Time t = Time::now();

		if (taskContext.sceneData != task.sceneData) {
			taskContext.sceneData = task.sceneData;

			task.sceneData->bind(context);
		}

		if (taskContext.cameraData != task.cameraData) {
			taskContext.cameraData = task.cameraData;

			IRenderTarget* renderTarget = task.cameraData->renderTarget->getVendorRenderTarget();
			context.bindFrame(renderTarget);
			context.clearFrameColor(task.cameraData->clearColor);
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

	context.setCullState(CullType::Cull_Back);

	timer.record("Execute");
}

void MeshPass::reset()
{
}
