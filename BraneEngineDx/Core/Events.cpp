#include "Events.h"

Events::Events(void* target)
{
	this->target = target;
}

bool Events::call(const string& name)
{
	if (target == NULL)
		return false;
	auto iter = voidFuncField.find(name);
	if (iter == voidFuncField.end())
		return false;
	iter->second(target);
	return true;
}

bool Events::call(const string& name, void* ptr)
{
	if (target == NULL)
		return false;
	auto iter = pointerFuncField.find(name);
	if (iter == pointerFuncField.end())
		return false;
	iter->second(target, ptr);
	return true;
}

bool Events::call(const string& name, void** handle)
{
	if (target == NULL)
		return false;
	auto iter = handleFuncField.find(name);
	if (iter == handleFuncField.end())
		return false;
	iter->second(target, handle);
	return true;
}

bool Events::call(const string& name, Object* obj)
{
	if (target == NULL)
		return false;
	auto iter = objectFuncField.find(name);
	if (iter == objectFuncField.end())
		return false;
	iter->second(target, obj);
	return true;
}

bool Events::call(const string& name, float v)
{
	if (target == NULL)
		return false;
	auto iter = floatFuncField.find(name);
	if (iter == floatFuncField.end())
		return false;
	iter->second(target, v);
	return true;
}

bool Events::call(const string& name, int v)
{
	if (target == NULL)
		return false;
	auto iter = intFuncField.find(name);
	if (iter == intFuncField.end())
		return false;
	iter->second(target, v);
	return true;
}

bool Events::call(const string& name, string str)
{
	if (target == NULL)
		return false;
	auto iter = stringFuncField.find(name);
	if (iter == stringFuncField.end())
		return false;
	iter->second(target, str);
	return true;
}

void Events::onBegin()
{
	for (auto b = onBeginField.begin(), e = onBeginField.end(); b != e; b++)
		if (*b != NULL)
			(*b)((Object*)target);
}

void Events::onTick(float deltaTime)
{
	for (auto b = onTickField.begin(), e = onTickField.end(); b != e; b++)
		if (*b != NULL)
			(*b)((Object*)target, deltaTime);
}

void Events::onAfterTick()
{
	for (auto b = onAfterTickField.begin(), e = onAfterTickField.end(); b != e; b++)
		if (*b != NULL)
			(*b)((Object*)target);
}

void Events::onEnd()
{
	for (auto b = onEndField.begin(), e = onEndField.end(); b != e; b++)
		if (*b != NULL)
			(*b)((Object*)target);
}

void Events::registerFunc(const string& name, EventVoidFunc func)
{
	voidFuncField[name] = func;
}

void Events::registerFunc(const string& name, EventPointerFunc func)
{
	pointerFuncField[name] = func;
}

void Events::registerFunc(const string& name, EventHandleFunc func)
{
	handleFuncField[name] = func;
}

void Events::registerFunc(const string& name, EventObjectFunc func)
{
	objectFuncField[name] = func;
}

void Events::registerFunc(const string& name, EventFloatFunc func)
{
	floatFuncField[name] = func;
}

void Events::registerFunc(const string& name, EventIntFunc func)
{
	intFuncField[name] = func;
}

void Events::registerFunc(const string& name, EventStringFunc func)
{
	stringFuncField[name] = func;
}

void Events::registerOnBegin(EventOnBegin func)
{
	onBeginField.insert(func);
}

void Events::registerOnTick(EventOnTick func)
{
	onTickField.insert(func);
}

void Events::registerOnAfterTick(EventOnAfterTick func)
{
	onAfterTickField.insert(func);
}

void Events::registerOnEnd(EventOnEnd func)
{
	onEndField.insert(func);
}

bool Events::operator()(const string& name)
{
	return call(name);
}

bool Events::operator()(const string& name, void* ptr)
{
	return call(name, ptr);
}

bool Events::operator()(const string& name, void** handle)
{
	return call(name, handle);
}

bool Events::operator()(const string& name, Object* obj)
{
	return call(name, obj);
}

bool Events::operator()(const string& name, float v)
{
	return call(name, v);
}

bool Events::operator()(const string& name, int v)
{
	return call(name, v);
}

bool Events::operator()(const string& name, string str)
{
	return call(name, str);
}
