#pragma once

#include "../RenderCore/RenderInterface.h"
#include "../RenderTarget.h"

class MultiSampleResolvePass : public RenderPass
{
public:
	RenderTarget* renderTarget = NULL;

	virtual void prepare();
	virtual void execute(IRenderContext& context);
};