#pragma once

#include "DX12DescriptorHeap.h"

class DX12RootSignature;

class DX12RootSignatureDesc
{
	friend DX12RootSignature;
public:
	DX12RootSignatureDesc() = default;
	void addBuffer(int slot);
	void addTexture(int slot);
	void addSampler(int slot);
	void addImage(int slot);

	void addStaticSampler(const D3D12_STATIC_SAMPLER_DESC& desc, int regist, int space = 1);

	void getDesc(D3D12_ROOT_SIGNATURE_DESC& desc);
protected:
	struct Parameter
	{
		D3D12_ROOT_PARAMETER view = {};
		D3D12_ROOT_PARAMETER sampler = {};
	};

	vector<D3D12_DESCRIPTOR_RANGE> ranges;
	vector<D3D12_DESCRIPTOR_RANGE> samplerRanges;

	Parameter parameter;

	map<int, int> cbvSlot2Range;
	map<int, int> srvSlot2Range;
	map<int, int> samplerSlot2Range;
	map<int, int> uavSlot2Range;

	vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
	D3D12_ROOT_SIGNATURE_FLAGS flags;

	void addSlot(map<int, int>& slot2Range, vector<D3D12_DESCRIPTOR_RANGE>& destRanges, D3D12_DESCRIPTOR_RANGE_TYPE type, int slot);
	int getSlotIndex(map<int, int>& slot2Range, int slot);

	int getBufferIndex(int slot);
	int getTextureIndex(int slot);
	int getSamplerIndex(int slot);
	int getImageIndex(int slot);

	int getCSUHeapSize() const;
	int getSamplerHeapSize() const;
};

class DX12RootSignature
{
public:
	DX12RootSignature() = default;
	void init(ComPtr<ID3D12Device> device);
	void setDesc(DX12RootSignatureDesc& desc);

	bool isValid() const;

	void setBuffer(int slot, const DX12ResourceView& cbv);
	void setTexture(int slot, const DX12ResourceView& srv);
	void setSampler(int slot, const DX12ResourceView& sampler);
	void setImage(int slot, const DX12ResourceView& uav);

	void bindGraphic(ComPtr<ID3D12GraphicsCommandList> cmdLst);
	void bindCompute(ComPtr<ID3D12GraphicsCommandList> cmdLst);
	ComPtr<ID3D12RootSignature> get();

	void release();
protected:
	DX12RootSignatureDesc* desc = NULL;
	DX12DescriptorHeap cbv_srv_uavHeap = DX12DescriptorHeap(0, false, false);
	DX12DescriptorHeap samplerHeap = DX12DescriptorHeap(0, false, false);
	ComPtr<ID3D12Device> device;
	ComPtr<ID3DBlob> rootSignatureBlob;
	ComPtr<ID3D12RootSignature> rootSignature;
};