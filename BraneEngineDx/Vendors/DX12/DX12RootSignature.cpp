#include "DX12RootSignature.h"

void DX12RootSignatureDesc::addBuffer(int slot)
{
	addSlot(cbvSlot2Range, ranges, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, slot);
}

void DX12RootSignatureDesc::addTexture(int slot)
{
	addSlot(srvSlot2Range, ranges, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, slot);
}

void DX12RootSignatureDesc::addSampler(int slot)
{
	addSlot(samplerSlot2Range, samplerRanges, D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, slot);
}

void DX12RootSignatureDesc::addImage(int slot)
{
	addSlot(uavSlot2Range, ranges, D3D12_DESCRIPTOR_RANGE_TYPE_UAV, slot);
}

void DX12RootSignatureDesc::addStaticSampler(const D3D12_STATIC_SAMPLER_DESC& desc, int slot, int space)
{
	D3D12_STATIC_SAMPLER_DESC& _desc = staticSamplers.emplace_back(desc);
	_desc.RegisterSpace = space;
	_desc.ShaderRegister = slot;
}

 void DX12RootSignatureDesc::getDesc(D3D12_ROOT_SIGNATURE_DESC& desc)
{
	 desc.Flags = flags;

	 int paramCount = 0;

	 if (ranges.size() > 0) {
		 paramCount++;
	 }

	 if (samplerRanges.size() > 0) {
		 paramCount++;
	 }

	 desc.NumParameters = paramCount;

	 desc.NumStaticSamplers = staticSamplers.size();
	 desc.pStaticSamplers = staticSamplers.data();

	 if (paramCount > 0) {
		 desc.pParameters = (const D3D12_ROOT_PARAMETER*)&parameter;

		 parameter.view.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		 parameter.view.DescriptorTable.NumDescriptorRanges = ranges.size();
		 parameter.view.DescriptorTable.pDescriptorRanges = ranges.data();

		 parameter.sampler.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		 parameter.sampler.DescriptorTable.NumDescriptorRanges = samplerRanges.size();
		 parameter.sampler.DescriptorTable.pDescriptorRanges = samplerRanges.data();
	 }
}

 void DX12RootSignatureDesc::addSlot(map<int, int>& slot2Range, vector<D3D12_DESCRIPTOR_RANGE>& destRanges, D3D12_DESCRIPTOR_RANGE_TYPE type, int slot)
 {
	 auto iter = slot2Range.find(slot);
	 if (iter == slot2Range.end()) {
		 int offset = destRanges.size();
		 slot2Range.insert(make_pair(slot, offset));
		 D3D12_DESCRIPTOR_RANGE& range = destRanges.emplace_back();
		 range.RangeType = type;
		 range.NumDescriptors = 1;
		 range.BaseShaderRegister = slot;
		 range.OffsetInDescriptorsFromTableStart = offset;
		 range.RegisterSpace = 0;
	 }
 }

 int DX12RootSignatureDesc::getSlotIndex(map<int, int>& slot2Range, int slot)
 {
	 auto iter = slot2Range.find(slot);
	 if (iter != slot2Range.end())
		 return iter->second;
	 return -1;
 }

 int DX12RootSignatureDesc::getBufferIndex(int slot)
 {
	 return getSlotIndex(cbvSlot2Range, slot);
 }

 int DX12RootSignatureDesc::getTextureIndex(int slot)
 {
	 return getSlotIndex(srvSlot2Range, slot);
 }

 int DX12RootSignatureDesc::getSamplerIndex(int slot)
 {
	 return getSlotIndex(samplerSlot2Range, slot);
 }

 int DX12RootSignatureDesc::getImageIndex(int slot)
 {
	 return getSlotIndex(uavSlot2Range, slot);
 }

 int DX12RootSignatureDesc::getCSUHeapSize() const
 {
	 return ranges.size();
 }

 int DX12RootSignatureDesc::getSamplerHeapSize() const
 {
	 return samplerRanges.size();
 }

void DX12RootSignature::init(ComPtr<ID3D12Device> device)
{
	this->device = device;
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		desc.NodeMask = 1;
		cbv_srv_uavHeap.init(device, desc);
	}
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		desc.NodeMask = 1;
		samplerHeap.init(device, desc);
	}
}

void DX12RootSignature::setDesc(DX12RootSignatureDesc& desc)
{
	cbv_srv_uavHeap.resize(desc.getCSUHeapSize());
	samplerHeap.resize(desc.getSamplerHeapSize());

	D3D12_ROOT_SIGNATURE_DESC rootDesc;
	ZeroMemory(&rootDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	desc.getDesc(rootDesc);
	rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	ComPtr<ID3DBlob> errorBlob;
	if (FAILED(D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			&rootSignatureBlob, &errorBlob))) {
		const char* error_string = (const char*)errorBlob->GetBufferPointer();
		throw runtime_error(error_string);
	}
	if (FAILED(device->CreateRootSignature(1,
			rootSignatureBlob->GetBufferPointer(),
			rootSignatureBlob->GetBufferSize(),
			IID_PPV_ARGS(&rootSignature)))) {
		throw runtime_error("create rootSignature failed");
	}
	this->desc = &desc;
}

bool DX12RootSignature::isValid() const
{
	return desc != NULL && rootSignature != NULL;
}

void DX12RootSignature::setBuffer(int slot, const DX12ResourceView& cbv)
{
	if (desc == NULL)
		throw runtime_error("rootSignatureDesc not initialized");
	if (!cbv.isValid() || cbv.type != DX12ResourceViewType::CBV)
		throw runtime_error("cbv not valid");
	int index = desc->getBufferIndex(slot);
	if (index >= 0)
		cbv_srv_uavHeap.update(cbv, index);
}

void DX12RootSignature::setTexture(int slot, const DX12ResourceView& srv)
{
	if (desc == NULL)
		throw runtime_error("rootSignatureDesc not initialized");
	if (!srv.isValid() || srv.type != DX12ResourceViewType::SRV)
		throw runtime_error("srv not valid");
	int index = desc->getTextureIndex(slot);
	if (index >= 0)
		cbv_srv_uavHeap.update(srv, index);
}

void DX12RootSignature::setSampler(int slot, const DX12ResourceView& sampler)
{
	if (desc == NULL)
		throw runtime_error("rootSignatureDesc not initialized");
	if (!sampler.isValid() || sampler.type != DX12ResourceViewType::Sampler)
		throw runtime_error("sampler not valid");
	int index = desc->getSamplerIndex(slot);
	if (index >= 0)
		samplerHeap.update(sampler, index);
}

void DX12RootSignature::setImage(int slot, const DX12ResourceView& uav)
{
	if (desc == NULL)
		throw runtime_error("rootSignatureDesc not initialized");
	if (!uav.isValid() || uav.type != DX12ResourceViewType::UAV)
		throw runtime_error("cbv not valid");
	int index = desc->getImageIndex(slot);
	if (index >= 0)
		cbv_srv_uavHeap.update(uav, index);
}

void DX12RootSignature::bindGraphic(ComPtr<ID3D12GraphicsCommandList> cmdLst)
{
	if (desc == NULL)
		throw runtime_error("rootSignatureDesc not initialized");
	int heapCount = 0;
	ID3D12DescriptorHeap* heaps[2];
	if (cbv_srv_uavHeap.size() > 0) {
		heaps[heapCount] = cbv_srv_uavHeap.get().Get();
		heapCount++;
	}
	if (samplerHeap.size() > 0) {
		heaps[heapCount] = samplerHeap.get().Get();
		heapCount++;
	}

	cmdLst->SetGraphicsRootSignature(rootSignature.Get());

	cmdLst->SetDescriptorHeaps(heapCount, heaps);

	heapCount = 0;
	if (cbv_srv_uavHeap.size() > 0) {
		cmdLst->SetGraphicsRootDescriptorTable(heapCount, cbv_srv_uavHeap.get()->GetGPUDescriptorHandleForHeapStart());
		heapCount++;
	}
	if (samplerHeap.size() > 0) {
		cmdLst->SetGraphicsRootDescriptorTable(heapCount, samplerHeap.get()->GetGPUDescriptorHandleForHeapStart());
		heapCount++;
	}
}

void DX12RootSignature::bindCompute(ComPtr<ID3D12GraphicsCommandList> cmdLst)
{
	if (desc == NULL)
		throw runtime_error("rootSignatureDesc not initialized");
	int heapCount = 0;
	ID3D12DescriptorHeap* heaps[2] = {};
	if (cbv_srv_uavHeap.size() > 0) {
		heaps[heapCount] = cbv_srv_uavHeap.get().Get();
		heapCount++;
	}
	if (samplerHeap.size() > 0) {
		heaps[heapCount] = samplerHeap.get().Get();
		heapCount++;
	}

	cmdLst->SetComputeRootSignature(rootSignature.Get());

	cmdLst->SetDescriptorHeaps(heapCount, heaps);

	heapCount = 0;
	if (cbv_srv_uavHeap.size() > 0) {
		cmdLst->SetComputeRootDescriptorTable(heapCount, cbv_srv_uavHeap.get()->GetGPUDescriptorHandleForHeapStart());
		heapCount++;
	}
	if (samplerHeap.size() > 0) {
		cmdLst->SetComputeRootDescriptorTable(heapCount, samplerHeap.get()->GetGPUDescriptorHandleForHeapStart());
		heapCount++;
	}
}

ComPtr<ID3D12RootSignature> DX12RootSignature::get()
{
	return rootSignature;
}

void DX12RootSignature::release()
{
	this->desc = NULL;
	rootSignatureBlob.Reset();
	rootSignature.Reset();
	cbv_srv_uavHeap.release();
	samplerHeap.release();
}
