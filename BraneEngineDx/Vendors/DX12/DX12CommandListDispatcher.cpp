#include "DX12CommandListDispatcher.h"

void DX12CommandList::reset(ComPtr<ID3D12Device> device)
{
    if (cmdlst == NULL) {
        device->CreateCommandAllocator(type, IID_PPV_ARGS(&allocator));
        device->CreateCommandList(1, type, allocator.Get(), NULL, IID_PPV_ARGS(&cmdlst));
        this->allocator = allocator;
        useCount = 0;
    }
    else if (useCount > 0) {
        //cmdlst->Close();
        allocator->Reset();
        cmdlst->Reset(allocator.Get(), NULL);
        useCount = 0;
    }
}

ComPtr<ID3D12GraphicsCommandList> DX12CommandList::get(bool addUseCount)
{
    if (cmdlst != NULL && addUseCount)
        useCount++;
    return cmdlst;
}

D3D12_COMMAND_LIST_TYPE DX12CommandList::getType() const
{
    return type;
}

int DX12CommandList::getUseCount() const
{
    return useCount;
}

void DX12CommandListDispatcher::Batch::reset(ComPtr<ID3D12Device> device)
{
    for (int i = 0; i < directlists.size(); i++)
        directlists[i].reset(device);
    for (int i = 0; i < bundlelists.size(); i++)
        bundlelists[i].reset(device);
    for (int i = 0; i < computelists.size(); i++)
        computelists[i].reset(device);
    for (int i = 0; i < copylists.size(); i++)
        copylists[i].reset(device);
}

DX12CommandList* DX12CommandListDispatcher::Batch::get(D3D12_COMMAND_LIST_TYPE type, ComPtr<ID3D12Device> device)
{
    vector<DX12CommandList>* lists = NULL;
    switch (type)
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        lists = &directlists;
        break;
    case D3D12_COMMAND_LIST_TYPE_BUNDLE:
        lists = &bundlelists;
        break;
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        lists = &computelists;
        break;
    case D3D12_COMMAND_LIST_TYPE_COPY:
        lists = &copylists;
        break;
    default:
        throw runtime_error("Not support");
        break;
    }
    DX12CommandList* cmdList = NULL;
    for (int i = 0; i < lists->size(); i++) {
        DX12CommandList& _cmdlst = (*lists)[i];
        _cmdlst.reset(device);
        if (_cmdlst.useCount < listMaxShader)
            cmdList = &_cmdlst;
    }

    if (cmdList == NULL) {
        cmdList = &lists->emplace_back(DX12CommandList());
        cmdList->type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        cmdList->reset(device);
        cmdList->cmdlst->SetName((L"CmdLst ThreadID" + to_wstring(threadID)).c_str());
    }

    return cmdList;
}

void DX12CommandListDispatcher::init(ComPtr<ID3D12Device> device)
{
    this->device = device;
}

void DX12CommandListDispatcher::reset(int threadID)
{
    auto iter = batches.find(threadID);
    if (iter != batches.end()) {
        iter->second.reset(device);
    }
}

DX12CommandList& DX12CommandListDispatcher::getCommandList(int threadID, D3D12_COMMAND_LIST_TYPE type)
{
    auto iter = batches.find(threadID);
    Batch* batch = NULL;
    if (iter == batches.end()) {
        batch = &batches.emplace(make_pair(threadID, Batch())).first->second;
        batch->threadID = threadID;
    }
    else {
        batch = &iter->second;
    }
    return *batch->get(type, device);
}
