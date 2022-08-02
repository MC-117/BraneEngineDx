#pragma once
#include "../../Core/MeshData.h"

#ifdef VENDOR_USE_DX11

#ifndef _DX11MESHDATA_H_
#define _DX11MESHDATA_H_

#include "DX11.h"

class DX11MeshData : public MeshData
{
public:
	DX11Context& dxContext;
	ComPtr<ID3D11Buffer> dx11VertexBuffer = NULL;
	ComPtr<ID3D11Buffer> dx11UVBuffer = NULL;
	ComPtr<ID3D11Buffer> dx11NormalBuffer = NULL;
	ComPtr<ID3D11Buffer> dx11ElementBuffer = NULL;

	DX11MeshData(DX11Context& context);
	virtual ~DX11MeshData();

	virtual bool isValid() const;
	virtual bool isGenerated() const;
	virtual void bindShape();
	virtual void release();
};

#endif // !_DX11MESHDATA_H_

#endif // VENDOR_USE_DX11
