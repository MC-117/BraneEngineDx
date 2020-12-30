#pragma once
#include "../../Core/ShaderStage.h"

#ifdef VENDOR_USE_DX11

#ifndef _DX11SHADERSTAGE_H_
#define _DX11SHADERSTAGE_H_

#include "DX11.h"

class DX11ShaderStage : public ShaderStage
{
public:
	const string MatInsBufName = "MatInsBuf";
	DX11Context& dxContext;
	ID3DBlob* dx11ShaderBlob = NULL;
	ID3D11DeviceChild* dx11Shader = NULL;
	ID3D11ShaderReflection* dx11ShaderReflector = NULL;
	ID3D11ShaderReflectionConstantBuffer* dx11MatInsBufReflector = NULL;

	DX11ShaderStage(DX11Context& context, const ShaderStageDesc& desc);
	virtual ~DX11ShaderStage();

	virtual unsigned int compile(const string& code, string& errorString);
	virtual void release();

	static const char* getShaderVersion(ShaderStageType type);
	static HRESULT createShader(ID3D11Device* device, ShaderStageType type, ID3DBlob* pShaderBlob, ID3D11DeviceChild** ppShader);
};

class DX11ShaderProgram : public ShaderProgram
{
public:
	static unsigned int nextProgramID;
	static DX11ShaderProgram* currentDx11Program;
	DX11Context& dxContext;
	ID3D11ShaderReflectionConstantBuffer* dx11MatInsBufReflector = NULL;
	ID3D11Buffer* matInsBuf = NULL;
	unsigned char* matInsBufHost = NULL;
	unsigned int matInsBufSize = 0;
	unordered_map<string, AttributeDesc> attributes;

	DX11ShaderProgram(DX11Context& context);
	virtual ~DX11ShaderProgram();

	virtual unsigned int bind();
	// AttributeDesc::meta represent shader stage type
	virtual AttributeDesc getAttributeOffset(const string& name);
	virtual bool dispatchCompute(unsigned int dimX, unsigned int dimY, unsigned int dimZ);
	virtual void memoryBarrier(unsigned int bitEnum);
	virtual void uploadData();

	virtual void uploadAttribute(const string& name, unsigned int size, void* data);
	virtual void uploadTexture(const string& name, ID3D11ShaderResourceView* tex, ID3D11SamplerState* sample);
	virtual void uploadImage(const string& name, ID3D11UnorderedAccessView* tex);
};

#endif // !_DX11SHADERSTAGE_H_

#endif // VENDOR_USE_DX11
