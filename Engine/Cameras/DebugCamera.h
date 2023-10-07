#pragma once
#ifndef _DEBUGCAMERA_H_
#define _DEBUGCAMERA_H_

#include "../Core/Camera.h"

class ENGINE_API DebugCamera : public Camera
{
public:
	float cameraSpeed = 1;
	DebugCamera(string name = "DebugCamera");

	virtual void tick(float deltaTime);
};

#endif // !_DEBUGCAMERA_H_
