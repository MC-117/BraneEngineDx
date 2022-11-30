#include "Spine2DRenderPack.h"

bool Spine2DRenderCommand::isValid() const
{
	return material && !material->isNull() && mesh != NULL && mesh->isValid();
}

Enum<ShaderFeature> Spine2DRenderCommand::getShaderFeature() const
{
	return ShaderFeature::Shader_Default;
}

RenderMode Spine2DRenderCommand::getRenderMode() const
{
	return RenderMode(RenderStage::RS_Post, blendMode, 0);
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

bool Spine2DRenderPack::setRenderCommand(const IRenderCommand& command)
{
	const Spine2DRenderCommand* spine2dCommand = dynamic_cast<const Spine2DRenderCommand*>(&command);
	if (spine2dCommand == NULL)
		return false;

	materialData = dynamic_cast<MaterialRenderData*>(command.material->getRenderData());
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

void Spine2DRenderPack::excute(IRenderContext& context, RenderTaskContext& taskContext)
{
	for (const auto& draw : drawList) {
		context.setCullState(draw.resource.cullType);

		auto iter = materialData->desc.colorField.find("baseColor");
		if (iter != materialData->desc.colorField.end()) {
			iter->second.val = draw.resource.color;
		}
		context.bindTexture((ITexture*)draw.resource.texture->getVendorTexture(), "mainTexture");
		context.bindMaterialBuffer(materialData->vendorMaterial);

		context.meshDrawCall(draw.meshPartDesc);
	}
	drawList.clear();
}
