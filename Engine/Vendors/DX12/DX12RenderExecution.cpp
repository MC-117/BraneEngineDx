#include "DX12RenderExecution.h"

DX12RenderExecution::DX12RenderExecution(DX12Context& context) : dxContext(context)
{
}

DX12RenderExecution::~DX12RenderExecution()
{
}

void DX12RenderExecution::executeParticle(const vector<DrawArraysIndirectCommand>& cmds)
{
	for (int i = 0; i < cmds.size(); i++) {
		const DrawArraysIndirectCommand& cmd = cmds[i];
		dxContext.graphicContext.drawInstanced(cmd.count, cmd.instanceCount,
			cmd.first, cmd.baseInstance);
	}
}

void DX12RenderExecution::executeMesh(const vector<DrawElementsIndirectCommand>& cmds)
{
	for (int i = 0; i < cmds.size(); i++) {
		const DrawElementsIndirectCommand& cmd = cmds[i];
		dxContext.graphicContext.drawIndexedInstanced(cmd.count, cmd.instanceCount,
			cmd.firstIndex, cmd.baseVertex, cmd.baseInstance);
	}
}
