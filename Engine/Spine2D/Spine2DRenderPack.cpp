#include "Spine2DRenderPack.h"

#include "../Core/RenderCore/RenderTask.h"
#include "../Core/RenderCore/RenderCoreUtility.h"

bool Spine2DRenderCommand::isValid() const
{
	return materialRenderData && materialRenderData->isValid() && mesh != NULL && mesh->isValid();
}

Enum<ShaderFeature> Spine2DRenderCommand::getShaderFeature() const
{
	return ShaderFeature::Shader_Default;
}

CullType Spine2DRenderCommand::getCullType() const
{
	return resource.cullType;
}

uint16_t Spine2DRenderCommand::getRenderStage() const
{
	return RenderStage::RS_Post;
}

RenderMode Spine2DRenderCommand::getRenderMode(const Name& passName, const CameraRenderData* cameraRenderData) const
{
	return RenderMode(RenderStage::RS_Post, blendMode);
}

bool Spine2DRenderCommand::canCastShadow() const
{
	return false;
}

IRenderPack* Spine2DRenderCommand::createRenderPack(SceneRenderData& sceneData, RenderCommandList& commandList) const
{
	return new Spine2DRenderPack();
}

Spine2DRenderPack::Spine2DRenderPack()
{
}

bool Spine2DRenderPack::setRenderCommand(const IRenderCommand& command, const RenderTask& task)
{
	const Spine2DRenderCommand* spine2dCommand = dynamic_cast<const Spine2DRenderCommand*>(&command);
	if (spine2dCommand == NULL)
		return false;

	materialData = command.materialRenderData;
	if (materialData == NULL)
		return false;

	Draw& draw = drawList.emplace_back();
	draw.resource = spine2dCommand->resource;
	draw.meshPartDesc = {
		spine2dCommand->mesh->meshData,
		spine2dCommand->mesh->vertexFirst,
		spine2dCommand->mesh->vertexCount,
		spine2dCommand->mesh->elementFirst,
		spine2dCommand->mesh->elementCount
	};

	return true;
}

void Spine2DRenderPack::excute(IRenderContext& context, RenderTask& task, RenderTaskContext& taskContext)
{
	for (const auto& draw : drawList) {
		auto iter = task.materialVariant->desc.colorField.find("baseColor");
		if (iter != task.materialVariant->desc.colorField.end()) {
			iter->second.val = draw.resource.color;
		}

		static const ShaderPropertyName mainTextureName = "mainTexture";

		context.bindTexture((ITexture*)draw.resource.texture->getVendorTexture(), mainTextureName);
		context.bindMaterialBuffer(task.materialVariant);

		context.meshDrawCall(draw.meshPartDesc);
	}
}

void Spine2DRenderPack::reset()
{
	materialData = NULL;
	drawList.clear();
}
