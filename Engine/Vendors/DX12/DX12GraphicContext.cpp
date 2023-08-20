#include "DX12GraphicContext.h"

void DX12GraphicContext::init(ComPtr<ID3D12Device> device)
{
    this->device = device;
    commandListDispatcher.init(device);
}

bool DX12GraphicContext::executeCommandList(ComPtr<ID3D12CommandQueue> queue)
{
    if (commandListSet.empty())
        return false;
    vector<ID3D12GraphicsCommandList*> cmds;
    cmds.resize(commandListSet.size());
    int i = 0;
    for (auto b = commandListSet.begin(), e = commandListSet.end(); b != e; b++, i++) {
        cmds[i] = (*b)->get(false).Get();
        cmds[i]->Close();
    }
    queue->ExecuteCommandLists(cmds.size(), (ID3D12CommandList**)cmds.data());
    commandListSet.clear();
    commandList = NULL;
    return true;
}

bool DX12GraphicContext::executeComputeCommandList(ComPtr<ID3D12CommandQueue> queue)
{
    if (computeCommandListSet.empty())
        return false;
    vector<ID3D12GraphicsCommandList*> cmds;
    cmds.resize(computeCommandListSet.size());
    int i = 0;
    for (auto b = computeCommandListSet.begin(), e = computeCommandListSet.end(); b != e; b++, i++) {
        cmds[i] = (*b)->get(false).Get();
        cmds[i]->Close();
    }
    queue->ExecuteCommandLists(cmds.size(), (ID3D12CommandList**)cmds.data());
    computeCommandListSet.clear();
    computeCommandList = NULL;
    return true;
}

void DX12GraphicContext::reset(int threadID)
{
    commandListDispatcher.reset(threadID);
}

void DX12GraphicContext::beginRender(int threadID, D3D12_COMMAND_LIST_TYPE type)
{
    commandList = &commandListDispatcher.getCommandList(threadID, type);
}

void DX12GraphicContext::endRender()
{
}

DX12CommandList* DX12GraphicContext::getCommandList()
{
    return commandList;
}

DX12CommandList& DX12GraphicContext::getImGuiCommandList()
{
    if (imGuiCommandList == NULL) {
        imGuiCommandList = &commandListDispatcher.getCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT);
    }
    return *imGuiCommandList;
}

void DX12GraphicContext::transitionBarrier(DX12Buffer& buffer, D3D12_SUBRESOURCE_DATA* data, D3D12_RESOURCE_STATES transitionState, D3D12_RESOURCE_BARRIER_FLAGS flags)
{

}

void DX12GraphicContext::setRootSignature(DX12RootSignature* rootSignature)
{
    if (rootSignature == NULL || !rootSignature->isValid())
        throw runtime_error("invalid rootSignature");
    this->rootSignature = rootSignature;
    graphicPipelineState.setRootSignature(rootSignature->get());
}

void DX12GraphicContext::setComputeRootSignature(DX12RootSignature* rootSignature)
{
    if (rootSignature == NULL || !rootSignature->isValid())
        throw runtime_error("invalid rootSignature");
    this->rootSignature = rootSignature;
    computePipelineState.setRootSignature(rootSignature->get());
}

void DX12GraphicContext::setInputLayout(DX12InputLayoutType type)
{
    graphicPipelineState.setInputLayout(type);
}

void DX12GraphicContext::setCullType(DX12CullType type)
{
    graphicPipelineState.setCullType(type);
}

void DX12GraphicContext::setBlendFlags(Enum<DX12BlendFlags> flags)
{
    graphicPipelineState.setBlendFlags(flags);
}

void DX12GraphicContext::setDepthFlags(Enum<DX12DepthFlags> flags)
{
    graphicPipelineState.setDepthFlags(flags);
}

void DX12GraphicContext::setRTVs(int count, DX12Descriptor* rtvs, DX12Descriptor& dsv)
{
    auto cmdlst = commandList->get(false);
    renderTargets.resize(count);
    rtvHandles.resize(count);
    for (int i = 0; i < count; i++) {
        DX12Descriptor& rtv = rtvs[i];
        renderTargets[i] = rtv;
        rtvHandles[i] = rtv;
        if (rtv.isActive())
            rtv.getBuffer()->transitionBarrier(cmdlst, D3D12_RESOURCE_STATE_RENDER_TARGET);
    }

    depthStencilTarget = dsv;
    dsvHandle = depthStencilTarget;
    if (dsv.isActive())
        dsv.getBuffer()->transitionBarrier(cmdlst, D3D12_RESOURCE_STATE_DEPTH_WRITE);

    D3D12_CPU_DESCRIPTOR_HANDLE* pDsvHandle = NULL;
    if (depthStencilTarget.isActive())
        pDsvHandle = &dsvHandle;
    cmdlst->OMSetRenderTargets(rtvHandles.size(), rtvHandles.data(), FALSE, pDsvHandle);
}

void DX12GraphicContext::setSampleCount(int count)
{
    sampleCount = count == 0 ? 1 : count;
}

DX12Descriptor DX12GraphicContext::getRTV(int index)
{
    if (index >= renderTargets.size())
        return DX12Descriptor();
    return renderTargets[index];
}

void DX12GraphicContext::setGraphicShader(const DX12ShaderProgramData& data)
{
    graphicPipelineState.setShader(data);
}

void DX12GraphicContext::setComputeShader(const DX12ShaderProgramData& data)
{
    computePipelineState.setShader(data);
}

void DX12GraphicContext::bindVertexBuffer(int slot, DX12SubBuffer* buffer)
{
    if (slot < 0)
        return;
    auto iter = defaultVBufferBindings.find(slot);
    if (iter == defaultVBufferBindings.end()) {
        defaultVBufferBindings.insert(make_pair(slot, buffer));
    }
    else {
        iter->second = buffer;
    }
}

void DX12GraphicContext::bindDefaultCBV(int slot, const DX12ResourceView& cbv)
{
    if (slot < 0)
        return;
    auto iter = defaultCBufferBindings.find(slot);
    if (iter == defaultCBufferBindings.end()) {
        defaultCBufferBindings.insert(make_pair(slot, cbv));
    }
    else {
        iter->second = cbv;
    }
}

void DX12GraphicContext::bindDefaultSRV(int slot, const DX12ResourceView& srv)
{
    if (slot < 0)
        return;
    auto iter = defaultTBufferBindings.find(slot);
    if (iter == defaultTBufferBindings.end()) {
        defaultTBufferBindings.insert(make_pair(slot, srv));
    }
    else {
        iter->second = srv;
    }
}

void DX12GraphicContext::bindIndexBuffer(DX12SubBuffer* buffer)
{
    defaultIBuffer = buffer;
}

void DX12GraphicContext::setMeshData(const DX12GraphicMeshData& meshData)
{
    this->meshData = meshData;
    setInputLayout(meshData.layoutType);
}

void DX12GraphicContext::setTopology(D3D12_PRIMITIVE_TOPOLOGY topology)
{
    this->topology = topology;
}

void DX12GraphicContext::setViewport(const Vector4u& rect)
{
    viewportRect = rect;
}

void DX12GraphicContext::clearRTV(DX12Descriptor rtv, const Color& color, const Vector4u& rect)
{
    if (rtv.isActive()) {
        D3D12_RECT dRect = { rect.x(), rect.y(), rect.z(), rect.w() };
        auto cmdlst = commandList->get(false);
        rtv.getBuffer()->transitionBarrier(cmdlst, D3D12_RESOURCE_STATE_RENDER_TARGET);
        cmdlst->ClearRenderTargetView(rtv, (const float*)&color, 1, &dRect);
    }
}

void DX12GraphicContext::clearDSV(DX12Descriptor rtv, D3D12_CLEAR_FLAGS flags, float depth, uint8_t stencil, const Vector4u& rect)
{
    if (rtv.isActive()) {
        D3D12_RECT dRect = { rect.x(), rect.y(), rect.z(), rect.w() };
        auto cmdlst = commandList->get(false);
        rtv.getBuffer()->transitionBarrier(cmdlst, D3D12_RESOURCE_STATE_DEPTH_WRITE);
        commandList->get(false)->ClearDepthStencilView(rtv, flags, depth, stencil, 1, &dRect);
    }
}

void DX12GraphicContext::resolveMS(DX12Buffer& dst, DX12Buffer& src, DXGI_FORMAT format)
{
    auto cmdlst = commandList->get(false);
    src.transitionBarrier(cmdlst, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
    dst.transitionBarrier(cmdlst, D3D12_RESOURCE_STATE_RESOLVE_DEST);
    commandList->get(false)->ResolveSubresource(dst.get().Get(), 0, src.get().Get(), 0, format);
    dst.transitionBarrier(cmdlst, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void DX12GraphicContext::copyBuffer(DX12SubBuffer& dst, unsigned int dstOff, DX12SubBuffer& src, unsigned int srcOff, unsigned int size)
{
    commandList->get(false)->CopyBufferRegion(
        dst.get().Get(), dst.getOffset() + dstOff,
        src.get().Get(), src.getOffset() + dstOff, size);
}

void DX12GraphicContext::copyBuffer(DX12Buffer& dst, unsigned int dstOff, DX12SubBuffer& src, unsigned int srcOff, unsigned int size)
{
    commandList->get(false)->CopyBufferRegion(
        dst.get().Get(), dstOff,
        src.get().Get(), src.getOffset() + dstOff, size);
}

void DX12GraphicContext::copyBuffer(DX12SubBuffer& dst, unsigned int dstOff, DX12Buffer& src, unsigned int srcOff, unsigned int size)
{
    commandList->get(false)->CopyBufferRegion(
        dst.get().Get(), dst.getOffset() + dstOff,
        src.get().Get(), dstOff, size);
}

void DX12GraphicContext::copyBuffer(DX12Buffer& dst, unsigned int dstOff, DX12Buffer& src, unsigned int srcOff, unsigned int size)
{
    commandList->get(false)->CopyBufferRegion(
        dst.get().Get(), dstOff,
        src.get().Get(), dstOff, size);
}

void DX12GraphicContext::dispatch(unsigned int dimX, unsigned int dimY, unsigned int dimZ)
{
    if (!validateCompute())
        throw runtime_error("pipeline not complete");
    flushCompute();
    commandList->get(false)->Dispatch(dimX, dimY, dimZ);
}

void DX12GraphicContext::push()
{
    if (commandList->getType() == D3D12_COMMAND_LIST_TYPE_COMPUTE)
        computeCommandListSet.insert(commandList);
    else if (commandList->getType() == D3D12_COMMAND_LIST_TYPE_DIRECT)
        commandListSet.insert(commandList);
}

void DX12GraphicContext::draw(unsigned int vertexCount, unsigned int vertexBase)
{
    if (!validateGraphic())
        throw runtime_error("pipeline not complete");
    flushGraphic();
    commandList->get(false)->DrawInstanced(vertexCount, 1, vertexBase, 0);
}

void DX12GraphicContext::drawIndexed(unsigned int indexCount, unsigned int indexBase, unsigned int vertexBase)
{
    if (!validateGraphic())
        throw runtime_error("pipeline not complete");
    flushGraphic();
    commandList->get(false)->DrawIndexedInstanced(indexCount, 1, indexBase, vertexBase, 0);
}

void DX12GraphicContext::drawIndirect()
{
}

void DX12GraphicContext::drawInstanced(unsigned int vertexCount, unsigned int instanceCount,
    unsigned int vertexBase, unsigned int instanceBase)
{
    if (!validateGraphic())
        throw runtime_error("pipeline not complete");
    flushGraphic();
    commandList->get(false)->DrawInstanced(vertexCount, instanceCount, vertexBase, instanceBase);
}

void DX12GraphicContext::drawIndexedInstanced(unsigned int indexCount, unsigned int instanceCount,
    unsigned int indexBase, unsigned int vertexBase, unsigned int instanceBase)
{
    if (!validateGraphic())
        throw runtime_error("pipeline not complete");
    flushGraphic();
    commandList->get(false)->DrawIndexedInstanced(indexCount, instanceCount, indexBase, vertexBase, instanceBase);
}

void DX12GraphicContext::drawInstancedIndirect()
{
}

bool DX12GraphicContext::validateGraphic()
{
    if (renderTargets.empty() && !depthStencilTarget.isActive())
        return false;
    if (rootSignature == NULL || !rootSignature->isValid())
        return false;
    if (commandList == NULL)
        return false;
    if (!graphicPipelineState.isValid())
        return false;
    return true;
}

bool DX12GraphicContext::validateCompute()
{
    if (rootSignature == NULL || !rootSignature->isValid())
        return false;
    if (commandList == NULL)
        return false;
    if (!computePipelineState.isValid())
        return false;
    return true;
}

void DX12GraphicContext::flushGraphic()
{
    ComPtr<ID3D12GraphicsCommandList> cmdLst = commandList->get(true);

    vector<DXGI_FORMAT> formats;
    int count = renderTargets.size();
    formats.resize(count);
    for (int i = 0; i < count; i++) {
        formats[i] = renderTargets[i].getFormat();
    }
    graphicPipelineState.setRTVFormat(count, formats.data());

    if (depthStencilTarget.isActive()) {
        graphicPipelineState.setDSVFormat(depthStencilTarget.getFormat());
    }
    graphicPipelineState.setSampleCount(sampleCount);

    DX12GraphicPipelineState::ID id = graphicPipelineState.getID();
    auto iter = graphicPipelineStateMap.find(id);
    if (iter == graphicPipelineStateMap.end()) {
        auto& pso = graphicPipelineStateMap.insert(make_pair(id, graphicPipelineState)).first->second;
        cmdLst->SetPipelineState(pso.get(device).Get());
    }
    else {
        cmdLst->SetPipelineState(iter->second.get(device).Get());
    }

    D3D12_VERTEX_BUFFER_VIEW vertexViews[5] = {};
    if (meshData.vertex != NULL)
    {
        vertexViews[0] = { meshData.vertex->get()->GetGPUVirtualAddress(),
            (UINT)meshData.vertex->getSize(), sizeof(Vector3f) };
    }
    if (meshData.uv != NULL)
    {
        vertexViews[1] = { meshData.uv->get()->GetGPUVirtualAddress(),
            (UINT)meshData.uv->getSize(), sizeof(Vector2f) };
    }
    if (meshData.normal != NULL)
    {
        vertexViews[2] = { meshData.normal->get()->GetGPUVirtualAddress(),
            (UINT)meshData.normal->getSize(), sizeof(Vector3f) };
    }
    if (meshData.bone != NULL)
    {
        vertexViews[3] = { meshData.bone->get()->GetGPUVirtualAddress(),
            (UINT)meshData.bone->getSize(), sizeof(Vector4u) };
    }
    if (meshData.weight != NULL)
    {
        vertexViews[4] = { meshData.weight->get()->GetGPUVirtualAddress(),
            (UINT)meshData.weight->getSize(), sizeof(Vector4f) };
    }

    D3D12_INDEX_BUFFER_VIEW indexView = {};
    if (defaultIBuffer != NULL) {
        indexView = { defaultIBuffer->get()->GetGPUVirtualAddress() + defaultIBuffer->getOffset(),
            (UINT)defaultIBuffer->getSize(), DXGI_FORMAT_R32_UINT };
        defaultIBuffer = NULL;
    }
    else if(meshData.element != NULL) {
        indexView = { meshData.element->get()->GetGPUVirtualAddress(),
            (UINT)meshData.element->getSize(), DXGI_FORMAT_R32_UINT };
    }

    for (auto b = defaultVBufferBindings.begin(), e = defaultVBufferBindings.end(); b != e; b++) {
        if (b->second == NULL) {
            cmdLst->IASetVertexBuffers(b->first, 1, NULL);
        }
        else {
            D3D12_VERTEX_BUFFER_VIEW vertexView = {
                b->second->get()->GetGPUVirtualAddress() + b->second->getOffset(),
                (UINT)b->second->getSize(),  (UINT)b->second->getStride() };
            cmdLst->IASetVertexBuffers(b->first, 1, &vertexView);
        }
    }
    defaultVBufferBindings.clear();

    cmdLst->IASetVertexBuffers(meshData.startSlot, meshData.slotCount, vertexViews);
    cmdLst->IASetIndexBuffer(&indexView);

    cmdLst->IASetPrimitiveTopology(topology);

    for (auto b = defaultCBufferBindings.begin(), e = defaultCBufferBindings.end(); b != e; b++) {
        if (b->second.isValid())
            rootSignature->setBuffer(b->first, b->second);
    }

    for (auto b = defaultTBufferBindings.begin(), e = defaultTBufferBindings.end(); b != e; b++) {
        if (b->second.isValid())
            rootSignature->setTexture(b->first, b->second);
    }

    rootSignature->bindGraphic(cmdLst);

    D3D12_VIEWPORT viewport;
    viewport.TopLeftX = viewportRect.x();
    viewport.TopLeftY = viewportRect.y();
    viewport.Width = viewportRect.z();
    viewport.Height = viewportRect.w();

    viewport.MinDepth = 0;
    viewport.MaxDepth = 1;

    D3D12_RECT scissor;
    scissor.left = viewportRect.x();
    scissor.top = viewportRect.y();
    scissor.right = viewportRect.x() + viewportRect.z();
    scissor.bottom = viewportRect.y() + viewportRect.w();

    cmdLst->RSSetViewports(1, &viewport);
    cmdLst->RSSetScissorRects(1, &scissor);

    commandListSet.insert(commandList);
}

void DX12GraphicContext::flushCompute()
{
    ComPtr<ID3D12GraphicsCommandList> cmdLst = commandList->get(true);

    DX12ComputePipelineState::ID id = computePipelineState.getID();
    auto iter = computePipelineStateMap.find(id);
    if (iter == computePipelineStateMap.end()) {
        auto& pso = computePipelineStateMap.insert(make_pair(id, computePipelineState)).first->second;
        cmdLst->SetPipelineState(pso.get(device).Get());
    }
    else {
        cmdLst->SetPipelineState(iter->second.get(device).Get());
    }

    for (auto b = defaultCBufferBindings.begin(), e = defaultCBufferBindings.end(); b != e; b++) {
        if (b->second.isValid())
            rootSignature->setBuffer(b->first, b->second);
    }

    for (auto b = defaultTBufferBindings.begin(), e = defaultTBufferBindings.end(); b != e; b++) {
        if (b->second.isValid())
            rootSignature->setTexture(b->first, b->second);
    }

    rootSignature->bindCompute(cmdLst);

    if (commandList->getType() == D3D12_COMMAND_LIST_TYPE_COMPUTE)
        computeCommandListSet.insert(commandList);
    else if (commandList->getType() == D3D12_COMMAND_LIST_TYPE_DIRECT)
        commandListSet.insert(commandList);
}
