#include "DX12MeshData.h"

DX12MeshData::DX12MeshData(DX12Context& context) : dxContext(context)
{
}

DX12MeshData::~DX12MeshData()
{
	release();
}

bool DX12MeshData::isValid() const
{
	return true;
}

bool DX12MeshData::isGenerated() const
{
	return dx12ElementBuffer != NULL;
}

void DX12MeshData::init()
{
	D3D12_RESOURCE_DESC bDesc = {};
	ZeroMemory(&bDesc, sizeof(D3D12_RESOURCE_DESC));
	bDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	bDesc.Format = DXGI_FORMAT_UNKNOWN;
	bDesc.MipLevels = 1;
	bDesc.SampleDesc.Count = 1;
	bDesc.SampleDesc.Quality = 0;
	bDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bDesc.Alignment = 0;
	bDesc.Height = 1;
	bDesc.DepthOrArraySize = 1;
	D3D12_SUBRESOURCE_DATA initData;
	initData.RowPitch = 0;
	initData.SlicePitch = 0;
	DX12GraphicMeshData meshData;
	meshData.startSlot = 1;
	meshData.slotCount = 3;
	meshData.layoutType = DX12InputLayoutType::Mesh;

	if (dx12VertexBuffer == NULL) {
		bDesc.Width = vertices.size() * sizeof(Vector3f);
		//bDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		initData.pData = vertices.data()->data();
		dx12VertexBuffer = dxContext.meshBufferPool.allocate(bDesc.Width);
		dx12VertexBuffer->CreateResource(bDesc, D3D12_RESOURCE_STATE_COPY_DEST, sizeof(Vector3f));
		dxContext.resourceUploader.uploadResource(dx12VertexBuffer, 0, 1,
			&initData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	if (dx12UVBuffer == NULL) {
		bDesc.Width = uvs.size() * sizeof(Vector2f);
		//bDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		initData.pData = uvs.data()->data();
		dx12UVBuffer = dxContext.meshBufferPool.allocate(bDesc.Width);
		dx12UVBuffer->CreateResource(bDesc, D3D12_RESOURCE_STATE_COPY_DEST, sizeof(Vector2f));
		dxContext.resourceUploader.uploadResource(dx12UVBuffer, 0, 1,
			&initData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	if (dx12NormalBuffer == NULL) {
		bDesc.Width = normals.size() * sizeof(Vector3f);
		//bDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		initData.pData = normals.data()->data();
		dx12NormalBuffer = dxContext.meshBufferPool.allocate(bDesc.Width);
		dx12NormalBuffer->CreateResource(bDesc, D3D12_RESOURCE_STATE_COPY_DEST, sizeof(Vector3f));
		dxContext.resourceUploader.uploadResource(dx12NormalBuffer, 0, 1,
			&initData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	if (dx12ElementBuffer == NULL) {
		bDesc.Width = elements.size() * sizeof(unsigned int);
		//bDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
		initData.pData = elements.data();
		dx12ElementBuffer = dxContext.meshBufferPool.allocate(bDesc.Width);
		dx12ElementBuffer->CreateResource(bDesc, D3D12_RESOURCE_STATE_COPY_DEST, sizeof(unsigned int));
		dxContext.resourceUploader.uploadResource(dx12ElementBuffer, 0, 1,
			&initData, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	}
}

void DX12MeshData::bindShape()
{
	D3D12_RESOURCE_DESC bDesc = {};
	ZeroMemory(&bDesc, sizeof(D3D12_RESOURCE_DESC));
	bDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	bDesc.Format = DXGI_FORMAT_UNKNOWN;
	bDesc.MipLevels = 1;
	bDesc.SampleDesc.Count = 1;
	bDesc.SampleDesc.Quality = 0;
	bDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bDesc.Alignment = 0;
	bDesc.Height = 1;
	bDesc.DepthOrArraySize = 1;
	D3D12_SUBRESOURCE_DATA initData;
	initData.RowPitch = 0;
	initData.SlicePitch = 0;
	DX12GraphicMeshData meshData;
	meshData.startSlot = 1;
	meshData.slotCount = 3;
	meshData.layoutType = DX12InputLayoutType::Mesh;

	if (dx12VertexBuffer == NULL) {
		bDesc.Width = vertices.size() * sizeof(Vector3f);
		//bDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		initData.pData = vertices.data()->data();
		dx12VertexBuffer = dxContext.meshBufferPool.allocate(bDesc.Width);
		dx12VertexBuffer->CreateResource(bDesc, D3D12_RESOURCE_STATE_COPY_DEST, sizeof(Vector3f));
		dxContext.resourceUploader.uploadResource(dx12VertexBuffer, 0, 1,
			&initData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	if (dx12UVBuffer == NULL) {
		bDesc.Width = uvs.size() * sizeof(Vector2f);
		//bDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		initData.pData = uvs.data()->data();
		dx12UVBuffer = dxContext.meshBufferPool.allocate(bDesc.Width);
		dx12UVBuffer->CreateResource(bDesc, D3D12_RESOURCE_STATE_COPY_DEST, sizeof(Vector2f));
		dxContext.resourceUploader.uploadResource(dx12UVBuffer, 0, 1,
			&initData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	if (dx12NormalBuffer == NULL) {
		bDesc.Width = normals.size() * sizeof(Vector3f);
		//bDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		initData.pData = normals.data()->data();
		dx12NormalBuffer = dxContext.meshBufferPool.allocate(bDesc.Width);
		dx12NormalBuffer->CreateResource(bDesc, D3D12_RESOURCE_STATE_COPY_DEST, sizeof(Vector3f));
		dxContext.resourceUploader.uploadResource(dx12NormalBuffer, 0, 1,
			&initData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	if (dx12ElementBuffer == NULL) {
		bDesc.Width = elements.size() * sizeof(unsigned int);
		//bDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
		initData.pData = elements.data();
		dx12ElementBuffer = dxContext.meshBufferPool.allocate(bDesc.Width);
		dx12ElementBuffer->CreateResource(bDesc, D3D12_RESOURCE_STATE_COPY_DEST, sizeof(unsigned int));
		dxContext.resourceUploader.uploadResource(dx12ElementBuffer, 0, 1,
			&initData, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	}

	meshData.vertex = dx12VertexBuffer;
	meshData.uv = dx12UVBuffer;
	meshData.normal = dx12NormalBuffer;
	meshData.element = dx12ElementBuffer;

	dxContext.graphicContext.setMeshData(meshData);

	currentMeshData = this;
}

void DX12MeshData::release()
{
	if (dx12VertexBuffer != NULL) {
		dx12VertexBuffer->release();
		dx12VertexBuffer = NULL;
	}
	if (dx12UVBuffer != NULL) {
		dx12UVBuffer->release();
		dx12UVBuffer = NULL;
	}
	if (dx12NormalBuffer != NULL) {
		dx12NormalBuffer->release();
		dx12NormalBuffer = NULL;
	}
	if (dx12ElementBuffer != NULL) {
		dx12ElementBuffer->release();
		dx12ElementBuffer = NULL;
	}
}
