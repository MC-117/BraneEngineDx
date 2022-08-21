#include "MeshPass.h"

void MeshPass::prepare()
{
	if (commandList == NULL)
		return;
	commandList->prepareCommand();
}

void MeshPass::execute(IRenderContext& context)
{
	if (commandList == NULL)
		return;

	timer.reset();

	commandList->meshTransformDataPack.upload();
	commandList->particleDataPack.upload();
	commandList->lightDataPack.upload();

	timer.record("Upload");
	RenderTask taskContext;
	Time setupTime, uploadBaseTime, uploadInsTime, execTime;
	for (auto item : commandList->taskSet) {
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
			commandList->meshTransformDataPack.bind(context);
			commandList->particleDataPack.bind(context);
			commandList->lightDataPack.bind(context);

			uploadBaseTime = uploadBaseTime + Time::now() - t;
		}

		if (taskContext.materialData != task.materialData) {
			t = Time::now();
			taskContext.materialData = task.materialData;
			task.materialData->upload();
			task.materialData->bind(context);
			if (commandList->lightDataPack.shadowTarget == NULL)
				context.bindTexture((ITexture*)Texture2D::whiteRGBADefaultTex.getVendorTexture(), "depthMap");
			else
				context.bindTexture((ITexture*)commandList->lightDataPack.shadowTarget->getDepthTexture()->getVendorTexture(), "depthMap");
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
}
