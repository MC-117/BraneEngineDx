#pragma once

#include "../RenderCore/RenderCommandList.h"

class MeshPass : public RenderPass
{
public:
	RenderCommandList* commandList = NULL;

	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();
protected:
	Timer timer;
};