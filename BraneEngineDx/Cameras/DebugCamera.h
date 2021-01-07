#pragma once
#ifndef _DEBUGCAMERA_H_
#define _DEBUGCAMERA_H_

#include "../Core/PostProcess/PostProcessingCamera.h"

class DebugCamera : public PostProcessingCamera
{
public:
	float cameraSpeed = 1;
	DebugCamera(string name = "DebugCamera");

	virtual void tick(float deltaTime);
};

#endif // !_DEBUGCAMERA_H_
