#pragma once

#include "../Core/RenderCore/MeshRenderPack.h"

struct Spine2DRenderResource
{
	Color color;
	CullType cullType;
	Texture2D* texture;
};

struct Spine2DRenderCommand : public IRenderCommand
{
	Spine2DRenderResource resource;
	BlendMode blendMode;

	virtual bool isValid() const;
	virtual Enum<ShaderFeature> getShaderFeature() const;
	virtual RenderMode getRenderMode() const;
	virtual IRenderPack* createRenderPack(SceneRenderData& sceneData, RenderCommandList& commandList) const;
};

class Spine2DRenderPack : public IRenderPack
{
public:
	struct Draw
	{
		Spine2DRenderResource resource;
		MeshPartDesc meshPartDesc;
	};
	MaterialRenderData* materialData;
	list<Draw> drawList;

	Spine2DRenderPack();

	virtual bool setRenderCommand(const IRenderCommand& command);
	virtual void excute(IRenderContext& context, RenderTaskContext& taskContext);
};