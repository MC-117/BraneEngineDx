#include "VirtualShadowMapDepthPass.h"

bool VirtualShadowMapDepthPass::setRenderCommand(const IRenderCommand& cmd)
{
	return false;
}

void VirtualShadowMapDepthPass::prepare()
{
}

void VirtualShadowMapDepthPass::execute(IRenderContext& context)
{
}

void VirtualShadowMapDepthPass::reset()
{
}

void VirtualShadowMapDepthPass::getOutputTextures(vector<pair<string, Texture*>>& textures)
{
}
