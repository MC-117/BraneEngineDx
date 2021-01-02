#include "DX11ShaderStage.h"

#ifdef VENDOR_USE_DX11

DX11ShaderStage::DX11ShaderStage(DX11Context& context, const ShaderStageDesc& desc)
    : dxContext(context), ShaderStage(desc)
{
}

DX11ShaderStage::~DX11ShaderStage()
{
	release();
}

unsigned int DX11ShaderStage::compile(const string& code, string& errorString)
{
	ID3DBlob* errorBlob;
	if (FAILED(D3DCompile(code.c_str(), code.size() * sizeof(char), name.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", getShaderVersion(stageType), D3DCOMPILE_ENABLE_STRICTNESS, 0, &dx11ShaderBlob, &errorBlob))) {
		errorString = (const char*)errorBlob->GetBufferPointer();
		return 0;
	}
	if (errorBlob != NULL)
		errorBlob->Release();
	if (FAILED(createShader(dxContext.device, stageType, dx11ShaderBlob, &dx11Shader))) {
		errorString = "DX11 Create Shader failed";
		return 0;
	}
	shaderId = (unsigned long long)dx11Shader;

	if (FAILED(D3DReflect(dx11ShaderBlob->GetBufferPointer(), dx11ShaderBlob->GetBufferSize(),
			IID_PPV_ARGS(&dx11ShaderReflector)))) {
		if (!errorString.empty())
			errorString += '\n';
		errorString += "D3DReflect: reflection error";
	}

	dx11MatInsBufReflector = dx11ShaderReflector->GetConstantBufferByName(MatInsBufName.c_str());
	if (dx11MatInsBufReflector != NULL) {
		D3D11_SHADER_BUFFER_DESC bufDesc;
		ZeroMemory(&bufDesc, sizeof(D3D11_SHADER_BUFFER_DESC));
		if (FAILED(dx11MatInsBufReflector->GetDesc(&bufDesc)))
			dx11MatInsBufReflector = NULL;
		else if (bufDesc.Name != MatInsBufName)
			dx11MatInsBufReflector = NULL;
	}

    return shaderId;
}

void DX11ShaderStage::release()
{
	if (dx11ShaderBlob != NULL) {
		dx11ShaderBlob->Release();
		dx11ShaderBlob = NULL;
	}
	if (dx11Shader != NULL) {
		dx11Shader->Release();
		dx11Shader = NULL;
	}
	if (dx11ShaderReflector != NULL) {
		dx11ShaderReflector->Release();
		dx11ShaderReflector = NULL;
	}
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

unsigned int DX11ShaderProgram::nextProgramID = 1;
DX11ShaderProgram* DX11ShaderProgram::currentDx11Program = NULL;

DX11ShaderProgram::DX11ShaderProgram(DX11Context& context) : dxContext(context)
{
	programId = nextProgramID;
	nextProgramID++;
}

DX11ShaderProgram::~DX11ShaderProgram()
{
	if (matInsBuf != NULL)
		matInsBuf->Release();
	if (currentDx11Program == this)
		currentDx11Program = NULL;
}

unsigned int DX11ShaderProgram::bind()
{
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
		if (dx11MatInsBufReflector == NULL) {
			DX11ShaderStage* dss = (DX11ShaderStage*)b->second;
			dx11MatInsBufReflector = dss->dx11MatInsBufReflector;
		}
	}
	if (matInsBuf == NULL) {
		if (dx11MatInsBufReflector != NULL) {
			D3D11_SHADER_BUFFER_DESC bufDesc;
			ZeroMemory(&bufDesc, sizeof(D3D11_SHADER_BUFFER_DESC));
			if (SUCCEEDED(dx11MatInsBufReflector->GetDesc(&bufDesc))) {
				D3D11_BUFFER_DESC cbDesc;
				ZeroMemory(&cbDesc, sizeof(D3D11_BUFFER_DESC));
				cbDesc.Usage = D3D11_USAGE_DYNAMIC;
				cbDesc.ByteWidth = bufDesc.Size;
				cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				dxContext.device->CreateBuffer(&cbDesc, NULL, &matInsBuf);
				matInsBufSize = bufDesc.Size;
				matInsBufHost = (unsigned char*)malloc(matInsBufSize);
			}
		}
	}
	if (matInsBuf != NULL)
		for (auto b = shaderStages.begin(), e = shaderStages.end(); b != e; b++) {
			switch (b->first)
			{
			case Vertex_Shader_Stage:
				dxContext.deviceContext->VSSetConstantBuffers(MAT_INS_BIND_INDEX, 1, &matInsBuf);
				break;
			case Fragment_Shader_Stage:
				dxContext.deviceContext->PSSetConstantBuffers(MAT_INS_BIND_INDEX, 1, &matInsBuf);
				break;
			case Geometry_Shader_Stage:
				dxContext.deviceContext->GSSetConstantBuffers(MAT_INS_BIND_INDEX, 1, &matInsBuf);
				break;
			case Compute_Shader_Stage:
				dxContext.deviceContext->CSSetConstantBuffers(MAT_INS_BIND_INDEX, 1, &matInsBuf);
				break;
			case Tessellation_Control_Shader_Stage:
				dxContext.deviceContext->HSSetConstantBuffers(MAT_INS_BIND_INDEX, 1, &matInsBuf);
				break;
			case Tessellation_Evalution_Shader_Stage:
				dxContext.deviceContext->DSSetConstantBuffers(MAT_INS_BIND_INDEX, 1, &matInsBuf);
				break;
			}
		}
	currentDx11Program = this;
	currentProgram = programId;
    return programId;
}

bool DX11ShaderProgram::dispatchCompute(unsigned int dimX, unsigned int dimY, unsigned int dimZ)
{
	if (!isComputable())
		return false;
	if (currentProgram != programId)
		return false;
	dxContext.deviceContext->Dispatch(dimX, dimY, dimZ);
	return true;
}

DX11ShaderProgram::AttributeDesc DX11ShaderProgram::getAttributeOffset(const string& name)
{
	auto iter = attributes.find(name);
	if (iter != attributes.end())
		return iter->second;
	AttributeDesc desc = { name, false, -1, 0, -1 };
	if (dx11MatInsBufReflector != NULL) {
		D3D11_SHADER_VARIABLE_DESC vdesc;
		ZeroMemory(&vdesc, sizeof(D3D11_SHADER_VARIABLE_DESC));
		auto var = dx11MatInsBufReflector->GetVariableByName(name.c_str());
		if (var != NULL) {
			if (SUCCEEDED(var->GetDesc(&vdesc))) {
				if (vdesc.Name == name) {
					desc.offset = vdesc.StartOffset;
					desc.size = vdesc.Size;
				}
			}
		}
	}
	if (desc.offset == -1)
		for (auto b = shaderStages.begin(), e = shaderStages.end(); b != e; b++) {
			DX11ShaderStage* dss = (DX11ShaderStage*)b->second;
			if (dss->dx11ShaderReflector == NULL)
				continue;
			if (desc.offset == -1) {
				desc.isTex = true;
				D3D11_SHADER_INPUT_BIND_DESC bdesc;
				ZeroMemory(&bdesc, sizeof(D3D11_SHADER_INPUT_BIND_DESC));
				if (SUCCEEDED(dss->dx11ShaderReflector->GetResourceBindingDescByName(name.c_str(), &bdesc))) {
					if (bdesc.Name == name) {
						desc.offset = bdesc.BindPoint;
						desc.size = 1;
						desc.meta = b->first;
					}
				}
			}
			if (desc.offset != -1) {
				attributes.emplace(make_pair(name, desc));
				break;
			}
		}
	return desc;
}

void DX11ShaderProgram::memoryBarrier(unsigned int bitEnum)
{
}

void DX11ShaderProgram::uploadData()
{
	if (matInsBuf == NULL || matInsBufHost == NULL || matInsBufSize == 0)
		return;
	D3D11_MAPPED_SUBRESOURCE mpd;
	dxContext.deviceContext->Map(matInsBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mpd);
	memcpy_s(mpd.pData, matInsBufSize, matInsBufHost, matInsBufSize);
	dxContext.deviceContext->Unmap(matInsBuf, 0);
}

void DX11ShaderProgram::uploadAttribute(const string& name, unsigned int size, void* data)
{
	if (matInsBufHost == NULL)
		return;
	AttributeDesc desc = getAttributeOffset(name);
	if (desc.isTex || desc.offset == -1 || desc.size < size)
		return;
	memcpy_s(matInsBufHost + desc.offset, desc.size, data, size);
}

void DX11ShaderProgram::uploadTexture(const string& name, ID3D11ShaderResourceView* tex, ID3D11SamplerState* sample)
{
	if (tex == NULL)
		return;
	AttributeDesc desc = getAttributeOffset(name);
	if (!desc.isTex || desc.offset == -1 || desc.meta == -1)
		return;
	switch (desc.meta)
	{
	case Vertex_Shader_Stage:
		dxContext.deviceContext->VSSetShaderResources(desc.offset, 1, &tex);
		if (sample != NULL)
			dxContext.deviceContext->VSSetSamplers(desc.offset, 1, &sample);
		break;
	case Fragment_Shader_Stage:
		dxContext.deviceContext->PSSetShaderResources(desc.offset, 1, &tex);
		if (sample != NULL)
			dxContext.deviceContext->PSSetSamplers(desc.offset, 1, &sample);
		break;
	case Geometry_Shader_Stage:
		dxContext.deviceContext->GSSetShaderResources(desc.offset, 1, &tex);
		if (sample != NULL)
			dxContext.deviceContext->GSSetSamplers(desc.offset, 1, &sample);
		break;
	case Compute_Shader_Stage:
		dxContext.deviceContext->CSSetShaderResources(desc.offset, 1, &tex);
		if (sample != NULL)
			dxContext.deviceContext->CSSetSamplers(desc.offset, 1, &sample);
		break;
	case Tessellation_Control_Shader_Stage:
		dxContext.deviceContext->HSSetShaderResources(desc.offset, 1, &tex);
		if (sample != NULL)
			dxContext.deviceContext->HSSetSamplers(desc.offset, 1, &sample);
		break;
	case Tessellation_Evalution_Shader_Stage:
		dxContext.deviceContext->DSSetShaderResources(desc.offset, 1, &tex);
		if (sample != NULL)
			dxContext.deviceContext->DSSetSamplers(desc.offset, 1, &sample);
		break;
	default:
		return;
	}
}

void DX11ShaderProgram::uploadImage(const string& name, ID3D11UnorderedAccessView* tex)
{
	if (tex == NULL)
		return;
	AttributeDesc desc = getAttributeOffset(name);
	if (!desc.isTex || desc.offset == -1 || desc.meta == -1)
		return;
	if (desc.meta == Compute_Shader_Stage)
		dxContext.deviceContext->CSGetUnorderedAccessViews(desc.offset, 1, &tex);
}

#endif // VENDOR_USE_DX11
