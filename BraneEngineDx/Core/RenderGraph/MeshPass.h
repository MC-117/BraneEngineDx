#pragma once

#include "../RenderCore/RenderCommandList.h"

class MeshPass : public RenderPass
{
public:
	RenderCommandList* commandList = NULL;
	bool requireClearFrame = true;

	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();
protected:
	Timer timer;
};