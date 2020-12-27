#pragma once
#include "../../Core/MeshData.h"

#ifdef VENDOR_USE_DX11

#ifndef _DX11MESHDATA_H_
#define _DX11MESHDATA_H_

#include "DX11.h"

class DX11MeshData : public MeshData
{
public:
	static ID3D11InputLayout* dx11MeshDataInputLayout;
	
	const DX11Context& dxContext;
	ID3D11Buffer* dx11VertexBuffer = NULL;
	ID3D11Buffer* dx11UVBuffer = NULL;
	ID3D11Buffer* dx11NormalBuffer = NULL;
	ID3D11Buffer* dx11ElementBuffer = NULL;

	DX11MeshData(const DX11Context& context);
	virtual ~DX11MeshData();

	virtual bool isGenerated() const;
	virtual void bindShape();
	virtual void release();
};

#endif // !_DX11MESHDATA_H_

#endif // VENDOR_USE_DX11
