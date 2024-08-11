#pragma once
#include "../../Core/IMaterial.h"

#ifdef VENDOR_USE_DX12

#include "DX12Context.h"
#include "DX12Shader.h"
#include "DX12Texture2D.h"

class DX12Material : public IMaterial
{
public:
	DX12Context& dxContext;
	vector<DX12Descriptor> dx12UAVs;
	DX12RootSignature rootSignature;
	DX12SubBuffer* drawInfoBuf = NULL;
	DrawInfo drawInfo;
	DX12SubBuffer* matInsBuf = NULL;
	void* matInsBufHost = NULL;

	DX12Material(DX12Context& context, MaterialDesc& desc);
	virtual ~DX12Material();

	void uploadAttribute(DX12ShaderProgram* program, const Name& name, unsigned int size, void* data);

	virtual void preprocess();
	virtual void processBaseData();
	virtual void processScalarData();
	virtual void processCountData();
	virtual void processColorData();
	virtual void processMatrixData();
	virtual void processTextureData();
	virtual void processImageData();
	virtual void postprocess();

	void release();
protected:
	Name lastShaderName = "__Null";
};

#endif // VENDOR_USE_DX12
