#include "MultiSampleResolvePass.h"

void MultiSampleResolvePass::prepare()
{
	if (renderTarget == NULL)
		return;
	renderTarget->init();
}

void MultiSampleResolvePass::execute(IRenderContext& context)
{
	context.resolveMultisampleFrame(renderTarget->getVendorRenderTarget());
}
