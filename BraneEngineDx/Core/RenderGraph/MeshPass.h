#pragma once

#include "../RenderCore/RenderCommandList.h"

class MeshPass : public RenderPass
{
public:
	RenderCommandList* commandList = NULL;

	virtual void prepare();
	virtual void execute(IRenderContext& context);
protected:
	Timer timer;
};