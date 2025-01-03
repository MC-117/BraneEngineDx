#pragma once
#include "../../Core/IMaterial.h"

#ifdef VENDOR_USE_DX11

#ifndef _DX11MATERIAL_H_
#define _DX11MATERIAL_H_

#include "DX11.h"
#include "DX11ShaderStage.h"
#include "DX11Texture2D.h"

class DX11Material : public IMaterial
{
public:
	DX11Context& dxContext;
	vector<ComPtr<ID3D11UnorderedAccessView>> dx11UAVs;
	ComPtr<ID3D11Buffer> matInsBuf = NULL;
	unsigned char* matInsBufHost = NULL;
	unsigned int matInsBufSize = 0;

	DX11Material(DX11Context& context, MaterialDesc& desc);
	virtual ~DX11Material();

	virtual void uploadAttribute(const Name& name, unsigned int size, void* data);
	virtual void uploadTexture(const Name& name, ComPtr<ID3D11ShaderResourceView> tex, ComPtr<ID3D11SamplerState> sample);
	virtual void uploadImage(const Name& name, ComPtr<ID3D11UnorderedAccessView> tex);

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
};

#endif // !_DX11MATERIAL_H_

#endif // VENDOR_USE_DX11
