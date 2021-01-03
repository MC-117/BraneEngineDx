#include "DX11Material.h"

#ifdef VENDOR_USE_DX11

DX11Material::DX11Material(DX11Context& context, MaterialDesc& desc)
	: dxContext(context), IMaterial(desc)
{
}

void DX11Material::processBaseData()
{
	DX11ShaderProgram* program = DX11ShaderProgram::currentDx11Program;
	if (program != NULL)
		program->uploadAttribute("pass", sizeof(float), &desc.currentPass);
}

void DX11Material::processScalarData()
{
	DX11ShaderProgram* program = DX11ShaderProgram::currentDx11Program;
	if (program == NULL)
		return;
	for (auto b = desc.scalarField.begin(), e = desc.scalarField.end(); b != e; b++)
		program->uploadAttribute(b->first, sizeof(float), &b->second.val);
}

void DX11Material::processCountData()
{
	DX11ShaderProgram* program = DX11ShaderProgram::currentDx11Program;
	if (program == NULL)
		return;
	for (auto b = desc.countField.begin(), e = desc.countField.end(); b != e; b++)
		program->uploadAttribute(b->first, sizeof(int), &b->second.val);
}

void DX11Material::processColorData()
{
	DX11ShaderProgram* program = DX11ShaderProgram::currentDx11Program;
	if (program == NULL)
		return;
	for (auto b = desc.colorField.begin(), e = desc.colorField.end(); b != e; b++)
		program->uploadAttribute(b->first, sizeof(Color), &b->second.val);
}

void DX11Material::processTextureData()
{
	DX11ShaderProgram* program = DX11ShaderProgram::currentDx11Program;
	if (program == NULL)
		return;
	for (auto b = desc.textureField.begin(), e = desc.textureField.end(); b != e; b++) {
		if (b->second.val->bind() == 0)
			continue;
		DX11Texture2D* tex = (DX11Texture2D*)b->second.val->getVendorTexture();
		program->uploadTexture(b->first, tex->getSRV(), tex->getSampler());
	}
}

void DX11Material::processImageData()
{
	DX11ShaderProgram* program = DX11ShaderProgram::currentDx11Program;
	if (program == NULL)
		return;
	for (auto b = desc.imageField.begin(), e = desc.imageField.end(); b != e; b++) {
		if (!b->second.val.isValid())
			continue;
		if (b->second.val.texture->bind() == 0)
			continue;
		DX11Texture2D* tex = (DX11Texture2D*)b->second.val.texture->getVendorTexture();
		program->uploadImage(b->first, tex->getUAV(b->second.val.level));
	}
}

void DX11Material::postprocess()
{
	DX11ShaderProgram* program = DX11ShaderProgram::currentDx11Program;
	if (program == NULL)
		return;
	program->uploadData();
}

#endif // VENDOR_USE_DX11
