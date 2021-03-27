#include "DX11RenderExecution.h"
#include "DX11ShaderStage.h"

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
		cmdBuffer->Release();
	}
}

void DX11RenderExecution::executeParticle(const vector<DrawArraysIndirectCommand>& cmds)
{
	dxContext.deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	size_t size = sizeof(DrawArraysIndirectCommand) * cmds.size();
	if (cmdBufferDesc.ByteWidth != size) {
		cmdBufferDesc.ByteWidth = size;
		if (cmdBuffer != NULL) {
			cmdBuffer->Release();
		}
		dxContext.device->CreateBuffer(&cmdBufferDesc, NULL, &cmdBuffer);
	}
	D3D11_MAPPED_SUBRESOURCE cmdmappedData;
	dxContext.deviceContext->Map(cmdBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cmdmappedData);
	memcpy_s(cmdmappedData.pData, cmdBufferDesc.ByteWidth, cmds.data(), cmdBufferDesc.ByteWidth);
	dxContext.deviceContext->Unmap(cmdBuffer, 0);
	for (int i = 0; i < cmds.size(); i++) {
		DX11ShaderProgram::currentDx11Program->drawInfo.baseVertex = 0;
		DX11ShaderProgram::currentDx11Program->drawInfo.baseInstance = cmds[i].baseInstance;
		DX11ShaderProgram::currentDx11Program->uploadDrawInfo();
		dxContext.deviceContext->DrawInstancedIndirect(cmdBuffer, sizeof(DrawArraysIndirectCommand) * i);
	}
}

void DX11RenderExecution::executeMesh(const vector<DrawElementsIndirectCommand>& cmds)
{
	size_t size = sizeof(DrawElementsIndirectCommand) * cmds.size();
	if (cmdBufferDesc.ByteWidth != size) {
		cmdBufferDesc.ByteWidth = size;
		if (cmdBuffer != NULL) {
			cmdBuffer->Release();
		}
		dxContext.device->CreateBuffer(&cmdBufferDesc, NULL, &cmdBuffer);
	}
	D3D11_MAPPED_SUBRESOURCE cmdmappedData;
	dxContext.deviceContext->Map(cmdBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cmdmappedData);
	memcpy_s(cmdmappedData.pData, cmdBufferDesc.ByteWidth, cmds.data(), cmdBufferDesc.ByteWidth);
	dxContext.deviceContext->Unmap(cmdBuffer, 0);
	for (int i = 0; i < cmds.size(); i++) {
		DX11ShaderProgram::currentDx11Program->drawInfo.baseVertex = cmds[i].baseVertex;
		DX11ShaderProgram::currentDx11Program->drawInfo.baseInstance = cmds[i].baseInstance;
		DX11ShaderProgram::currentDx11Program->uploadDrawInfo();
		//dxContext.deviceContext->DrawIndexedInstanced(cmds[i].count, cmds[i].instanceCount, cmds[i].firstIndex, cmds[i].baseVertex, cmds[i].baseInstance);
		dxContext.deviceContext->DrawIndexedInstancedIndirect(cmdBuffer, sizeof(DrawElementsIndirectCommand) * i);
	}
}
