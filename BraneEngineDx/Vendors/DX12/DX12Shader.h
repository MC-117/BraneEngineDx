#pragma once
#include "../../Core/ShaderStage.h"

#ifdef VENDOR_USE_DX12

#include "DX12Context.h"

class DX12ShaderStage : public ShaderStage
{
public:
	static const char* MatInsBufName;
	static const char* DrawInfoBufName;
	static ShaderPropertyName materialParameterBufferName;
	static ShaderPropertyName drawInfoBufferName;
	DX12Context& dxContext;
	ComPtr<ID3DBlob> dx12ShaderBlob = NULL;
	ComPtr<ID3D12ShaderReflection> dx12ShaderReflector = NULL;

	DX12ShaderStage(DX12Context& context, const ShaderStageDesc& desc);
	virtual ~DX12ShaderStage();

	virtual unsigned int compile(const string& code, string& errorString);
	virtual void resolveRootSignature(DX12RootSignatureDesc& desc);
	virtual void release();

	static const char* getShaderVersion(ShaderStageType type);
};

struct DrawInfo
{
	unsigned int baseVertex;
	unsigned int baseInstance;
	unsigned int passID;
	unsigned int passNum;
	unsigned int padding[60];
};

class DX12ShaderProgram : public ShaderProgram
{
public:
	static unsigned int nextProgramID;
	static DX12ShaderProgram* currentDx12Program;
	DX12Context& dxContext;
	ComPtr<ID3D12Resource> matInsBuf;
	unsigned char* matInsBufHost = NULL;
	unsigned char* matInsBufGPUHost = NULL;
	unsigned int matInsBufSize = 0;
	DrawInfo drawInfo;
	ComPtr<ID3D12Resource> drawInfoBuf;
	unordered_map<string, AttributeDesc> attributes;
	DX12RootSignatureDesc rootSignatureDesc;

	DX12ShaderProgram(DX12Context& context);
	virtual ~DX12ShaderProgram();

	virtual bool init();

	virtual unsigned int bind();
	virtual bool dispatchCompute(unsigned int dimX, unsigned int dimY, unsigned int dimZ);
	virtual void memoryBarrier(unsigned int bitEnum);
	virtual void uploadDrawInfo();
	virtual void uploadData();

	virtual void uploadAttribute(const string& name, unsigned int size, void* data);
	virtual void uploadTexture(const string& name, DX12Descriptor tex, DX12Descriptor sample);
	virtual void uploadImage(const string& name, DX12Descriptor tex);

	void bindCBToStage(unsigned int index, ComPtr<ID3D12Resource> buffer);

	DX12SubBuffer* allocateMatInsBuf();
};

#endif // VENDOR_USE_DX12