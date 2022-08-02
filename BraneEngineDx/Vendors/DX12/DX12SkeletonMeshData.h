#pragma once
#include "../../Core/SkeletonMeshData.h"

#ifdef VENDOR_USE_DX12

#ifndef _DX12SKELETONMESHDATA_H_
#define _DX12SKELETONMESHDATA_H_

#include "DX12GPUBuffer.h"

class DX12SkeletonMeshData : public SkeletonMeshData
{
public:
	DX12Context& dxContext;
	DX12Buffer* dx12VertexBuffer = NULL;
	DX12Buffer* dx12UVBuffer = NULL;
	DX12Buffer* dx12NormalBuffer = NULL;
	DX12Buffer* dx12ElementBuffer = NULL;
	DX12Buffer* dx12BoneIndexBuffer = NULL;
	DX12Buffer* dx12BoneWeightBuffer = NULL;

	DX12Buffer* dx12MorphVNBuffer = NULL;
	DX12ResourceView dx12MorphVNView;

	DX12SkeletonMeshData(DX12Context& context);
	virtual ~DX12SkeletonMeshData();

	virtual bool isValid() const;
	virtual bool isGenerated() const;
	virtual void bindShape();
	virtual void release();
};

#endif // !_DX12SKELETONMESHDATA_H_

#endif // VENDOR_USE_DX12