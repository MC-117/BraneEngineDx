#include "DX12Shader.h"

DX12ShaderStage::DX12ShaderStage(DX12Context& context, const ShaderStageDesc& desc)
    : dxContext(context), ShaderStage(desc)
{
}

DX12ShaderStage::~DX12ShaderStage()
{
    release();
}

unsigned int DX12ShaderStage::compile(const string& code, string& errorString)
{
	ComPtr<ID3DBlob> errorBlob = NULL;
	unsigned int compileFlag = D3DCOMPILE_ENABLE_STRICTNESS;
	if (code.find("#pragma debug") != string::npos) {
		cout << "#pragma debug\n";
		compileFlag |= D3DCOMPILE_SKIP_OPTIMIZATION |
			D3DCOMPILE_DEBUG | D3DCOMPILE_PREFER_FLOW_CONTROL;
	}
	if (FAILED(D3DCompile(code.c_str(), code.size() * sizeof(char), name.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", getShaderVersion(stageType), compileFlag, 0, &dx12ShaderBlob, &errorBlob))) {
		errorString = (const char*)errorBlob->GetBufferPointer();
		return 0;
	}
	shaderId = (unsigned long long)dx12ShaderBlob->GetBufferPointer();

	if (FAILED(D3DReflect(dx12ShaderBlob->GetBufferPointer(), dx12ShaderBlob->GetBufferSize(),
		IID_PPV_ARGS(&dx12ShaderReflector)))) {
		if (!errorString.empty())
			errorString += '\n';
		errorString += "D3DReflect: reflection error";
	}

	dx12MatInsBufReflector = dx12ShaderReflector->GetConstantBufferByName(MatInsBufName.c_str());
	if (dx12MatInsBufReflector != NULL) {
		D3D12_SHADER_BUFFER_DESC bufDesc;
		ZeroMemory(&bufDesc, sizeof(D3D12_SHADER_BUFFER_DESC));
		if (FAILED(dx12MatInsBufReflector->GetDesc(&bufDesc)))
			dx12MatInsBufReflector = NULL;
		else if (bufDesc.Name != MatInsBufName)
			dx12MatInsBufReflector = NULL;
	}

	bBindings.clear();
	tBindings.clear();
	sBindings.clear();
	ubBindings.clear();
	
	D3D12_SHADER_DESC shaderDesc = {};
	dx12ShaderReflector->GetDesc(&shaderDesc);

	int bindingCount = shaderDesc.BoundResources;
	for (int i = 0; i < bindingCount; i++) {
		D3D12_SHADER_INPUT_BIND_DESC bdesc = {};
		dx12ShaderReflector->GetResourceBindingDesc(i, &bdesc);
		if (bdesc.Type == D3D_SHADER_INPUT_TYPE::D3D_SIT_CBUFFER) {
			bBindings.push_back(bdesc.BindPoint);
		}
		else if (bdesc.Type == D3D_SHADER_INPUT_TYPE::D3D_SIT_TBUFFER) {
			tBindings.push_back(bdesc.BindPoint);
		}
		else if (bdesc.Type == D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE) {
			tBindings.push_back(bdesc.BindPoint);
		}
		else if (bdesc.Type == D3D_SHADER_INPUT_TYPE::D3D_SIT_STRUCTURED) {
			tBindings.push_back(bdesc.BindPoint);
		}
		else if (bdesc.Type == D3D_SHADER_INPUT_TYPE::D3D_SIT_SAMPLER) {
			sBindings.push_back(bdesc.BindPoint);
		}
		else if (bdesc.Type == D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWSTRUCTURED) {
			ubBindings.push_back(bdesc.BindPoint);
		}
		else if (bdesc.Type == D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWTYPED) {
			ubBindings.push_back(bdesc.BindPoint);
		}
	}

	return shaderId;
}

void DX12ShaderStage::resolveRootSignature(DX12RootSignatureDesc& desc)
{
	for (int i = 0; i < bBindings.size(); i++)
		desc.addBuffer(bBindings[i]);
	for (int i = 0; i < tBindings.size(); i++)
		desc.addTexture(tBindings[i]);
	for (int i = 0; i < sBindings.size(); i++)
		desc.addSampler(sBindings[i]);
	for (int i = 0; i < ubBindings.size(); i++)
		desc.addImage(ubBindings[i]);
}

void DX12ShaderStage::release()
{
	dx12ShaderBlob.Reset();
	dx12ShaderReflector.Reset();
	shaderId = 0;
}

const char* DX12ShaderStage::getShaderVersion(ShaderStageType type)
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

unsigned int DX12ShaderProgram::nextProgramID = 1;
DX12ShaderProgram* DX12ShaderProgram::currentDx12Program = NULL;

DX12ShaderProgram::DX12ShaderProgram(DX12Context& context) : dxContext(context)
{
	programId = nextProgramID;
	nextProgramID++;
}

DX12ShaderProgram::~DX12ShaderProgram()
{
	matInsBuf.Reset();
	if (currentDx12Program == this)
		currentDx12Program = NULL;
}

unsigned int DX12ShaderProgram::bind()
{
	if (currentProgram == programId)
		return programId;

	DX12ShaderProgramData programData;
	programData.programID = programId;
	int cbSize = 0;
	for (auto b = shaderStages.begin(), e = shaderStages.end(); b != e; b++) {
		DX12ShaderStage* stage = (DX12ShaderStage*)b->second;
		switch (b->first)
		{
		case Vertex_Shader_Stage:
			programData.VS = stage->dx12ShaderBlob;
			break;
		case Fragment_Shader_Stage:
			programData.PS = stage->dx12ShaderBlob;
			break;
		case Geometry_Shader_Stage:
			programData.GS = stage->dx12ShaderBlob;
			break;
		case Compute_Shader_Stage:
			programData.CS = stage->dx12ShaderBlob;
			break;
		case Tessellation_Control_Shader_Stage:
			programData.HS = stage->dx12ShaderBlob;
			break;
		case Tessellation_Evalution_Shader_Stage:
			programData.DS = stage->dx12ShaderBlob;
			break;
		}
		if (stage->dx12MatInsBufReflector != NULL) {
			D3D12_SHADER_BUFFER_DESC bufDesc;
			ZeroMemory(&bufDesc, sizeof(D3D12_SHADER_BUFFER_DESC));
			if (SUCCEEDED(stage->dx12MatInsBufReflector->GetDesc(&bufDesc))) {
				if (bufDesc.Size > cbSize) {
					dx12MatInsBufReflector = stage->dx12MatInsBufReflector;
				}
			}
		}
		stage->resolveRootSignature(rootSignatureDesc);
	}

	if (isComputable()) {
		dxContext.graphicContext.setComputeShader(programData);
	}
	else {
		dxContext.graphicContext.setGraphicShader(programData);
	}
	
	bindCBToStage(DRAW_INFO_BIND_INDEX, drawInfoBuf);
	bindCBToStage(MAT_INS_BIND_INDEX, matInsBuf);
	currentDx12Program = this;
	currentProgram = programId;
	return programId;
}

DX12ShaderProgram::AttributeDesc DX12ShaderProgram::getAttributeOffset(const string& name)
{
	auto iter = attributes.find(name);
	if (iter != attributes.end())
		return iter->second;
	AttributeDesc desc = { name, false, -1, 0, -1 };
	if (dx12MatInsBufReflector != NULL) {
		D3D12_SHADER_VARIABLE_DESC vdesc;
		ZeroMemory(&vdesc, sizeof(D3D12_SHADER_VARIABLE_DESC));
		auto var = dx12MatInsBufReflector->GetVariableByName(name.c_str());
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
			DX12ShaderStage* dss = (DX12ShaderStage*)b->second;
			if (dss->dx12ShaderReflector == NULL)
				continue;
			if (desc.offset == -1) {
				desc.isTex = true;
				D3D12_SHADER_INPUT_BIND_DESC bdesc;
				ZeroMemory(&bdesc, sizeof(D3D12_SHADER_INPUT_BIND_DESC));
				if (SUCCEEDED(dss->dx12ShaderReflector->GetResourceBindingDescByName(name.c_str(), &bdesc))) {
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

bool DX12ShaderProgram::dispatchCompute(unsigned int dimX, unsigned int dimY, unsigned int dimZ)
{
	if (!isComputable())
		return false;
	if (currentProgram != programId)
		return false;
	dxContext.graphicContext.dispatch(dimX, dimY, dimZ);
	return true;
}

void DX12ShaderProgram::memoryBarrier(unsigned int bitEnum)
{
}

void DX12ShaderProgram::uploadDrawInfo()
{
}

void DX12ShaderProgram::uploadData()
{
}

void DX12ShaderProgram::uploadAttribute(const string& name, unsigned int size, void* data)
{
}

void DX12ShaderProgram::uploadTexture(const string& name, DX12Descriptor tex, DX12Descriptor sample)
{
}

void DX12ShaderProgram::uploadImage(const string& name, DX12Descriptor tex)
{
}

void DX12ShaderProgram::bindCBToStage(unsigned int index, ComPtr<ID3D12Resource> buffer)
{
}

DX12SubBuffer* DX12ShaderProgram::allocateMatInsBuf()
{
	if (dx12MatInsBufReflector != NULL) {
		DX12SubBuffer* buffer = NULL;
		D3D12_SHADER_BUFFER_DESC bufDesc;
		ZeroMemory(&bufDesc, sizeof(D3D12_SHADER_BUFFER_DESC));
		if (SUCCEEDED(dx12MatInsBufReflector->GetDesc(&bufDesc))) {
			int alignedSize = GRS_UPPER(bufDesc.Size, 256);
			buffer = dxContext.constantBufferPool.suballocate(alignedSize, 256);
			if (buffer == NULL) {
				throw runtime_error("MatInsBuf CreateResource failed");
			}
		}
		return buffer;
	}
	return NULL;
}
