#pragma once

#include "Object.h"

ENGINE_API bool isSameBranch(const Object& child, const Object& other);
ENGINE_API Object* findFirst(const Object& root, const Serialization& serialization);

template<class T>
Ref<T> getInstanceRef(const SerializationInfo& from, const string& name)
{
	Ref<T> ref;
	const SerializationInfo* refInfo = from.get("targetTransform");
	if (refInfo != NULL)
		ref.deserialize(*refInfo);
	return ref;
}
