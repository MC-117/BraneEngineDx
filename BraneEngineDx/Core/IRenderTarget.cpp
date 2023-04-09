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

IDeviceSurface::IDeviceSurface(DeviceSurfaceDesc& desc) : desc(desc)
{
}

IDeviceSurface::~IDeviceSurface()
{
}

void IDeviceSurface::bindSurface()
{
}

void IDeviceSurface::resize(unsigned int width, unsigned int height, unsigned int multisampleLevel)
{
}

void IDeviceSurface::clearColor(const Color& color)
{
}

void IDeviceSurface::swapBuffer(bool vsync, int maxFps)
{
}

void IDeviceSurface::frameFence()
{
}
