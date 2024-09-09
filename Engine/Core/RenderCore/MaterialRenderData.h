#pragma once

#include "RenderInterface.h"
#include "../Material.h"

struct MaterialRenderData : public IRenderData
{
	Material* material;
	int renderOrder = 0;
	bool canCastShadow = true;
	MaterialDesc desc;
	map<Enum<ShaderFeature>, IMaterial*> variants;

	virtual bool isValid() const;

	Shader* getShader();
	Name getShaderName() const;

	IMaterial* getVariant(const Enum<ShaderFeature>& features, const ShaderMatchRule& rule = ShaderMatchRule());

	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
};