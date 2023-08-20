#include "SceneCapture.h"

SceneCapture::SceneCapture() : Render()
{
}

void SceneCapture::setSize(Unit2Di size)
{
}

IRendering::RenderType SceneCapture::getRenderType() const
{
	return IRendering::SceneCapture_Render;
}
