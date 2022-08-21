#pragma once
#include "../../Core/IRenderExecution.h"

#ifndef _DX11RENDEREXECUTION_H_
#define _DX11RENDEREXECUTION_H_

#ifdef VENDOR_USE_DX11

#include "DX11.h"

class DX11RenderExecution : public IRenderExecution
{
public:
	DX11Context& dxContext;
	D3D11_BUFFER_DESC cmdBufferDesc;
	ComPtr<ID3D11Buffer> cmdBuffer;
	D3D11_BUFFER_DESC drawInfoBufferDesc;
	ComPtr<ID3D11Buffer> drawInfoBuffer;

	DX11RenderExecution(DX11Context& context);
	virtual ~DX11RenderExecution();
	virtual void executeParticle(const vector<DrawArraysIndirectCommand>& cmds);
	virtual void executeMesh(const vector<DrawElementsIndirectCommand>& cmds);
};

#endif // !_DX11RENDEREXECUTION_H_

#endif // VENDOR_USE_DX11
