#pragma once

#include "DX12.h"

class DX12CommandListDispatcher;

class DX12CommandList
{
	friend DX12CommandListDispatcher;

protected:
	D3D12_COMMAND_LIST_TYPE type;
	int useCount = 0;
	ComPtr<ID3D12GraphicsCommandList> cmdlst;
	ComPtr<ID3D12CommandAllocator> allocator;

public:
	DX12CommandList() = default;
	ComPtr<ID3D12GraphicsCommandList> get(bool addUseCount);
	D3D12_COMMAND_LIST_TYPE getType() const;
	int getUseCount() const;

	void reset(ComPtr<ID3D12Device> device);
};

class DX12CommandListDispatcher
{
public:
	class Batch
	{
	public:
		int threadID;
		int listMaxShader = 3;
		vector<DX12CommandList> directlists;
		vector<DX12CommandList> bundlelists;
		vector<DX12CommandList> computelists;
		vector<DX12CommandList> copylists;

		void reset(ComPtr<ID3D12Device> device);
		DX12CommandList* get(D3D12_COMMAND_LIST_TYPE type, ComPtr<ID3D12Device> device);
	};

	ComPtr<ID3D12Device> device;
	unordered_map<int, Batch> batches;

	void init(ComPtr<ID3D12Device> device);

	void reset(int threadID);
	DX12CommandList& getCommandList(int threadID, D3D12_COMMAND_LIST_TYPE type);
};

