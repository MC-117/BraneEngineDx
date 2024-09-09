#include "Render.h"
#include "RenderPool.h"

Render::Render()
{
}


Render::~Render()
{
}

void Render::setBaseColor(Color color)
{
}

Color Render::getBaseColor()
{
	return { 0, 0, 0, 0 };
}

void Render::preRender(PreRenderInfo& info)
{
}

void Render::render(RenderInfo& info)
{
}

void Render::postRender()
{
}

Matrix4f Render::getTransformMatrix() const
{
	return transformMat;
}

vector<Matrix4f>& Render::getTransformMatrixs()
{
	vector<Matrix4f> re = { transformMat };
	return re;
}

IRendering::RenderType Render::getRenderType() const
{
	return IRendering::RenderType::None_Render;
}

Shape * Render::getShape() const
{
	return NULL;
}

Material * Render::getMaterial(unsigned int index)
{
	return NULL;
}

bool Render::getMaterialEnable(unsigned int index)
{
	return false;
}

Shader* Render::getShader() const
{
	return NULL;
}

RenderTarget * Render::getShadowRenderTarget() const
{
	return NULL;
}

bool Render::getCanCastShadow() const
{
	return canCastShadow;
}

void * Render::getRender() const
{
	return (void*)this;
}
