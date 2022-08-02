#pragma once
#include "../../Core/MeshData.h"

#ifdef VENDOR_USE_DX12

#include "DX12Context.h"

class DX12MeshData : public MeshData
{
public:
	DX12Context& dxContext;
	DX12Buffer* dx12VertexBuffer = NULL;
	DX12Buffer* dx12UVBuffer = NULL;
	DX12Buffer* dx12NormalBuffer = NULL;
	DX12Buffer* dx12ElementBuffer = NULL;

	DX12MeshData(DX12Context& context);
	virtual ~DX12MeshData();

	virtual bool isValid() const;
	virtual bool isGenerated() const;
	virtual void bindShape();
	virtual void release();
};

#endif // VENDOR_USE_DX12