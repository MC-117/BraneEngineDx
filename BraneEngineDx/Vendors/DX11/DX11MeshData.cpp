#include "DX11MeshData.h"

#ifdef VENDOR_USE_DX11

ID3D11InputLayout* DX11MeshData::dx11MeshDataInputLayout = NULL;

DX11MeshData::DX11MeshData(DX11Context& context) : dxContext(context)
{
}

DX11MeshData::~DX11MeshData()
{
	release();
}

bool DX11MeshData::isGenerated() const
{
	return dx11ElementBuffer != NULL;
}

void DX11MeshData::bindShape()
{
	if (dx11ElementBuffer != NULL && currentMeshData == this)
		return;

	if (dx11MeshDataInputLayout == NULL) {
		const char* signatureShader = "void main(float3 pos : POSITION, float2 uv : TEXCOORD, float3 normal : NORMAL) { }";
		const size_t len = strlen(signatureShader);
		ID3DBlob* sigBlob;
		ID3DBlob* errorBlob;
		if (FAILED(D3DCompile(signatureShader, len, "SignatureShader", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
				"main", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &sigBlob, &errorBlob))) {
			throw runtime_error((const char*)errorBlob->GetBufferPointer());
		}
		const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[3] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		if (FAILED(dxContext.device->CreateInputLayout(inputLayoutDesc, 3, sigBlob->GetBufferPointer(),
				sigBlob->GetBufferSize(), &dx11MeshDataInputLayout))) {
			throw runtime_error("DX11: Create mesh input layout failed");
		}
		sigBlob->Release();
		if (errorBlob != NULL)
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

	ID3D11Buffer* buffers[] = {
		dx11VertexBuffer,
		dx11UVBuffer,
		dx11NormalBuffer
	};
	UINT strides[] = {
		sizeof(Vector3f),
		sizeof(Vector2f),
		sizeof(Vector3f)
	};
	UINT offsets[] = { 0, 0, 0 };

	dxContext.deviceContext->IASetVertexBuffers(0, 3, buffers, strides, offsets);
	dxContext.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dxContext.deviceContext->IASetIndexBuffer(dx11ElementBuffer, DXGI_FORMAT_R32_UINT, 0);
	dxContext.deviceContext->IASetInputLayout(dx11MeshDataInputLayout);
	currentMeshData = this;
}

void DX11MeshData::release()
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
}

#endif // VENDOR_USE_DX11
