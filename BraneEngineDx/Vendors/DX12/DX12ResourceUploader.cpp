#include "DX12ResourceUploader.h"

void DX12ResourceUploader::init(ComPtr<ID3D12Device> device)
{
	this->device = device;
	uploadBufferPool.init(device, CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, true);
	commandListDispatcher.init(device);
	genMipShaderCode = "\
Texture2D srcMap : register(t7);\n\
SamplerState srcMapSampler : register(s7);\n\
RWTexture2D<unorm float4> mipMap : register(u0);\n\
\n\
[numthreads(8, 8, 1)]\n\
void main(uint3 gid : SV_DispatchThreadID)\n\
{\n\
	uint2 pos = uint2(gid.xy);\n\
	uint2 size;\n\
	mipMap.GetDimensions(size.x, size.y);\n\
	if (pos.x < size.x && pos.y < size.y)\n\
	{\n\
		float2 uv = float2(pos.x / (float)size.x, pos.y / (float)size.y);\n\
		mipMap[pos] = srcMap.SampleLevel(srcMapSampler, uv, 0);\n\
	}\n\
}";
	initGenMipResource();
}

void DX12ResourceUploader::reset(int threadID)
{
	commandListDispatcher.reset(threadID);
	commandList = &commandListDispatcher.getCommandList(threadID, D3D12_COMMAND_LIST_TYPE_DIRECT);

	if (uploadBufferList == &uploadBufferList1)
		uploadBufferList = &uploadBufferList2;
	else
		uploadBufferList = &uploadBufferList1;

	for (auto b = uploadBufferList->begin(), e = uploadBufferList->end(); b != e; b++)
		(*b)->release();
	uploadBufferList->clear();

	if (uploadDescHeapList == &uploadDescHeapList1)
		uploadDescHeapList = &uploadDescHeapList2;
	else
		uploadDescHeapList = &uploadDescHeapList1;
	for (auto b = uploadDescHeapList->begin(), e = uploadDescHeapList->end(); b != e; b++)
	{
		(*b)->release();
		delete* b;
	}
	uploadDescHeapList->clear();
}

DX12CommandList* DX12ResourceUploader::getCommandList()
{
	return commandList;
}

DX12SubBuffer* DX12ResourceUploader::createUploadResource(UINT size)
{
	DX12SubBuffer* uploadBuffer = uploadBufferPool.suballocate(size, 4);

	uploadBufferList->push_back(uploadBuffer);

	return uploadBuffer;
}

bool DX12ResourceUploader::execute(ComPtr<ID3D12CommandQueue> queue)
{
	if (commandList->getUseCount() > 0) {
		ComPtr<ID3D12GraphicsCommandList> cmdLst = commandList->get(false).Get();
		cmdLst->Close();
		queue->ExecuteCommandLists(1, (ID3D12CommandList**)cmdLst.GetAddressOf());
		return true;
	}
	return false;
}

void DX12ResourceUploader::uploadResource(DX12Buffer* buffer, int base, int size, D3D12_SUBRESOURCE_DATA* data, D3D12_RESOURCE_STATES transitionState)
{
	ComPtr<ID3D12Resource> texture = buffer->get();

	UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, 1);

	DX12SubBuffer* uploadBuffer = createUploadResource(uploadBufferSize);

	ComPtr<ID3D12GraphicsCommandList> cmdLst = commandList->get(true).Get();
	UpdateSubresources(cmdLst.Get(), texture.Get(), uploadBuffer->get().Get(), 0, base + uploadBuffer->getOffset(), size, data);
	buffer->transitionBarrier(cmdLst, transitionState);
}

void DX12ResourceUploader::generateMips(DX12Buffer* buffer, DXGI_FORMAT format, int width, int height, int mips)
{
	if (buffer == NULL)
		return;

	DX12DescriptorHeap* descHeap = new DX12DescriptorHeap((mips - 1) * 2);

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 1;
	descHeap->init(device, desc);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	for (int level = 1; level < mips; level++) {
		srvDesc.Texture2D.MostDetailedMip = level - 1;
		uavDesc.Texture2D.MipSlice = level;
		descHeap->updateSRV(*buffer, srvDesc, (level - 1) * 2);
		descHeap->updateUAV(*buffer, uavDesc, level * 2 - 1);
	}

	uploadDescHeapList->push_back(descHeap);

	ComPtr<ID3D12GraphicsCommandList> cmdLst = commandList->get(true).Get();

	cmdLst->SetPipelineState(mipPipelineState.Get());
	cmdLst->SetComputeRootSignature(mipRootSignature.Get());
	cmdLst->SetDescriptorHeaps(1, descHeap->get().GetAddressOf());

	for (int level = 1; level < mips; level++) {
		cmdLst->SetComputeRootDescriptorTable(0, (*descHeap)[(level - 1) * 2]);
		cmdLst->Dispatch(ceil(width / 8.0f), ceil(height / 8.0f), 1);
		buffer->transitionBarrier(cmdLst, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			D3D12CalcSubresource(level, 0, 0, mips, 1));
		width /= 2, height /= 2;
	}
}

void DX12ResourceUploader::initGenMipResource()
{
	if (genMipShaderBlob == NULL) {
		ComPtr<ID3DBlob> errorBlob = NULL;
		unsigned int compileFlag = D3DCOMPILE_ENABLE_STRICTNESS;
		if (FAILED(D3DCompile(genMipShaderCode.c_str(), genMipShaderCode.size() * sizeof(char), "GenMip", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main", "cs_5_0", compileFlag, 0, &genMipShaderBlob, &errorBlob))) {
			string errorString = (const char*)errorBlob->GetBufferPointer();
			throw runtime_error(errorString);
		}
	}
	if (mipRootSignature == NULL) {
		D3D12_DESCRIPTOR_RANGE ranges[2] = {};
		ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		ranges[0].NumDescriptors = 1;
		ranges[0].BaseShaderRegister = 7;
		ranges[0].OffsetInDescriptorsFromTableStart = 0;
		ranges[0].RegisterSpace = 0;
		ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		ranges[1].NumDescriptors = 1;
		ranges[1].BaseShaderRegister = 0;
		ranges[1].OffsetInDescriptorsFromTableStart = 1;
		ranges[1].RegisterSpace = 0;
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param.DescriptorTable.NumDescriptorRanges = 2;
		param.DescriptorTable.pDescriptorRanges = ranges;
		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.Filter = D3D12_ENCODE_BASIC_FILTER(D3D12_FILTER_TYPE_LINEAR,
			D3D12_FILTER_TYPE_LINEAR, D3D12_FILTER_TYPE_POINT, 0);
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		sampler.MaxAnisotropy = 16;
		sampler.MinLOD = 0;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 7;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		D3D12_ROOT_SIGNATURE_DESC desc = {};
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		desc.NumParameters = 1;
		desc.pParameters = &param;
		desc.NumStaticSamplers = 1;
		desc.pStaticSamplers = &sampler;

		ComPtr<ID3DBlob> errorBlob;
		if (FAILED(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1,
			&mipRootSignatureBlob, &errorBlob))) {
			throw runtime_error((const char*)errorBlob->GetBufferPointer());
		}
		if (FAILED(device->CreateRootSignature(1,
			mipRootSignatureBlob->GetBufferPointer(),
			mipRootSignatureBlob->GetBufferSize(),
			IID_PPV_ARGS(&mipRootSignature)))) {
			throw runtime_error("create mipRootSignature failed");
		}
	}
	if (mipPipelineState == NULL) {
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
		desc.NodeMask = 1;
		desc.CS = CD3DX12_SHADER_BYTECODE(genMipShaderBlob.Get());
		desc.pRootSignature = mipRootSignature.Get();

		if (FAILED(device->CreateComputePipelineState(&desc, IID_PPV_ARGS(&mipPipelineState)))) {
			throw runtime_error("create mipPipelineState failed");
		}
	}
}