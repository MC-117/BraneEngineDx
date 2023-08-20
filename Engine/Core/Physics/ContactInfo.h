#pragma once

#include "../Unit.h"

class Transform;

struct ContactInfo
{
	string objectName;
	Vector3f location;
	Vector3f normal;
	Vector3f impact;
	Transform* otherObject = NULL;
	void* physicalObject = NULL;
	void* physicalCollider = NULL;
};