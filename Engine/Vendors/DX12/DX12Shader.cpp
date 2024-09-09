#include "DX12Shader.h"
#include "../../Core/Utility/RenderUtility.h"
#include "../../Core/Utility/EngineUtility.h"
#include "../../Core/Engine.h"
#include <fstream>

const char* DX12ShaderStage::MatInsBufName = "MatInsBuf";
const char* DX12ShaderStage::DrawInfoBufName = "DrawInfoBuf";
ShaderPropertyName DX12ShaderStage::materialParameterBufferName(DX12ShaderStage::MatInsBufName);
ShaderPropertyName DX12ShaderStage::drawInfoBufferName(DX12ShaderStage::DrawInfoBufName);

DX12ShaderStage::DX12ShaderStage(DX12Context& context, const ShaderStageDesc& desc)
    : dxContext(context), ShaderStage(desc)
{
}

DX12ShaderStage::~DX12ShaderStage()
{
    release();
}

unsigned int DX12ShaderStage::compile(const ShaderMacroSet& macroSet, const string& code, string& errorString)
{
	ShaderStage::compile(macroSet, code, errorString);
	ComPtr<ID3DBlob> errorBlob = NULL;
	unsigned int compileFlag = D3DCOMPILE_ENABLE_STRICTNESS;
	if (code.find("#pragma debug") != string::npos) {
		cout << "#pragma debug\n";
		compileFlag |= D3DCOMPILE_SKIP_OPTIMIZATION |
			D3DCOMPILE_DEBUG | D3DCOMPILE_PREFER_FLOW_CONTROL;
	}
	const char* shdtmp = ".shdtmp/";
	if (!filesystem::exists(shdtmp))
		filesystem::create_directory(shdtmp);
	string rootPath = Engine::windowContext.workingPath;
	string shaderPath = rootPath + '/' + shdtmp + name.c_str() + "_" + getShaderFeatureNames(this->shaderFeature) + getShaderExtension(stageType);
	ofstream f = ofstream(shaderPath);
	if (f.fail()) {
		string processedCode = macroSet.getDefineCode() + code;
		if (FAILED(D3DCompile(processedCode.c_str(), processedCode.size() * sizeof(char), name.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main", getShaderVersion(stageType), compileFlag, 0, &dx12ShaderBlob, &errorBlob))) {
			errorString = (const char*)errorBlob->GetBufferPointer();
			return 0;
		}
	}
	else {
		f << macroSet.getDefineCode();
		f << code;
		f.close();
		if (FAILED(D3DCompileFromFile(filesystem::path(shaderPath).c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main", getShaderVersion(stageType), compileFlag, 0, &dx12ShaderBlob, &errorBlob))) {
			errorString = (const char*)errorBlob->GetBufferPointer();
			return 0;
		}
	}
	shaderId = (unsigned long long)dx12ShaderBlob->GetBufferPointer();

	if (FAILED(D3DReflect(dx12ShaderBlob->GetBufferPointer(), dx12ShaderBlob->GetBufferSize(),
		IID_PPV_ARGS(&dx12ShaderReflector)))) {
		if (!errorString.empty())
			errorString += '\n';
		errorString += "D3DReflect: reflection error";
	}

	properties.clear();

	D3D12_SHADER_DESC shaderDesc = { 0 };
	dx12ShaderReflector->GetDesc(&shaderDesc);

	for (int i = 0; i < shaderDesc.BoundResources; i++) {
		D3D12_SHADER_INPUT_BIND_DESC desc = { 0 };
		dx12ShaderReflector->GetResourceBindingDesc(i, &desc);
		ShaderPropertyName shaderPropertyName = desc.Name;
		ShaderProperty& shaderProperty = properties.insert(make_pair(
			shaderPropertyName, ShaderProperty(shaderPropertyName))).first->second;
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
			ID3D12ShaderReflectionConstantBuffer* constantBuffer = dx12ShaderReflector->GetConstantBufferByName(desc.Name);
			D3D12_SHADER_BUFFER_DESC bufDesc = { 0 };
			if (SUCCEEDED(constantBuffer->GetDesc(&bufDesc))) {
				shaderProperty.size = bufDesc.Size;
				if (shaderPropertyName == materialParameterBufferName) {
					for (int i = 0; i < bufDesc.Variables; i++) {
						ID3D12ShaderReflectionVariable* variable = constantBuffer->GetVariableByIndex(i);
						D3D12_SHADER_VARIABLE_DESC desc = { 0 };
						variable->GetDesc(&desc);
						ShaderPropertyName paramName = desc.Name;
						ShaderProperty& param = properties.insert(make_pair(
							paramName, ShaderProperty(paramName))).first->second;
						param.type = ShaderProperty::Parameter;
						param.offset = desc.StartOffset;
						param.size = desc.Size;
						param.meta = 0;
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

void DX12ShaderStage::resolveRootSignature(DX12RootSignatureDesc& desc)
{
	for (auto& prop : properties)
	{
		switch (prop.second.type)
		{
		case ShaderProperty::ConstantBuffer:
			desc.addBuffer(prop.second.offset);
			break;
		case ShaderProperty::TextureBuffer:
		case ShaderProperty::Texture:
			desc.addTexture(prop.second.offset);
			break;
		case ShaderProperty::Sampler:
			desc.addSampler(prop.second.offset);
			break;
		case ShaderProperty::Image:
			desc.addImage(prop.second.offset);
			break;
		default:
			break;
		}
		
	}
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

bool DX12ShaderProgram::init()
{
	if (!isValid())
		return false;
	if (!dirty)
		return true;
	ShaderProgram::init();
	if (const ShaderPropertyDesc* desc = getAttributeOffset(DX12ShaderStage::materialParameterBufferName))
		if (const ShaderProperty* prop = desc->getConstantBuffer())
			matInsBufSize = prop->size;
	dirty = false;
	return true;
}

unsigned int DX12ShaderProgram::bind()
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
	if (matInsBufSize != 0) {
		int alignedSize = GRS_UPPER(matInsBufSize, 256);
		DX12SubBuffer* buffer = dxContext.constantBufferPool.suballocate(alignedSize, 256);
		if (buffer == NULL) {
			throw runtime_error("MatInsBuf CreateResource failed");
		}
		return buffer;
	}
	return NULL;
}
