#pragma once
#include "../../Core/SkeletonMeshData.h"

#ifdef VENDOR_USE_DX11

#ifndef _DX11SKELETONMESHDATA_H_
#define _DX11SKELETONMESHDATA_H_

#include "DX11.h"

class DX11SkeletonMeshData : public SkeletonMeshData
{
public:
	DX11Context& dxContext;
	ComPtr<ID3D11Buffer> dx11VertexBuffer = NULL;
	ComPtr<ID3D11Buffer> dx11UVBuffer = NULL;
	ComPtr<ID3D11Buffer> dx11NormalBuffer = NULL;
	ComPtr<ID3D11Buffer> dx11ElementBuffer = NULL;
	ComPtr<ID3D11Buffer> dx11BoneIndexBuffer = NULL;
	ComPtr<ID3D11Buffer> dx11BoneWeightBuffer = NULL;

	ComPtr<ID3D11Buffer> dx11MorphVNBuffer = NULL;
	ComPtr<ID3D11ShaderResourceView> dx11MorphVNView = NULL;
	/*ComPtr<ID3D11Buffer> dx11MorphWeightBuffer = NULL;
	ComPtr<ID3D11ShaderResourceView> dx11MorphWeightView = NULL;*/

	DX11SkeletonMeshData(DX11Context& context);
	virtual ~DX11SkeletonMeshData();

	virtual bool isValid() const;
	virtual bool isGenerated() const;
	virtual void bindShape();
	//virtual void updateMorphWeights(vector<float>& weights);
	virtual void release();
};

#endif // !_DX11SKELETONMESHDATA_H_

#endif // VENDOR_USE_DX11
