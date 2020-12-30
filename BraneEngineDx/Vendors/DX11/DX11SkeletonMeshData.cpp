#include "DX11SkeletonMeshData.h"

#ifdef VENDOR_USE_DX11

ID3D11InputLayout* DX11SkeletonMeshData::dx11SkeletonMeshDataInputLayout = NULL;

DX11SkeletonMeshData::DX11SkeletonMeshData(DX11Context& context)
    : dxContext(context)
{
}

DX11SkeletonMeshData::~DX11SkeletonMeshData()
{
    release();
}

bool DX11SkeletonMeshData::isGenerated() const
{
    return dx11ElementBuffer != NULL;
}

void DX11SkeletonMeshData::bindShape()
{
	if (dx11ElementBuffer != NULL && currentMeshData == this)
		return;

	if (dx11SkeletonMeshDataInputLayout == NULL) {
		const char* signatureShader = "void main(float3 pos : POSITION, float2 uv : TEXCORD, float3 normal : NORMAL, uint4 bondId : BONEINDEX, float4 weight : BONEWEIGHT) { }";
		const size_t len = strlen(signatureShader);
		ID3DBlob* sigBlob;
		ID3DBlob* errorBlob;
		if (FAILED(D3DCompile(signatureShader, len, "SignatureShader", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &sigBlob, &errorBlob))) {
			throw runtime_error((const char*)errorBlob->GetBufferPointer());
		}
		const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[5] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 4, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		if (FAILED(dxContext.device->CreateInputLayout(inputLayoutDesc, 5, sigBlob->GetBufferPointer(),
			sigBlob->GetBufferSize(), &dx11SkeletonMeshDataInputLayout))) {
			throw runtime_error("DX11: Create skeleton mesh input layout failed");
		}
		sigBlob->Release();
		errorBlob->Release();
	}

	D3D11_BUFFER_DESC bDesc = {};
	ZeroMemory(&bDesc, sizeof(D3D11_BUFFER_DESC));
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = vertices.data()->data();

	if (dx11VertexBuffer == NULL) {
		bDesc.ByteWidth = vertices.size() * sizeof(Vector3f);
		bDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		if (FAILED(dxContext.device->CreateBuffer(&bDesc, &initData, &dx11VertexBuffer)))
			throw runtime_error("DX11: Create vertex buffer failed");
	}

	if (dx11UVBuffer == NULL) {
		bDesc.ByteWidth = uvs.size() * sizeof(Vector2f);
		bDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		initData.pSysMem = uvs.data()->data();
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		if (FAILED(dxContext.device->CreateBuffer(&bDesc, &initData, &dx11UVBuffer)))
			throw runtime_error("DX11: Create uv buffer failed");
	}

	if (dx11NormalBuffer == NULL) {
		bDesc.ByteWidth = normals.size() * sizeof(Vector3f);
		bDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		initData.pSysMem = normals.data()->data();
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		if (FAILED(dxContext.device->CreateBuffer(&bDesc, &initData, &dx11NormalBuffer)))
			throw runtime_error("DX11: Create normal buffer failed");
	}

	if (dx11ElementBuffer == NULL) {
		bDesc.ByteWidth = elements.size() * sizeof(Vector3u);
		bDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		initData.pSysMem = elements.data()->data();
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		if (FAILED(dxContext.device->CreateBuffer(&bDesc, &initData, &dx11ElementBuffer)))
			throw runtime_error("DX11: Create element buffer failed");
	}

	if (dx11BoneIndexBuffer == NULL) {
		bDesc.ByteWidth = boneIndexes.size() * sizeof(Vector4u);
		bDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		initData.pSysMem = boneIndexes.data()->data();
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		if (FAILED(dxContext.device->CreateBuffer(&bDesc, &initData, &dx11BoneIndexBuffer)))
			throw runtime_error("DX11: Create bone index buffer failed");
	}

	if (dx11BoneWeightBuffer == NULL) {
		bDesc.ByteWidth = weights.size() * sizeof(Vector4f);
		bDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		initData.pSysMem = weights.data()->data();
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		if (FAILED(dxContext.device->CreateBuffer(&bDesc, &initData, &dx11BoneWeightBuffer)))
			throw runtime_error("DX11: Create bone weight buffer failed");
	}

	if (morphMeshData.morphCount != 0 && morphMeshData.vertexCount != 0) {
		if (dx11MorphVNBuffer == NULL) {
			bDesc.ByteWidth = morphMeshData.verticesAndNormals.size() * sizeof(Vector4f);
			bDesc.Usage = D3D11_USAGE_IMMUTABLE;
			bDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

			initData.pSysMem = morphMeshData.verticesAndNormals.data()->data();
			initData.SysMemPitch = 0;
			initData.SysMemSlicePitch = 0;

			if (FAILED(dxContext.device->CreateBuffer(&bDesc, &initData, &dx11MorphVNBuffer)))
				throw runtime_error("DX11: Create morph mesh buffer failed");

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = morphMeshData.verticesAndNormals.size();
			if (FAILED(dxContext.device->CreateShaderResourceView(dx11MorphVNBuffer, &srvDesc, &dx11MorphVNView)))
				throw runtime_error("Create Morph vertices and normals SRV failed");
		}

		if (dx11MorphWeightBuffer == NULL) {
			bDesc.ByteWidth = morphMeshData.morphCount * sizeof(float);
			bDesc.Usage = D3D11_USAGE_DYNAMIC;
			bDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			initData.pSysMem = new float[morphMeshData.morphCount]();
			initData.SysMemPitch = 0;
			initData.SysMemSlicePitch = 0;

			if (FAILED(dxContext.device->CreateBuffer(&bDesc, &initData, &dx11MorphWeightBuffer)))
				throw runtime_error("DX11: Create morph weight buffer failed");

			delete initData.pSysMem;

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = morphMeshData.morphCount;
			if (FAILED(dxContext.device->CreateShaderResourceView(dx11MorphWeightBuffer, &srvDesc, &dx11MorphVNView)))
				throw runtime_error("Create Morph Weight SRV failed");
		}
	}

	ID3D11Buffer* buffers[] = {
		dx11VertexBuffer,
		dx11UVBuffer,
		dx11NormalBuffer,
		dx11BoneIndexBuffer,
		dx11BoneWeightBuffer
	};
	UINT strides[] = {
		sizeof(Vector3f),
		sizeof(Vector2f),
		sizeof(Vector3f),
		sizeof(Vector4u),
		sizeof(Vector4f)
	};
	UINT offsets[] = { 0, 0, 0, 0, 0 };

	dxContext.deviceContext->IASetVertexBuffers(0, 4, buffers, strides, offsets);
	dxContext.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dxContext.deviceContext->IASetIndexBuffer(dx11ElementBuffer, DXGI_FORMAT_R32_UINT, 0);
	dxContext.deviceContext->IASetInputLayout(dx11SkeletonMeshDataInputLayout);

	if (dx11MorphVNView != NULL)
		dxContext.deviceContext->VSSetShaderResources(MORPHDATA_BIND_INDEX, 1, &dx11MorphVNView);
	if (dx11MorphWeightView != NULL)
		dxContext.deviceContext->VSSetShaderResources(MORPHWEIGHT_BIND_INDEX, 1, &dx11MorphWeightView);
	currentMeshData = this;
}

void DX11SkeletonMeshData::updateMorphWeights(vector<float>& weights)
{
	if (dx11MorphWeightBuffer != NULL) {
		D3D11_MAPPED_SUBRESOURCE mpd;
		size_t size = weights.size() * sizeof(float);
		dxContext.deviceContext->Map(dx11MorphWeightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mpd);
		memcpy_s((char*)mpd.pData, size, weights.data(), size);
		dxContext.deviceContext->Unmap(dx11MorphWeightBuffer, 0);
	}
}

void DX11SkeletonMeshData::release()
{
	if (dx11VertexBuffer) {
		dx11VertexBuffer->Release();
		dx11VertexBuffer = NULL;
	}
	if (dx11UVBuffer) {
		dx11UVBuffer->Release();
		dx11UVBuffer = NULL;
	}
	if (dx11NormalBuffer) {
		dx11NormalBuffer->Release();
		dx11NormalBuffer = NULL;
	}
	if (dx11ElementBuffer) {
		dx11ElementBuffer->Release();
		dx11ElementBuffer = NULL;
	}
	if (dx11BoneIndexBuffer) {
		dx11BoneIndexBuffer->Release();
		dx11BoneIndexBuffer = NULL;
	}
	if (dx11BoneWeightBuffer) {
		dx11BoneWeightBuffer->Release();
		dx11BoneWeightBuffer = NULL;
	}
}

#endif // VENDOR_USE_DX11
