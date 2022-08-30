#pragma once

#include "RenderInterface.h"
#include "../Material.h"

struct MaterialRenderData : public IRenderData
{
	Material* material;
	ShaderProgram* program;
	MaterialDesc desc;
	IMaterial* vendorMaterial;

	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
};