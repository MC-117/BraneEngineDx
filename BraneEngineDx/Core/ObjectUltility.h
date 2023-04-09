#pragma once

#include "Object.h"

bool isSameBranch(const Object& child, const Object& other);
Object* findFirst(const Object& root, const Serialization& serialization);