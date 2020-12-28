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

void IRenderTarget::clearColor(const Color& color)
{
}

void IRenderTarget::clearColors(const vector<Color>& colors)
{
}

void IRenderTarget::clearDepthStencil(float depth, unsigned int stencil)
{
}
