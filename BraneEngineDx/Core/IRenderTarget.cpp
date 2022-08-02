#include "IRenderTarget.h"

RenderTargetDesc* IRenderTarget::defaultRenderTargetDesc = NULL;
IRenderTarget* IRenderTarget::defaultRenderTarget = NULL;

IRenderTarget::IRenderTarget(RenderTargetDesc& desc) : desc(desc)
{
}

IRenderTarget::~IRenderTarget()
{
}

bool IRenderTarget::isDefault() const
{
    return this == defaultRenderTarget;
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

void IRenderTarget::clearBind()
{
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

void IRenderTarget::clearDepth(float depth)
{
}

void IRenderTarget::clearStencil(unsigned int stencil)
{
}
