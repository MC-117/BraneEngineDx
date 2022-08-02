#pragma once
#include "../../Core/IRenderExecution.h"

#ifndef _DX12RENDEREXECUTION_H_
#define _DX12RENDEREXECUTION_H_

#ifdef VENDOR_USE_DX12

#include "DX12Context.h"

class DX12RenderExecution : public IRenderExecution
{
public:
	DX12Context& dxContext;
	DX12Buffer* cmdBuffer = NULL;

	DX12RenderExecution(DX12Context& context);
	virtual ~DX12RenderExecution();
	virtual void executeParticle(const vector<DrawArraysIndirectCommand>& cmds);
	virtual void executeMesh(const vector<DrawElementsIndirectCommand>& cmds);
};

#endif // !_DX12RENDEREXECUTION_H_

#endif // VENDOR_USE_DX12