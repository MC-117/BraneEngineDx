#pragma once

#include "Live2DConfig.h"
#include "../Core/Camera.h"

class Live2DCamera : public Camera
{
public:
	Texture2D texture = Texture2D(size.x, size.y, 4, true, { TW_Clamp, TW_Clamp, TF_Linear, TF_Linear });
	RenderTarget renderTarget = RenderTarget(size.x, size.y, 4, true);

	Live2DCamera();
};