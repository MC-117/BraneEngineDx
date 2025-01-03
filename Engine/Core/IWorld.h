#pragma once
#include "Object.h"

class ENGINE_API IWorld
{
public:
	virtual Object* find(const string& name) const = 0;
	virtual Object* getObject() const = 0;
};
