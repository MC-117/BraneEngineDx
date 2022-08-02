#include "DX11MeshData.h"

#ifdef VENDOR_USE_DX11

DX11MeshData::DX11MeshData(DX11Context& context) : dxContext(context)
{
}

DX11MeshData::~DX11MeshData()
{
	release();
}

bool DX11MeshData::isValid() const
{
	return true;
}

bool DX11MeshData::isGenerated() const
{
	return dx11ElementBuffer != NULL;
}

void DX11MeshData::bindShape()
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

	ID3D11Buffer* buffers[] = {
		dx11VertexBuffer.Get(),
		dx11UVBuffer.Get(),
		dx11NormalBuffer.Get()
	};
	UINT strides[] = {
		sizeof(Vector3f),
		sizeof(Vector2f),
		sizeof(Vector3f)
	};
	UINT offsets[] = { 0, 0, 0 };

	dxContext.deviceContext->IASetVertexBuffers(1, 3, buffers, strides, offsets);
	dxContext.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dxContext.deviceContext->IASetIndexBuffer(dx11ElementBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	dxContext.deviceContext->IASetInputLayout(dxContext.meshInputLayout.Get());
	currentMeshData = this;
}

void DX11MeshData::release()
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
}

#endif // VENDOR_USE_DX11
