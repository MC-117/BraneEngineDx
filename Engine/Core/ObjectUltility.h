#pragma once

#include "Object.h"
#include "World.h"

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

ENGINE_API World* getRootWorld(const Object& object);

template<class T>
T* getObjectBehavior(Object& object)
{
	static_assert(std::is_base_of<ObjectBehavior, T>::value,
		"Only support classes based on Base");
	return castTo<T>(object.getBehavior(T::serialization()));
}
