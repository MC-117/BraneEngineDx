#include "ScreenHitData.h"
#include "../RenderTarget.h"
#include "RenderCommandList.h"

void ScreenHitData::resize(unsigned int width, unsigned int height)
{
	if (depthTexture == NULL)
		depthTexture = new Texture2D(width, height, 1, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_D32_F });
	if (renderTarget == NULL)
		renderTarget = new RenderTarget(width, height, *depthTexture);
	renderTarget->resize(width, height);
}

void ScreenHitData::create()
{
	hitInfoBuffer.resize(1);
	readBackBuffer.resize(1);
}

void ScreenHitData::release()
{
	hitInfoBuffer.resize(0);
	readBackBuffer.resize(0);
}

void ScreenHitData::upload()
{
	hitInfoBuffer.uploadData(1, &hitInfo);
}

void ScreenHitData::bind(IRenderContext& context)
{
	static const ShaderPropertyName hitDataName = "hitData";
	context.bindBufferBase(hitInfoBuffer.getVendorGPUBuffer(), hitDataName, { true });
}

void ScreenHitData::readBack(IRenderContext& context)
{
	context.copyBufferData(hitInfoBuffer.getVendorGPUBuffer(), readBackBuffer.getVendorGPUBuffer());
	readBackBuffer.readData(&hitInfo);
	hitFrame = Time::frames();
}

bool ScreenHitRenderCommand::isValid() const
{
	return sceneData && batchDrawData.isValid() && mesh != NULL && mesh->isValid();
}

Enum<ShaderFeature> ScreenHitRenderCommand::getShaderFeature() const
{
	Enum<ShaderFeature> shaderFeature;
	if (mesh->meshData->type == MT_Terrain) {
		shaderFeature |= Shader_Terrain;
	}
	else {
		if (mesh->meshData->type == MT_SkeletonMesh) {
			shaderFeature |= Shader_Skeleton;
			if (mesh->isMorph())
				shaderFeature |= Shader_Morph;
		}
	}
	return shaderFeature;
}

RenderMode ScreenHitRenderCommand::getRenderMode() const
{
	return RenderMode(material->getRenderOrder(), 0, 0);
}

bool ScreenHitRenderCommand::canCastShadow() const
{
	return false;
}

IRenderPack* ScreenHitRenderCommand::createRenderPack(SceneRenderData& sceneData, RenderCommandList& commandList) const
{
	return new ScreenHitRenderPack();
}

ScreenHitRenderPack::ScreenHitRenderPack()
{
}

bool ScreenHitRenderPack::setRenderCommand(const IRenderCommand& command)
{
	const ScreenHitRenderCommand* meshRenderCommand = dynamic_cast<const ScreenHitRenderCommand*>(&command);
	if (meshRenderCommand == NULL)
		return false;

	materialData = dynamic_cast<MaterialRenderData*>(command.material->getRenderData());
	if (materialData == NULL)
		return false;

	if (meshRenderCommand->meshBatchDrawCall
		&& meshRenderCommand->meshBatchDrawCall->getInstanceCount() > 0
		&& meshRenderCommand->meshBatchDrawCall->getDrawCommandCount() > 0) {
		meshBatchDrawCalls.insert(meshRenderCommand->meshBatchDrawCall);
	}

	return true;
}

void ScreenHitRenderPack::excute(IRenderContext& context, RenderTaskContext& taskContext)
{
	if (meshBatchDrawCalls.empty())
		return;

	if (taskContext.materialData != materialData) {
		taskContext.materialData = materialData;

		materialData->bindCullMode(context, false);
		materialData->bind(context);
	}

	CameraRenderData* cameraRenderData = dynamic_cast<CameraRenderData*>(taskContext.cameraData);
	if (cameraRenderData && cameraRenderData->hitData)
		cameraRenderData->hitData->bind(context);

	for (auto& item : meshBatchDrawCalls)
	{
		if (item->reverseCullMode) {
			materialData->bindCullMode(context, true);
		}
		for (int passIndex = 0; passIndex < materialData->desc.passNum; passIndex++) {
			materialData->desc.currentPass = passIndex;
			context.setDrawInfo(passIndex, materialData->desc.passNum, materialData->desc.materialID);
			context.bindDrawInfo();
			unsigned int commandOffset = item->getDrawCommandOffset();
			unsigned int commandEnd = commandOffset + item->getDrawCommandCount();
			for (; commandOffset < commandEnd; commandOffset++) {
				context.drawMeshIndirect(taskContext.batchDrawData.batchDrawCommandArray->getCommandBuffer(), sizeof(DrawElementsIndirectCommand) * commandOffset);
			}
		}
		if (item->reverseCullMode) {
			materialData->bindCullMode(context, false);
		}
	}
}

void ScreenHitRenderPack::reset()
{
	materialData = NULL;
	meshBatchDrawCalls.clear();
}
