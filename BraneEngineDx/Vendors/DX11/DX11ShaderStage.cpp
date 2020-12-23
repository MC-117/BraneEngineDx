#include "DX11ShaderStage.h"

DX11ShaderStage::DX11ShaderStage(const DX11Context& context, const ShaderStageDesc& desc)
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
		"main", getShaderVersion(stageType), D3DCOMPILE_ENABLE_STRICTNESS, 0, &dx11ShaderBlob, &errorBlob)))
		errorString = (const char*)errorBlob->GetBufferPointer();
	errorBlob->Release();
	if (FAILED(createShader(dxContext.device, stageType, dx11ShaderBlob, &dx11Shader)))
		return 0;
	shaderId = (unsigned int)dx11Shader;
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

DX11ShaderProgram::DX11ShaderProgram(const DX11Context& context) : dxContext(context)
{
	programId = nextProgramID;
	nextProgramID++;
}

DX11ShaderProgram::~DX11ShaderProgram()
{
}

unsigned int DX11ShaderProgram::bind()
{
	if (currentProgram == programId)
		return programId;
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
	dxContext.deviceContext->
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

void DX11ShaderProgram::memoryBarrier(unsigned int bitEnum)
{
}
