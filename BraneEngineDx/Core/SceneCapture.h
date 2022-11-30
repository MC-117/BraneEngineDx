#pragma once

#include "Camera.h"

class SceneCapture : public Render
{
public:
	SceneCapture();

	virtual void setSize(Unit2Di size);

	virtual IRendering::RenderType getRenderType() const;
};