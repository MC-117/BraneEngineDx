#include "MultiSampleResolvePass.h"

void MultiSampleResolvePass::prepare()
{
	if (renderTarget == NULL)
		return;
	renderTarget->init();
}

void MultiSampleResolvePass::execute(IRenderContext& context)
{
	if (renderTarget == NULL)
		return;
	context.resolveMultisampleFrame(renderTarget->getVendorRenderTarget());
}

void MultiSampleResolvePass::reset()
{
}
