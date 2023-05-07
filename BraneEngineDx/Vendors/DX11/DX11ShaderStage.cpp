#include "DX11ShaderStage.h"
#include "../../Core/Utility/RenderUtility.h"
#include "../../Core/Utility/EngineUtility.h"
#include "../../Core/Engine.h"
#include <fstream>

#ifdef VENDOR_USE_DX11

const char* DX11ShaderStage::MatInsBufName = "MatInsBuf";
const char* DX11ShaderStage::DrawInfoBufName = "DrawInfoBuf";
ShaderPropertyName DX11ShaderStage::materialParameterBufferName(DX11ShaderStage::MatInsBufName);
ShaderPropertyName DX11ShaderStage::drawInfoBufferName(DX11ShaderStage::DrawInfoBufName);

DX11ShaderStage::DX11ShaderStage(DX11Context& context, const ShaderStageDesc& desc)
    : dxContext(context), ShaderStage(desc)
{
}

DX11ShaderStage::~DX11ShaderStage()
{
	release();
}

unsigned int DX11ShaderStage::compile(const ShaderMacroSet& macroSet, const string& code, string& errorString)
{
	ShaderStage::compile(macroSet, code, errorString);
	ComPtr<ID3DBlob> errorBlob = NULL;
	unsigned int compileFlag = D3DCOMPILE_ENABLE_STRICTNESS;
	int shaderDebug = 0;
	Engine::engineConfig.configInfo.get("shaderDebug", shaderDebug);
	if (shaderDebug || code.find("#pragma debug") != string::npos) {
		compileFlag |= D3DCOMPILE_SKIP_OPTIMIZATION |
			D3DCOMPILE_DEBUG | D3DCOMPILE_PREFER_FLOW_CONTROL;
	}
	const char* shdtmp = ".shdtmp/";
	if (!filesystem::exists(shdtmp))
		filesystem::create_directory(shdtmp);
	string rootPath = getFileRoot(Engine::windowContext.executionPath);
	string shaderPath = rootPath + '/' + shdtmp + name + "_" + getShaderFeatureNames(this->shaderFeature) + getShaderExtension(stageType);
	ofstream f = ofstream(shaderPath);
	if (f.fail()) {
		if (FAILED(D3DCompile(this->code.c_str(), this->code.size() * sizeof(char), name.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main", getShaderVersion(stageType), compileFlag, 0, &dx11ShaderBlob, &errorBlob))) {
			errorString = (const char*)errorBlob->GetBufferPointer();
			return 0;
		}
	}
	else {
		f << this->code;
		f.close();
		if (FAILED(D3DCompileFromFile(filesystem::path(shaderPath).c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main", getShaderVersion(stageType), compileFlag, 0, &dx11ShaderBlob, &errorBlob))) {
			errorString = (const char*)errorBlob->GetBufferPointer();
			return 0;
		}
	}
	/*if (errorBlob != NULL)
		errorBlob->Release();*/
	if (FAILED(createShader(dxContext.device.Get(), stageType, dx11ShaderBlob.Get(), &dx11Shader))) {
		errorString = "DX11 Create Shader failed";
		return 0;
	}
	shaderId = (unsigned long long)dx11Shader.Get();

	if (FAILED(D3DReflect(dx11ShaderBlob->GetBufferPointer(), dx11ShaderBlob->GetBufferSize(),
			IID_PPV_ARGS(&dx11ShaderReflector)))) {
		if (!errorString.empty())
			errorString += '\n';
		errorString += "D3DReflect: reflection error";
		return 0;
	}

	properties.clear();

	D3D11_SHADER_DESC shaderDesc = { 0 };
	dx11ShaderReflector->GetDesc(&shaderDesc);

	for (int i = 0; i < shaderDesc.BoundResources; i++) {
		D3D11_SHADER_INPUT_BIND_DESC desc = { 0 };
		dx11ShaderReflector->GetResourceBindingDesc(i, &desc);
		size_t nameHash = ShaderPropertyName::calHash(desc.Name);
		ShaderProperty& shaderProperty = properties.insert(make_pair(
			nameHash, ShaderProperty())).first->second;
		shaderProperty.name = desc.Name;
		switch (desc.Type)
		{
		case D3D_SIT_CBUFFER:
			shaderProperty.type = ShaderProperty::ConstantBuffer;
			break;
		case D3D_SIT_TEXTURE:
			shaderProperty.type = desc.Dimension == D3D_SRV_DIMENSION_BUFFER ?
				ShaderProperty::TextureBuffer : ShaderProperty::Texture;
			break;
		case D3D_SIT_TBUFFER:
		case D3D_SIT_STRUCTURED:
		case D3D_SIT_BYTEADDRESS:
			shaderProperty.type = ShaderProperty::TextureBuffer;
			break;
		case D3D_SIT_SAMPLER:
			shaderProperty.type = ShaderProperty::Sampler;
			break;
		case D3D_SIT_UAV_RWTYPED:
		case D3D_SIT_UAV_RWSTRUCTURED:
		case D3D_SIT_UAV_RWBYTEADDRESS:
		case D3D_SIT_UAV_APPEND_STRUCTURED:
		case D3D_SIT_UAV_CONSUME_STRUCTURED:
		case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
			shaderProperty.type = ShaderProperty::Image;
			break;
		default:
			break;
		}
		shaderProperty.offset = desc.BindPoint;
		if (shaderProperty.type == ShaderProperty::ConstantBuffer) {
			ID3D11ShaderReflectionConstantBuffer* constantBuffer = dx11ShaderReflector->GetConstantBufferByName(desc.Name);
			D3D11_SHADER_BUFFER_DESC bufDesc = { 0 };
			if (SUCCEEDED(constantBuffer->GetDesc(&bufDesc))) {
				shaderProperty.size = bufDesc.Size;
				if (nameHash == materialParameterBufferName.getHash()) {
					for (int i = 0; i < bufDesc.Variables; i++) {
						ID3D11ShaderReflectionVariable* variable = constantBuffer->GetVariableByIndex(i);
						D3D11_SHADER_VARIABLE_DESC desc = { 0 };
						variable->GetDesc(&desc);
						ShaderProperty& shaderProperty = properties.insert(make_pair(
							ShaderPropertyName::calHash(desc.Name), ShaderProperty())).first->second;
						shaderProperty.name = desc.Name;
						shaderProperty.type = ShaderProperty::Parameter;
						shaderProperty.offset = desc.StartOffset;
						shaderProperty.size = desc.Size;
						shaderProperty.meta = 0;
					}
				}
			}
		}
		else
			shaderProperty.size = desc.BindCount;
		shaderProperty.meta = -1;
	}

	for (auto& prop : properties) {
		if (prop.second.type != ShaderProperty::Texture)
			continue;
		const ShaderProperty* samplerProp = getProperty(prop.second.name + "Sampler");
		if (samplerProp && samplerProp->type == ShaderProperty::Sampler)
			prop.second.meta = samplerProp->offset;
	}

    return shaderId;
}

void DX11ShaderStage::release()
{
	if (dx11ShaderBlob != NULL) {
		dx11ShaderBlob.Reset();
	}
	if (dx11Shader != NULL) {
		dx11Shader.Reset();
	}
	if (dx11ShaderReflector != NULL) {
		dx11ShaderReflector.Reset();
	}
	properties.clear();
	shaderId = 0;
}

const char* DX11ShaderStage::getShaderVersion(ShaderStageType type)
{
	switch (type)
	{
	case Vertex_Shader_Stage:
		return "vs_5_0";
	case Fragment_Shader_Stage:
		return "ps_5_0";
	case Geometry_Shader_Stage:
		return "gs_5_0";
	case Compute_Shader_Stage:
		return "cs_5_0";
	case Tessellation_Control_Shader_Stage:
		return "hs_5_0";
	case Tessellation_Evalution_Shader_Stage:
		return "ds_5_0";
	}
	return NULL;
}

HRESULT DX11ShaderStage::createShader(ID3D11Device* device, ShaderStageType type, ID3DBlob* pShaderBlob, ID3D11DeviceChild** ppShader)
{
	switch (type)
	{
	case Vertex_Shader_Stage:
		return device->CreateVertexShader(pShaderBlob->GetBufferPointer(),
			pShaderBlob->GetBufferSize(), NULL, (ID3D11VertexShader**)ppShader);
	case Fragment_Shader_Stage:
		return device->CreatePixelShader(pShaderBlob->GetBufferPointer(),
			pShaderBlob->GetBufferSize(), NULL, (ID3D11PixelShader**)ppShader);
	case Geometry_Shader_Stage:
		return device->CreateGeometryShader(pShaderBlob->GetBufferPointer(),
			pShaderBlob->GetBufferSize(), NULL, (ID3D11GeometryShader**)ppShader);
	case Compute_Shader_Stage:
		return device->CreateComputeShader(pShaderBlob->GetBufferPointer(),
			pShaderBlob->GetBufferSize(), NULL, (ID3D11ComputeShader**)ppShader);
	case Tessellation_Control_Shader_Stage:
		return device->CreateHullShader(pShaderBlob->GetBufferPointer(),
			pShaderBlob->GetBufferSize(), NULL, (ID3D11HullShader**)ppShader);
	case Tessellation_Evalution_Shader_Stage:
		return device->CreateDomainShader(pShaderBlob->GetBufferPointer(),
			pShaderBlob->GetBufferSize(), NULL, (ID3D11DomainShader**)ppShader);
	}
	return E_INVALIDARG;
}

bool DrawInfo::operator==(const DrawInfo& i) const
{
	return baseVertex == i.baseVertex &&
		baseInstance == i.baseInstance &&
		passID == i.passID &&
		passNum == i.passNum &&
		materialID == i.materialID &&
		gameTime == i.gameTime;
}

bool DrawInfo::operator!=(const DrawInfo& i) const
{
	return baseVertex != i.baseVertex ||
		baseInstance != i.baseInstance ||
		passID != i.passID ||
		passNum != i.passNum ||
		materialID != i.materialID ||
		gameTime != i.gameTime;
}

unsigned int DX11ShaderProgram::nextProgramID = 1;
DX11ShaderProgram* DX11ShaderProgram::currentDx11Program = NULL;

DX11ShaderProgram::DX11ShaderProgram(DX11Context& context) : dxContext(context)
{
	programId = nextProgramID;
	nextProgramID++;
}

DX11ShaderProgram::~DX11ShaderProgram()
{
	if (matInsBuf != NULL) {
		matInsBuf.Reset();
	}
	if (currentDx11Program == this)
		currentDx11Program = NULL;
}

bool DX11ShaderProgram::init()
{
	if (!isValid())
		return false;
	if (!dirty)
		return true;
	ShaderProgram::init();
	if (const AttributeDesc* desc = getAttributeOffset(DX11ShaderStage::materialParameterBufferName))
		if (const ShaderProperty* prop = desc->getConstantBuffer())
			matInsBufSize = prop->size;
	dirty = false;
	return true;
}

unsigned int DX11ShaderProgram::bind()
{
	if (dirty) {
		if (currentProgram == programId)
			currentProgram = 0;
		drawInfoBuf.Reset();
		matInsBuf.Reset();
		attributes.clear();
		matInsBufSize = 0;
	}
	if (currentProgram == programId)
		return programId;
	dxContext.deviceContext->VSSetShader(NULL, NULL, 0);
	dxContext.deviceContext->PSSetShader(NULL, NULL, 0);
	dxContext.deviceContext->GSSetShader(NULL, NULL, 0);
	dxContext.deviceContext->CSSetShader(NULL, NULL, 0);
	dxContext.deviceContext->HSSetShader(NULL, NULL, 0);
	dxContext.deviceContext->DSSetShader(NULL, NULL, 0);

	for (auto b = shaderStages.begin(), e = shaderStages.end(); b != e; b++) {
		switch (b->first)
		{
		case Vertex_Shader_Stage:
			dxContext.deviceContext->VSSetShader((ID3D11VertexShader*)b->second->getShaderID(), NULL, 0);
			break;
		case Fragment_Shader_Stage:
			dxContext.deviceContext->PSSetShader((ID3D11PixelShader*)b->second->getShaderID(), NULL, 0);
			break;
		case Geometry_Shader_Stage:
			dxContext.deviceContext->GSSetShader((ID3D11GeometryShader*)b->second->getShaderID(), NULL, 0);
			break;
		case Compute_Shader_Stage:
			dxContext.deviceContext->CSSetShader((ID3D11ComputeShader*)b->second->getShaderID(), NULL, 0);
			break;
		case Tessellation_Control_Shader_Stage:
			dxContext.deviceContext->HSSetShader((ID3D11HullShader*)b->second->getShaderID(), NULL, 0);
			break;
		case Tessellation_Evalution_Shader_Stage:
			dxContext.deviceContext->DSSetShader((ID3D11DomainShader*)b->second->getShaderID(), NULL, 0);
			break;
		}
	}

	currentDx11Program = this;
	currentProgram = programId;
	dirty = false;
    return programId;
}

bool DX11ShaderProgram::dispatchCompute(unsigned int dimX, unsigned int dimY, unsigned int dimZ)
{
	if (!isComputable())
		return false;
	uploadDrawInfo();
	if (currentProgram != programId)
		return false;
	dxContext.deviceContext->Dispatch(dimX, dimY, dimZ);
	return true;
}

int DX11ShaderProgram::getMaterialBufferSize()
{
	return matInsBufSize;
}

void DX11ShaderProgram::memoryBarrier(unsigned int bitEnum)
{
}

void DX11ShaderProgram::uploadDrawInfo()
{
	if (drawInfoBuf == NULL)
		return;
	D3D11_MAPPED_SUBRESOURCE mpd;
	dxContext.deviceContext->Map(drawInfoBuf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mpd);
	memcpy_s(mpd.pData, sizeof(DrawInfo), &drawInfo, sizeof(DrawInfo));
	dxContext.deviceContext->Unmap(drawInfoBuf.Get(), 0);
}

void DX11ShaderProgram::uploadData()
{
	if (matInsBuf == NULL || matInsBufHost == NULL || matInsBufSize == 0)
		return;
	D3D11_MAPPED_SUBRESOURCE mpd;
	dxContext.deviceContext->Map(matInsBuf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mpd);
	memcpy_s(mpd.pData, matInsBufSize, matInsBufHost, matInsBufSize);
	dxContext.deviceContext->Unmap(matInsBuf.Get(), 0);
}

void DX11ShaderProgram::uploadAttribute(const string& name, unsigned int size, void* data)
{
	if (matInsBufHost == NULL)
		return;
	const AttributeDesc* desc = getAttributeOffset(name);
	if (desc == NULL)
		return;
	const ShaderProperty* prop = desc->getParameter();
	if (prop == NULL)
		return;
	memcpy_s(matInsBufHost + prop->offset, prop->size, data, size);
}

bool DX11ShaderProgram::unbindBuffer(ComPtr<ID3D11DeviceContext> deviceContext, const ShaderPropertyName& name) const
{
	const AttributeDesc* desc = getAttributeOffset(name);
	if (desc == NULL)
		return false;
	for (auto& prop : desc->properties) {
		switch (prop.second->type)
		{
		case ShaderProperty::ConstantBuffer: {
			ID3D11Buffer* buffer = NULL;
			switch (prop.first)
			{
			case Vertex_Shader_Stage:
				deviceContext->VSSetConstantBuffers(prop.second->offset, 1, &buffer);
				break;
			case Fragment_Shader_Stage:
				deviceContext->PSSetConstantBuffers(prop.second->offset, 1, &buffer);
				break;
			case Geometry_Shader_Stage:
				deviceContext->GSSetConstantBuffers(prop.second->offset, 1, &buffer);
				break;
			case Compute_Shader_Stage:
				deviceContext->CSSetConstantBuffers(prop.second->offset, 1, &buffer);
				break;
			case Tessellation_Control_Shader_Stage:
				deviceContext->HSSetConstantBuffers(prop.second->offset, 1, &buffer);
				break;
			case Tessellation_Evalution_Shader_Stage:
				deviceContext->DSSetConstantBuffers(prop.second->offset, 1, &buffer);
				break;
			}
			break;
		}
		case ShaderProperty::TextureBuffer:
		case ShaderProperty::Texture: {
			ID3D11ShaderResourceView* srv = NULL;
			switch (prop.first)
			{
			case Vertex_Shader_Stage:
				deviceContext->VSSetShaderResources(prop.second->offset, 1, &srv);
				break;
			case Fragment_Shader_Stage:
				deviceContext->PSSetShaderResources(prop.second->offset, 1, &srv);
				break;
			case Geometry_Shader_Stage:
				deviceContext->GSSetShaderResources(prop.second->offset, 1, &srv);
				break;
			case Compute_Shader_Stage:
				deviceContext->CSSetShaderResources(prop.second->offset, 1, &srv);
				break;
			case Tessellation_Control_Shader_Stage:
				deviceContext->HSSetShaderResources(prop.second->offset, 1, &srv);
				break;
			case Tessellation_Evalution_Shader_Stage:
				deviceContext->DSSetShaderResources(prop.second->offset, 1, &srv);
				break;
			}
			break;
		}
		case ShaderProperty::Image: {
			ID3D11UnorderedAccessView* uav = NULL;
			switch (prop.first)
			{
			case Compute_Shader_Stage:
				deviceContext->CSSetUnorderedAccessViews(prop.second->offset, 1, &uav, NULL);
				break;
			case Fragment_Shader_Stage:
				deviceContext->OMSetRenderTargetsAndUnorderedAccessViews(D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL,
					NULL, NULL, prop.second->offset, 1, &uav, NULL);
				break;
			default:
				break;
			}
		}
		default:
			break;
		}
	}
}

bool DX11ShaderProgram::bindCBV(ComPtr<ID3D11DeviceContext> deviceContext, const ShaderPropertyName& name, ComPtr<ID3D11Buffer> buffer) const
{
	const AttributeDesc* desc = getAttributeOffset(name);
	if (desc == NULL)
		return false;
	for (auto& prop : desc->properties) {
		switch (prop.first)
		{
		case Vertex_Shader_Stage:
			deviceContext->VSSetConstantBuffers(prop.second->offset, 1, buffer.GetAddressOf());
			break;
		case Fragment_Shader_Stage:
			deviceContext->PSSetConstantBuffers(prop.second->offset, 1, buffer.GetAddressOf());
			break;
		case Geometry_Shader_Stage:
			deviceContext->GSSetConstantBuffers(prop.second->offset, 1, buffer.GetAddressOf());
			break;
		case Compute_Shader_Stage:
			deviceContext->CSSetConstantBuffers(prop.second->offset, 1, buffer.GetAddressOf());
			break;
		case Tessellation_Control_Shader_Stage:
			deviceContext->HSSetConstantBuffers(prop.second->offset, 1, buffer.GetAddressOf());
			break;
		case Tessellation_Evalution_Shader_Stage:
			deviceContext->DSSetConstantBuffers(prop.second->offset, 1, buffer.GetAddressOf());
			break;
		}
	}
	return true;
}

bool DX11ShaderProgram::bindSRV(ComPtr<ID3D11DeviceContext> deviceContext, const ShaderPropertyName& name, ComPtr<ID3D11ShaderResourceView> srv) const
{
	const AttributeDesc* desc = getAttributeOffset(name);
	if (desc == NULL)
		return false;
	for (auto& prop : desc->properties) {
		if (prop.second->type != ShaderProperty::Texture &&
			prop.second->type != ShaderProperty::TextureBuffer)
			continue;
		switch (prop.first)
		{
		case Vertex_Shader_Stage:
			deviceContext->VSSetShaderResources(prop.second->offset, 1, srv.GetAddressOf());
			break;
		case Fragment_Shader_Stage:
			deviceContext->PSSetShaderResources(prop.second->offset, 1, srv.GetAddressOf());
			break;
		case Geometry_Shader_Stage:
			deviceContext->GSSetShaderResources(prop.second->offset, 1, srv.GetAddressOf());
			break;
		case Compute_Shader_Stage:
			deviceContext->CSSetShaderResources(prop.second->offset, 1, srv.GetAddressOf());
			break;
		case Tessellation_Control_Shader_Stage:
			deviceContext->HSSetShaderResources(prop.second->offset, 1, srv.GetAddressOf());
			break;
		case Tessellation_Evalution_Shader_Stage:
			deviceContext->DSSetShaderResources(prop.second->offset, 1, srv.GetAddressOf());
			break;
		}
	}
	return true;
}

bool DX11ShaderProgram::bindSRVWithSampler(ComPtr<ID3D11DeviceContext> deviceContext, const ShaderPropertyName& name, ComPtr<ID3D11ShaderResourceView> srv, ComPtr<ID3D11SamplerState> sampler) const
{
	const AttributeDesc* desc = getAttributeOffset(name);
	if (desc == NULL)
		return false;
	for (auto& prop : desc->properties) {
		if (prop.second->type != ShaderProperty::Texture)
			continue;
		switch (prop.first)
		{
		case Vertex_Shader_Stage:
			deviceContext->VSSetShaderResources(prop.second->offset, 1, srv.GetAddressOf());
			if (sampler != NULL && prop.second->meta >= 0)
				deviceContext->VSSetSamplers(prop.second->meta, 1, sampler.GetAddressOf());
			break;
		case Fragment_Shader_Stage:
			deviceContext->PSSetShaderResources(prop.second->offset, 1, srv.GetAddressOf());
			if (sampler != NULL && prop.second->meta >= 0)
				deviceContext->PSSetSamplers(prop.second->meta, 1, sampler.GetAddressOf());
			break;
		case Geometry_Shader_Stage:
			deviceContext->GSSetShaderResources(prop.second->offset, 1, srv.GetAddressOf());
			if (sampler != NULL && prop.second->meta >= 0)
				deviceContext->GSSetSamplers(prop.second->meta, 1, sampler.GetAddressOf());
			break;
		case Compute_Shader_Stage:
			deviceContext->CSSetShaderResources(prop.second->offset, 1, srv.GetAddressOf());
			if (sampler != NULL && prop.second->meta >= 0)
				deviceContext->CSSetSamplers(prop.second->meta, 1, sampler.GetAddressOf());
			break;
		case Tessellation_Control_Shader_Stage:
			deviceContext->HSSetShaderResources(prop.second->offset, 1, srv.GetAddressOf());
			if (sampler != NULL && prop.second->meta >= 0)
				deviceContext->HSSetSamplers(prop.second->meta, 1, sampler.GetAddressOf());
			break;
		case Tessellation_Evalution_Shader_Stage:
			deviceContext->DSSetShaderResources(prop.second->offset, 1, srv.GetAddressOf());
			if (sampler != NULL && prop.second->meta >= 0)
				deviceContext->DSSetSamplers(prop.second->meta, 1, sampler.GetAddressOf());
			break;
		default:
			break;
		}
	}
	return true;
}

bool DX11ShaderProgram::bindUAV(ComPtr<ID3D11DeviceContext> deviceContext, const ShaderPropertyName& name, ComPtr<ID3D11UnorderedAccessView> uav) const
{
	const AttributeDesc* desc = getAttributeOffset(name);
	if (desc == NULL)
		return false;
	for (auto& prop : desc->properties) {
		if (prop.second->type != ShaderProperty::Image)
			continue;
		switch (prop.first)
		{
		case Compute_Shader_Stage:
			deviceContext->CSSetUnorderedAccessViews(prop.second->offset, 1, uav.GetAddressOf(), NULL);
			break;
		case Fragment_Shader_Stage:
			deviceContext->OMSetRenderTargetsAndUnorderedAccessViews(D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL,
				NULL, NULL, prop.second->offset, 1, uav.GetAddressOf(), NULL);
			break;
		default:
			break;
		}
	}
	return true;
}

#endif // VENDOR_USE_DX11
