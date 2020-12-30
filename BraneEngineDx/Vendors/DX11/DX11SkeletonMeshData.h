#pragma once
#include "../../Core/SkeletonMeshData.h"

#ifdef VENDOR_USE_DX11

#ifndef _DX11SKELETONMESHDATA_H_
#define _DX11SKELETONMESHDATA_H_

#include "DX11.h"

class DX11SkeletonMeshData : public SkeletonMeshData
{
public:
	static ID3D11InputLayout* dx11SkeletonMeshDataInputLayout;

	DX11Context& dxContext;
	ID3D11Buffer* dx11VertexBuffer = NULL;
	ID3D11Buffer* dx11UVBuffer = NULL;
	ID3D11Buffer* dx11NormalBuffer = NULL;
	ID3D11Buffer* dx11ElementBuffer = NULL;
	ID3D11Buffer* dx11BoneIndexBuffer = NULL;
	ID3D11Buffer* dx11BoneWeightBuffer = NULL;

	ID3D11Buffer* dx11MorphVNBuffer = NULL;
	ID3D11ShaderResourceView* dx11MorphVNView = NULL;
	ID3D11Buffer* dx11MorphWeightBuffer = NULL;
	ID3D11ShaderResourceView* dx11MorphWeightView = NULL;

	DX11SkeletonMeshData(DX11Context& context);
	virtual ~DX11SkeletonMeshData();

	virtual bool isGenerated() const;
	virtual void bindShape();
	virtual void updateMorphWeights(vector<float>& weights);
	virtual void release();
};

#endif // !_DX11SKELETONMESHDATA_H_

#endif // VENDOR_USE_DX11
