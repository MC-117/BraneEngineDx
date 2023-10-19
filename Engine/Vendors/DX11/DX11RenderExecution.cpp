#include "DX11RenderExecution.h"
#include "DX11ShaderStage.h"

#define INDIRECT_DRAW 0

DX11RenderExecution::DX11RenderExecution(DX11Context& context)
	: dxContext(context)
{
	ZeroMemory(&cmdBufferDesc, sizeof(cmdBufferDesc));
	cmdBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cmdBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	cmdBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cmdBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
}

DX11RenderExecution::~DX11RenderExecution()
{
	if (cmdBuffer != NULL) {
		cmdBuffer.Reset();
	}
}

void DX11RenderExecution::executeParticle(const vector<DrawArraysIndirectCommand>& cmds)
{
	dxContext.deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
#if INDIRECT_DRAW
	size_t size = sizeof(DrawArraysIndirectCommand) * cmds.size();
	if (cmdBufferDesc.ByteWidth != size) {
		cmdBufferDesc.ByteWidth = size;
		/*if (cmdBuffer != NULL) {
			cmdBuffer->Release();
		}*/
		dxContext.device->CreateBuffer(&cmdBufferDesc, NULL, &cmdBuffer);
	}
	D3D11_MAPPED_SUBRESOURCE cmdmappedData;
	dxContext.deviceContext->Map(cmdBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &cmdmappedData);
	memcpy_s(cmdmappedData.pData, cmdBufferDesc.ByteWidth, cmds.data(), cmdBufferDesc.ByteWidth);
	dxContext.deviceContext->Unmap(cmdBuffer.Get(), 0);
#endif
	for (int i = 0; i < cmds.size(); i++) {
#if INDIRECT_DRAW
		dxContext.deviceContext->DrawInstancedIndirect(cmdBuffer.Get(), sizeof(DrawArraysIndirectCommand) * i);
#else
		dxContext.deviceContext->DrawInstanced(cmds[i].count, cmds[i].instanceCount, cmds[i].first, cmds[i].baseInstance);
#endif
	}
}

void DX11RenderExecution::executeMesh(const vector<DrawElementsIndirectCommand>& cmds)
{
#if INDIRECT_DRAW
	size_t size = sizeof(DrawElementsIndirectCommand) * cmds.size();
	if (cmdBufferDesc.ByteWidth != size) {
		cmdBufferDesc.ByteWidth = size;
		/*if (cmdBuffer != NULL) {
			cmdBuffer->Release();
		}*/
		dxContext.device->CreateBuffer(&cmdBufferDesc, NULL, &cmdBuffer);
	}
	D3D11_MAPPED_SUBRESOURCE cmdmappedData;
	dxContext.deviceContext->Map(cmdBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &cmdmappedData);
	memcpy_s(cmdmappedData.pData, cmdBufferDesc.ByteWidth, cmds.data(), cmdBufferDesc.ByteWidth);
	dxContext.deviceContext->Unmap(cmdBuffer.Get(), 0);
#endif
	for (int i = 0; i < cmds.size(); i++) {
		const DrawElementsIndirectCommand& c = cmds[i];
#if INDIRECT_DRAW
		dxContext.deviceContext->DrawIndexedInstancedIndirect(cmdBuffer.Get(), sizeof(DrawElementsIndirectCommand) * i);
#else
		dxContext.deviceContext->DrawIndexedInstanced(c.count, c.instanceCount, c.firstIndex, c.baseVertex, c.baseInstance);
#endif
	}
}
