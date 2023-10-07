#pragma once

#include "Live2DConfig.h"
#include "../Core/Camera.h"

class ENGINE_API Live2DCamera : public Camera
{
public:
	Texture2D texture = Texture2D(size.x, size.y, 4, false, { TW_Clamp, TW_Clamp, TF_Linear, TF_Linear });
	RenderTarget renderTarget = RenderTarget(size.x, size.y, 4, false);

	Live2DCamera();
};