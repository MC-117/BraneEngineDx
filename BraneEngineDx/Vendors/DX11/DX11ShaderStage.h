#pragma once
#include "../../Core/ShaderStage.h"

#ifdef VENDOR_USE_DX11

#ifndef _DX11SHADERSTAGE_H_
#define _DX11SHADERSTAGE_H_

#include "DX11.h"
#include "../../Core/ShaderStage.h"


class DX11ShaderStage : public ShaderStage
{
public:
	const DX11Context& dxContext;
	ID3DBlob* dx11ShaderBlob = NULL;
	ID3D11DeviceChild* dx11Shader = NULL;

	DX11ShaderStage(const DX11Context& context, const ShaderStageDesc& desc);
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
	const DX11Context& dxContext;

	DX11ShaderProgram(const DX11Context& context);
	virtual ~DX11ShaderProgram();

	virtual unsigned int bind();
	virtual bool dispatchCompute(unsigned int dimX, unsigned int dimY, unsigned int dimZ);
	virtual void memoryBarrier(unsigned int bitEnum);
};

#endif // !_DX11SHADERSTAGE_H_

#endif // VENDOR_USE_DX11
