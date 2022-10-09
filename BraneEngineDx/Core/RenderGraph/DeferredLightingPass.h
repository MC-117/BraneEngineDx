#pragma once

#include "../RenderCore/RenderInterface.h"
#include "../RenderCore/MaterialRenderData.h"

struct DeferredLightingTask
{
	ShaderProgram* program = NULL;
	MaterialRenderData* material = NULL;
};

class DeferredLightingPass : public RenderPass
{
public:
	virtual void prepare();
	virtual void execute(IRenderContext& context);
	virtual void reset();
protected:
	Timer timer;
};