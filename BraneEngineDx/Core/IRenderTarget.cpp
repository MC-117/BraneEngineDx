#include "IRenderTarget.h"

IRenderTarget::IRenderTarget(RenderTargetDesc& desc) : desc(desc)
{
}

IRenderTarget::~IRenderTarget()
{
}

void IRenderTarget::setDepthOnly(Texture* depthTex)
{
}

ITexture2D* IRenderTarget::getInternalDepthTexture()
{
    return nullptr;
}

unsigned int IRenderTarget::bindFrame()
{
    return 0;
}

void IRenderTarget::resize(unsigned int width, unsigned int height)
{
}

void IRenderTarget::SetMultisampleFrame()
{
}
