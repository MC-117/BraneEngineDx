#include "DX12SkeletonMeshData.h"

DX12SkeletonMeshData::DX12SkeletonMeshData(DX12Context& context)
    : dxContext(context)
{
}

DX12SkeletonMeshData::~DX12SkeletonMeshData()
{
	release();
}

bool DX12SkeletonMeshData::isValid() const
{
	return true;
}

bool DX12SkeletonMeshData::isGenerated() const
{
	return dx12ElementBuffer != NULL;
}

void DX12SkeletonMeshData::init()
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
	meshData.slotCount = 5;
	meshData.layoutType = DX12InputLayoutType::SkeletonMesh;

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
		dx12NormalBuffer->CreateResource(bDesc, D3D12_RESOURCE_STATE_COPY_DEST, sizeof(Vector2f));
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

	if (dx12BoneIndexBuffer == NULL) {
		bDesc.Width = boneIndexes.size() * sizeof(Vector4u);
		//bDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
		initData.pData = boneIndexes.data()->data();
		dx12BoneIndexBuffer = dxContext.meshBufferPool.allocate(bDesc.Width);
		dx12BoneIndexBuffer->CreateResource(bDesc, D3D12_RESOURCE_STATE_COPY_DEST, sizeof(Vector4u));
		dxContext.resourceUploader.uploadResource(dx12BoneIndexBuffer, 0, 1,
			&initData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	if (dx12BoneWeightBuffer == NULL) {
		bDesc.Width = weights.size() * sizeof(Vector4f);
		//bDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		initData.pData = weights.data()->data();
		dx12BoneWeightBuffer = dxContext.meshBufferPool.allocate(bDesc.Width);
		dx12BoneWeightBuffer->CreateResource(bDesc, D3D12_RESOURCE_STATE_COPY_DEST, sizeof(Vector4f));
		dxContext.resourceUploader.uploadResource(dx12BoneWeightBuffer, 0, 1,
			&initData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	if (morphMeshData.morphCount != 0 && morphMeshData.vertexCount != 0) {
		if (dx12MorphVNBuffer == NULL) {
			bDesc.Width = morphMeshData.verticesAndNormals.size() * sizeof(Vector4f);
			//bDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			initData.pData = morphMeshData.verticesAndNormals.data()->data();
			dx12MorphVNBuffer = dxContext.meshBufferPool.allocate(bDesc.Width);
			dx12MorphVNBuffer->CreateResource(bDesc, D3D12_RESOURCE_STATE_COPY_DEST, sizeof(Vector4f));
			dxContext.resourceUploader.uploadResource(dx12MorphVNBuffer, 0, 1,
				&initData, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			srvDesc.Buffer.NumElements = morphMeshData.verticesAndNormals.size();
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			dx12MorphVNView.init(*dx12MorphVNBuffer, srvDesc);

			if (!dx12MorphVNView.isValid())
				throw runtime_error("CreateShaderResourceView failed");
		}
	}
}

void DX12SkeletonMeshData::bindShape()
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
	meshData.slotCount = 5;
	meshData.layoutType = DX12InputLayoutType::SkeletonMesh;

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
		dx12NormalBuffer->CreateResource(bDesc, D3D12_RESOURCE_STATE_COPY_DEST, sizeof(Vector2f));
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

	if (dx12BoneIndexBuffer == NULL) {
		bDesc.Width = boneIndexes.size() * sizeof(Vector4u);
		//bDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
		initData.pData = boneIndexes.data()->data();
		dx12BoneIndexBuffer = dxContext.meshBufferPool.allocate(bDesc.Width);
		dx12BoneIndexBuffer->CreateResource(bDesc, D3D12_RESOURCE_STATE_COPY_DEST, sizeof(Vector4u));
		dxContext.resourceUploader.uploadResource(dx12BoneIndexBuffer, 0, 1,
			&initData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	if (dx12BoneWeightBuffer == NULL) {
		bDesc.Width = weights.size() * sizeof(Vector4f);
		//bDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		initData.pData = weights.data()->data();
		dx12BoneWeightBuffer = dxContext.meshBufferPool.allocate(bDesc.Width);
		dx12BoneWeightBuffer->CreateResource(bDesc, D3D12_RESOURCE_STATE_COPY_DEST, sizeof(Vector4f));
		dxContext.resourceUploader.uploadResource(dx12BoneWeightBuffer, 0, 1,
			&initData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	if (morphMeshData.morphCount != 0 && morphMeshData.vertexCount != 0) {
		if (dx12MorphVNBuffer == NULL) {
			bDesc.Width = morphMeshData.verticesAndNormals.size() * sizeof(Vector4f);
			//bDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			initData.pData = morphMeshData.verticesAndNormals.data()->data();
			dx12MorphVNBuffer = dxContext.meshBufferPool.allocate(bDesc.Width);
			dx12MorphVNBuffer->CreateResource(bDesc, D3D12_RESOURCE_STATE_COPY_DEST, sizeof(Vector4f));
			dxContext.resourceUploader.uploadResource(dx12MorphVNBuffer, 0, 1,
				&initData, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			srvDesc.Buffer.NumElements = morphMeshData.verticesAndNormals.size();
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			dx12MorphVNView.init(*dx12MorphVNBuffer, srvDesc);

			if (!dx12MorphVNView.isValid())
				throw runtime_error("CreateShaderResourceView failed");
		}
	}

	meshData.vertex = dx12VertexBuffer;
	meshData.uv = dx12UVBuffer;
	meshData.normal = dx12NormalBuffer;
	meshData.element = dx12ElementBuffer;
	meshData.bone = dx12BoneIndexBuffer;
	meshData.weight = dx12BoneWeightBuffer;

	dxContext.graphicContext.setMeshData(meshData);
	throw runtime_error("Not supported fix binding point for morph data");
	//dxContext.graphicContext.bindDefaultSRV(MORPHDATA_BIND_INDEX, dx12MorphVNView);

	currentMeshData = this;
}

void DX12SkeletonMeshData::release()
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

	if (dx12BoneIndexBuffer != NULL) {
		dx12BoneIndexBuffer->release();
		dx12BoneIndexBuffer = NULL;
	}

	if (dx12BoneWeightBuffer != NULL) {
		dx12BoneWeightBuffer->release();
		dx12BoneWeightBuffer = NULL;
	}

	if (dx12MorphVNBuffer != NULL) {
		dx12MorphVNBuffer->release();
		dx12MorphVNBuffer = NULL;
		dx12MorphVNView.reset();
	}
}