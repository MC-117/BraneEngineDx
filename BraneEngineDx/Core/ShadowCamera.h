#pragma once
#ifndef _SHADOWCAMERA_H_
#define _SHADOWCAMERA_H_

#include "Camera.h"
#include "ShadowCameraRender.h"

class ShadowCamera : public Camera
{
public:
	ShadowCameraRender shadowCameraRender;

	ShadowCamera(Unit2Di size = { 8132, 8132 }, string name = "ShadowCamera");
};

#endif // !_SHADOWCAMERA_H_
