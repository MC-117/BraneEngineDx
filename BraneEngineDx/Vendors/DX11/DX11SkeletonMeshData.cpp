#include "DX11SkeletonMeshData.h"

#ifdef VENDOR_USE_DX11

DX11SkeletonMeshData::DX11SkeletonMeshData(DX11Context& context)
    : dxContext(context)
{
}

DX11SkeletonMeshData::~DX11SkeletonMeshData()
{
    release();
}

bool DX11SkeletonMeshData::isValid() const
{
	return true;
}

bool DX11SkeletonMeshData::isGenerated() const
{
    return dx11ElementBuffer != NULL;
}

void DX11SkeletonMeshData::init()
{
	D3D11_BUFFER_DESC bDesc = {};
	ZeroMemory(&bDesc, sizeof(D3D11_BUFFER_DESC));
	D3D11_SUBRESOURCE_DATA initData;

	if (dx11VertexBuffer == NULL) {
		bDesc.ByteWidth = vertices.size() * sizeof(Vector3f);
		bDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		initData.pSysMem = vertices.data()->data();
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
		bDesc.ByteWidth = elements.size() * sizeof(unsigned int);
		bDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		initData.pSysMem = elements.data();
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
			if (FAILED(dxContext.device->CreateShaderResourceView(dx11MorphVNBuffer.Get(), &srvDesc, &dx11MorphVNView)))
				throw runtime_error("Create Morph vertices and normals SRV failed");
		}

		/*if (dx11MorphWeightBuffer == NULL) {
			unsigned int len = morphMeshData.morphCount + 1;
			float* morphWeights = new float[len]();
			morphWeights[0] = morphMeshData.morphCount;

			bDesc.ByteWidth = len * sizeof(float);
			bDesc.Usage = D3D11_USAGE_DYNAMIC;
			bDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			initData.pSysMem = morphWeights;
			initData.SysMemPitch = 0;
			initData.SysMemSlicePitch = 0;

			if (FAILED(dxContext.device->CreateBuffer(&bDesc, &initData, &dx11MorphWeightBuffer)))
				throw runtime_error("DX11: Create morph weight buffer failed");

			delete[] morphWeights;

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = len;
			if (FAILED(dxContext.device->CreateShaderResourceView(dx11MorphWeightBuffer.Get(), &srvDesc, &dx11MorphWeightView)))
				throw runtime_error("Create Morph Weight SRV failed");
		}*/
	}
}

void DX11SkeletonMeshData::bindShape()
{
	if (dx11ElementBuffer != NULL && currentMeshData == this)
		return;

	D3D11_BUFFER_DESC bDesc = {};
	ZeroMemory(&bDesc, sizeof(D3D11_BUFFER_DESC));
	D3D11_SUBRESOURCE_DATA initData;

	if (dx11VertexBuffer == NULL) {
		bDesc.ByteWidth = vertices.size() * sizeof(Vector3f);
		bDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		initData.pSysMem = vertices.data()->data();
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
		bDesc.ByteWidth = elements.size() * sizeof(unsigned int);
		bDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		initData.pSysMem = elements.data();
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
			if (FAILED(dxContext.device->CreateShaderResourceView(dx11MorphVNBuffer.Get(), &srvDesc, &dx11MorphVNView)))
				throw runtime_error("Create Morph vertices and normals SRV failed");
		}

		/*if (dx11MorphWeightBuffer == NULL) {
			unsigned int len = morphMeshData.morphCount + 1;
			float* morphWeights = new float[len]();
			morphWeights[0] = morphMeshData.morphCount;

			bDesc.ByteWidth = len * sizeof(float);
			bDesc.Usage = D3D11_USAGE_DYNAMIC;
			bDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			initData.pSysMem = morphWeights;
			initData.SysMemPitch = 0;
			initData.SysMemSlicePitch = 0;

			if (FAILED(dxContext.device->CreateBuffer(&bDesc, &initData, &dx11MorphWeightBuffer)))
				throw runtime_error("DX11: Create morph weight buffer failed");

			delete[] morphWeights;

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = len;
			if (FAILED(dxContext.device->CreateShaderResourceView(dx11MorphWeightBuffer.Get(), &srvDesc, &dx11MorphWeightView)))
				throw runtime_error("Create Morph Weight SRV failed");
		}*/
	}

	ID3D11Buffer* buffers[] = {
		dx11VertexBuffer.Get(),
		dx11UVBuffer.Get(),
		dx11NormalBuffer.Get(),
		dx11BoneIndexBuffer.Get(),
		dx11BoneWeightBuffer.Get()
	};
	UINT strides[] = {
		sizeof(Vector3f),
		sizeof(Vector2f),
		sizeof(Vector3f),
		sizeof(Vector4u),
		sizeof(Vector4f)
	};
	UINT offsets[] = { 0, 0, 0, 0, 0 };

	dxContext.deviceContext->IASetVertexBuffers(1, 5, buffers, strides, offsets);
	dxContext.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dxContext.deviceContext->IASetIndexBuffer(dx11ElementBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	dxContext.deviceContext->IASetInputLayout(dxContext.skeletonMeshInputLayout.Get());

	if (dx11MorphVNView != NULL)
		dxContext.deviceContext->VSSetShaderResources(MORPHDATA_BIND_INDEX, 1, dx11MorphVNView.GetAddressOf());
	/*if (dx11MorphWeightView != NULL)
		dxContext.deviceContext->VSSetShaderResources(MORPHWEIGHT_BIND_INDEX, 1, dx11MorphWeightView.GetAddressOf());*/
	currentMeshData = this;
}

//void DX11SkeletonMeshData::updateMorphWeights(vector<float>& weights)
//{
//	if (dx11MorphWeightBuffer != NULL) {
//		D3D11_MAPPED_SUBRESOURCE mpd;
//		size_t size = weights.size() * sizeof(float);
//		float morphCount = morphMeshData.morphCount;
//		dxContext.deviceContext->Map(dx11MorphWeightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mpd);
//		memcpy_s((char*)mpd.pData, sizeof(float), &morphCount, sizeof(float));
//		memcpy_s((char*)mpd.pData + sizeof(float), size, weights.data(), size);
//		dxContext.deviceContext->Unmap(dx11MorphWeightBuffer.Get(), 0);
//	}
//}

void DX11SkeletonMeshData::release()
{
	if (dx11VertexBuffer) {
		dx11VertexBuffer.Reset();
	}
	if (dx11UVBuffer) {
		dx11UVBuffer.Reset();
	}
	if (dx11NormalBuffer) {
		dx11NormalBuffer.Reset();
	}
	if (dx11ElementBuffer) {
		dx11ElementBuffer.Reset();
	}
	if (dx11BoneIndexBuffer) {
		dx11BoneIndexBuffer.Reset();
	}
	if (dx11BoneWeightBuffer) {
		dx11BoneWeightBuffer.Reset();
	}
}

#endif // VENDOR_USE_DX11
