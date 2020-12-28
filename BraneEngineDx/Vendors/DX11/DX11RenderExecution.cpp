#include "DX11RenderExecution.h"

DX11RenderExecution::DX11RenderExecution(const DX11Context& context)
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
	if (cmdBuffer != NULL)
		cmdBuffer->Release();
}

void DX11RenderExecution::executeParticle(const vector<DrawArraysIndirectCommand>& cmds)
{
	dxContext.deviceContext->IASetInputLayout(NULL);
	size_t size = sizeof(DrawArraysIndirectCommand) * cmds.size();
	if (cmdBufferDesc.ByteWidth != size) {
		cmdBufferDesc.ByteWidth = size;
		if (cmdBuffer != NULL)
			cmdBuffer->Release();
		dxContext.device->CreateBuffer(&cmdBufferDesc, NULL, &cmdBuffer);
	}
	D3D11_MAPPED_SUBRESOURCE cmdmappedData;
	dxContext.deviceContext->Map(cmdBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cmdmappedData);
	memcpy_s(cmdmappedData.pData, cmdBufferDesc.ByteWidth, cmds.data(), cmdBufferDesc.ByteWidth);
	dxContext.deviceContext->Unmap(cmdBuffer, 0);
	for (int i = 0; i < cmds.size(); i++)
		dxContext.deviceContext->DrawIndexedInstancedIndirect(cmdBuffer, sizeof(DrawArraysIndirectCommand) * i);
}

void DX11RenderExecution::executeMesh(const vector<DrawElementsIndirectCommand>& cmds)
{
	size_t size = sizeof(DrawElementsIndirectCommand) * cmds.size();
	if (cmdBufferDesc.ByteWidth != size) {
		cmdBufferDesc.ByteWidth = size;
		if (cmdBuffer != NULL)
			cmdBuffer->Release();
		dxContext.device->CreateBuffer(&cmdBufferDesc, NULL, &cmdBuffer);
	}
	D3D11_MAPPED_SUBRESOURCE cmdmappedData;
	dxContext.deviceContext->Map(cmdBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cmdmappedData);
	memcpy_s(cmdmappedData.pData, cmdBufferDesc.ByteWidth, cmds.data(), cmdBufferDesc.ByteWidth);
	dxContext.deviceContext->Unmap(cmdBuffer, 0);
	for (int i = 0; i < cmds.size(); i++)
		dxContext.deviceContext->DrawIndexedInstancedIndirect(cmdBuffer, sizeof(DrawElementsIndirectCommand) * i);
}
