#include "MaterialRenderData.h"

bool MaterialRenderData::isValid() const
{
	return desc.shader;
}

Shader* MaterialRenderData::getShader()
{
	return desc.shader;
}

Name MaterialRenderData::getShaderName() const
{
	if (desc.shader)
		return desc.shader->getName();
	return Name::none;
}

IMaterial* MaterialRenderData::getVariant(const Enum<ShaderFeature>& features, const ShaderMatchRule& rule)
{
	auto iter = variants.find(features);
	if (iter == variants.end()) {
		ShaderProgram* program = desc.shader->getProgram(features, rule);
		if (program) {
			IMaterial* vendorMaterial = VendorManager::getInstance().getVendor().newMaterial(desc);
			if (vendorMaterial == NULL) {
				throw runtime_error("Vendor new Material failed");
			}
			vendorMaterial->program = program;
			variants.emplace(features, vendorMaterial);
			return vendorMaterial;
		}
	}
	else {
		return iter->second;
	}
	return NULL;
}

void MaterialRenderData::create()
{
	if (material == NULL)
		return;
	desc = material->desc;
	renderOrder = material->getRenderOrder();
	canCastShadow = material->canCastShadow;
	for (auto attr : desc.textureField) {
		if (attr.second.val.isValid()) {
			attr.second.val->bind();
		}
	}
	for (auto attr : desc.imageField) {
		if (attr.second.val.isValid())
			attr.second.val.texture->bind();
	}
}

void MaterialRenderData::release()
{
	for (auto item : variants) {
		delete item.second;
	}
	variants.clear();
}

void MaterialRenderData::upload()
{
	for (auto item : variants) {
		IMaterial* variant = item.second;
		variant->preprocess();
		variant->processScalarData();
		variant->processCountData();
		variant->processColorData();
		variant->processMatrixData();
	}
}

void MaterialRenderData::bind(IRenderContext& context)
{
}
