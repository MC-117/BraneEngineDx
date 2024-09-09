#pragma once
#include "../../Core/ShaderStage.h"

#ifdef VENDOR_USE_DX11

#ifndef _DX11SHADERSTAGE_H_
#define _DX11SHADERSTAGE_H_

#include "DX11.h"

class DX11ShaderStage : public ShaderStage
{
public:
	static const char* MatInsBufName;
	static const char* DrawInfoBufName;
	static ShaderPropertyName materialParameterBufferName;
	static ShaderPropertyName drawInfoBufferName;
	DX11Context& dxContext;
	ComPtr<ID3DBlob> dx11ShaderBlob = NULL;
	ComPtr<ID3D11DeviceChild> dx11Shader = NULL;
	ComPtr<ID3D11ShaderReflection> dx11ShaderReflector = NULL;

	DX11ShaderStage(DX11Context& context, const ShaderStageDesc& desc);
	virtual ~DX11ShaderStage();

	virtual unsigned int compile(const ShaderMacroSet& macroSet, const string& code, string& errorString);
	virtual void release();

	static const char* getShaderVersion(ShaderStageType type);
	static HRESULT createShader(ID3D11Device* device, ShaderStageType type, ID3DBlob* pShaderBlob, ID3D11DeviceChild** ppShader);
};

struct DrawInfo
{
	unsigned int passID;
	unsigned int passNum;
	unsigned int materialID;
	float gameTime;

	bool operator==(const DrawInfo& i) const;
	bool operator!=(const DrawInfo& i) const;
};

class DX11ShaderProgram : public ShaderProgram
{
public:
	static unsigned int nextProgramID;
	static DX11ShaderProgram* currentDx11Program;
	DX11Context& dxContext;
	DrawInfo drawInfo;
	ComPtr<ID3D11Buffer> drawInfoBuf = NULL;

	DX11ShaderProgram(DX11Context& context);
	virtual ~DX11ShaderProgram();

	virtual bool init();

	virtual unsigned int bind();
	virtual bool dispatchCompute(unsigned int dimX, unsigned int dimY, unsigned int dimZ);
	virtual void memoryBarrier(unsigned int bitEnum);
	virtual void uploadDrawInfo();

	bool unbindBuffer(ComPtr<ID3D11DeviceContext> deviceContext, const ShaderPropertyName& name) const;
	bool bindCBV(ComPtr<ID3D11DeviceContext> deviceContext, const ShaderPropertyName& name, ComPtr<ID3D11Buffer> buffer) const;
	bool bindSRV(ComPtr<ID3D11DeviceContext> deviceContext, const ShaderPropertyName& name, ComPtr<ID3D11ShaderResourceView> srv) const;
	bool bindSRVWithSampler(ComPtr<ID3D11DeviceContext> deviceContext, const ShaderPropertyName& name, ComPtr<ID3D11ShaderResourceView> srv, ComPtr<ID3D11SamplerState> sampler) const;
	bool bindUAV(ComPtr<ID3D11DeviceContext> deviceContext, const ShaderPropertyName& name, ComPtr<ID3D11UnorderedAccessView> uav) const;
	
	void bindCBToStage(const ShaderPropertyName& name, ComPtr<ID3D11Buffer> buffer);
};

#endif // !_DX11SHADERSTAGE_H_

#endif // VENDOR_USE_DX11
