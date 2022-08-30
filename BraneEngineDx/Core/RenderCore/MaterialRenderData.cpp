#include "MaterialRenderData.h"

void MaterialRenderData::create()
{
	if (material == NULL)
		return;
	desc = material->desc;
	for (auto attr : desc.textureField) {
		if (attr.second.val)
			attr.second.val->bind();
	}
	for (auto attr : desc.imageField) {
		if (attr.second.val.isValid())
			attr.second.val.texture->bind();
	}
	if (vendorMaterial == NULL) {
		vendorMaterial = VendorManager::getInstance().getVendor().newMaterial(desc);
		if (vendorMaterial == NULL) {
			throw runtime_error("Vendor new Material failed");
		}
	}
	vendorMaterial->program = program;
}

void MaterialRenderData::release()
{
	if (vendorMaterial)
		delete vendorMaterial;
}

void MaterialRenderData::upload()
{
	if (vendorMaterial == NULL)
		return;
	vendorMaterial->preprocess();
	vendorMaterial->processScalarData();
	vendorMaterial->processCountData();
	vendorMaterial->processColorData();
	vendorMaterial->processMatrixData();
}

void MaterialRenderData::bind(IRenderContext& context)
{
	if (vendorMaterial == NULL)
		return;
	if (material->isTwoSide)
		context.setCullState(Cull_Off);
	else if (material->cullFront)
		context.setCullState(Cull_Front);
	else
		context.setCullState(Cull_Back);
	context.bindMaterialBuffer(vendorMaterial);
	context.bindMaterialTextures(vendorMaterial);
	context.bindMaterialImages(vendorMaterial);
}