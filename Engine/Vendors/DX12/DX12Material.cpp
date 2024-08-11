#include "DX12Material.h"

#ifdef VENDOR_USE_DX12

DX12Material::DX12Material(DX12Context& context, MaterialDesc& desc)
	: dxContext(context), IMaterial(desc)
{
	rootSignature.init(context.device);
}

DX12Material::~DX12Material()
{
	release();
}

void DX12Material::uploadAttribute(DX12ShaderProgram* program, const Name& name, unsigned int size, void* data)
{
	if (matInsBufHost == NULL || program == NULL)
		return;
	const DX12ShaderProgram::AttributeDesc* desc = program->getAttributeOffset(name);
	if (desc == NULL)
		return;
	const ShaderProperty* prop = desc->getParameter();
	if (prop == NULL)
		return;
	memcpy_s((char*)matInsBufHost + prop->offset, prop->size, data, size);
}

void DX12Material::preprocess()
{
	DX12ShaderProgram* program = DX12ShaderProgram::currentDx12Program;
	if (lastShaderName == program->name)
		return;
	if (drawInfoBuf == NULL) {
		static const int size = sizeof(DrawInfo);
		drawInfoBuf = dxContext.constantBufferPool.suballocate(size, 256);
		if (drawInfoBuf == NULL) {
			throw runtime_error("drawInfoBuf CreateResource failed");
		}
	}

	if (matInsBuf == NULL) {
		matInsBuf = program->allocateMatInsBuf();
		if (matInsBuf != NULL) {
			if (matInsBufHost != NULL)
				delete matInsBufHost;
			matInsBufHost = new char[matInsBuf->getSize()];
		}
	}

	rootSignature.setDesc(program->rootSignatureDesc);
	if (drawInfoBuf != NULL)
		rootSignature.setBuffer(DRAW_INFO_BIND_INDEX, drawInfoBuf->getCBV());
	if (matInsBuf != NULL)
		rootSignature.setBuffer(MAT_INS_BIND_INDEX, matInsBuf->getCBV());
}

void DX12Material::processBaseData()
{
	drawInfo.passID = desc.currentPass;
	drawInfo.passNum = desc.passNum;
}

void DX12Material::processScalarData()
{
	DX12ShaderProgram* program = DX12ShaderProgram::currentDx12Program;
	if (program == NULL)
		return;
	for (auto b = desc.scalarField.begin(), e = desc.scalarField.end(); b != e; b++)
		uploadAttribute(program, b->first, sizeof(float), &b->second.val);
}

void DX12Material::processCountData()
{
	DX12ShaderProgram* program = DX12ShaderProgram::currentDx12Program;
	if (program == NULL)
		return;
	for (auto b = desc.countField.begin(), e = desc.countField.end(); b != e; b++)
		uploadAttribute(program, b->first, sizeof(int), &b->second.val);
}

void DX12Material::processColorData()
{
	DX12ShaderProgram* program = DX12ShaderProgram::currentDx12Program;
	if (program == NULL)
		return;
	for (auto b = desc.colorField.begin(), e = desc.colorField.end(); b != e; b++)
		uploadAttribute(program, b->first, sizeof(Color), &b->second.val);
}

void DX12Material::processMatrixData()
{
	DX12ShaderProgram* program = DX12ShaderProgram::currentDx12Program;
	if (program == NULL)
		return;
	for (auto b = desc.matrixField.begin(), e = desc.matrixField.end(); b != e; b++)
		uploadAttribute(program, b->first, sizeof(Matrix4f), b->second.val.data());
}

void DX12Material::processTextureData()
{
	DX12ShaderProgram* program = DX12ShaderProgram::currentDx12Program;
	if (program == NULL)
		return;
	for (auto b = desc.textureField.begin(), e = desc.textureField.end(); b != e; b++) {
		if (b->second.val == NULL || b->second.val->bind() == 0)
			continue;
		DX12Texture2D* tex = (DX12Texture2D*)b->second.val->getVendorTexture();
		const DX12ShaderProgram::AttributeDesc* desc = program->getAttributeOffset(b->first);
		if (desc == NULL)
			continue;
		for (auto& prop : desc->properties) {
			if (prop.second->type != ShaderProperty::Texture)
				continue;
			rootSignature.setTexture(prop.second->offset, tex->getSRV());
			DX12ResourceView sampler = tex->getSampler();
			if (sampler.isValid() && prop.second->meta >= 0)
				rootSignature.setSampler(prop.second->meta, sampler);
			return;
		}
	}
}

void DX12Material::processImageData()
{
	DX12ShaderProgram* program = DX12ShaderProgram::currentDx12Program;
	if (program == NULL)
		return;
	for (auto b = desc.imageField.begin(), e = desc.imageField.end(); b != e; b++) {
		if (!b->second.val.isValid())
			continue;
		if (b->second.val.texture->bind() == 0)
			continue;
		DX12Texture2D* tex = (DX12Texture2D*)b->second.val.texture->getVendorTexture();
		const DX12ShaderProgram::AttributeDesc* desc = program->getAttributeOffset(b->first);
		if (desc == NULL)
			continue;
		for (auto& prop : desc->properties) {
			if (prop.second->type != ShaderProperty::Image)
				continue;
			rootSignature.setImage(prop.second->offset, tex->getUAV(b->second.val.level));
			return;
		}
	}
}

void DX12Material::postprocess()
{
	if (matInsBuf != NULL) {
		matInsBuf->upload(matInsBufHost, matInsBuf->getSize(), 0);
	}
	if (rootSignature.isValid()) {
		if (desc.shader->isComputable())
			dxContext.graphicContext.setComputeRootSignature(&rootSignature);
		else
			dxContext.graphicContext.setRootSignature(&rootSignature);
	}
}

void DX12Material::release()
{
	rootSignature.release();
	if (drawInfoBuf != NULL) {
		drawInfoBuf->release();
		drawInfoBuf = NULL;
	}
	if (matInsBuf != NULL) {
		matInsBuf->release();
		matInsBuf = NULL;
	}
	if (matInsBufHost != NULL) {
		delete matInsBufHost;
		matInsBufHost = NULL;
	}
}

#endif // VENDOR_USE_DX12