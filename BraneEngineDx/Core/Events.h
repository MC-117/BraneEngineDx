#pragma once
#ifndef _EVENTS_H_
#define _EVENTS_H_

#include "Unit.h"

class Object;

typedef void(*EventVoidFunc)(void*);
typedef void(*EventPointerFunc)(void*, void*);
typedef void(*EventHandleFunc)(void*, void**);
typedef void(*EventObjectFunc)(void*, Object*);
typedef void(*EventFloatFunc)(void*, float);
typedef void(*EventIntFunc)(void*, int);
typedef void(*EventStringFunc)(void*, string);

class Events
{
public:
	void* target = NULL;
	map<string, EventVoidFunc> voidFuncField;
	map<string, EventPointerFunc> pointerFuncField;
	map<string, EventHandleFunc> handleFuncField;
	map<string, EventObjectFunc> objectFuncField;
	map<string, EventFloatFunc> floatFuncField;
	map<string, EventIntFunc> intFuncField;
	map<string, EventStringFunc> stringFuncField;

	Events(void* target);

	virtual bool call(const string& name);
	virtual bool call(const string& name, void* ptr);
	virtual bool call(const string& name, void** handle);
	virtual bool call(const string& name, Object* obj);
	virtual bool call(const string& name, float v);
	virtual bool call(const string& name, int v);
	virtual bool call(const string& name, string str);

	virtual void registerFunc(const string& name, EventVoidFunc func) final;
	virtual void registerFunc(const string& name, EventPointerFunc func) final;
	virtual void registerFunc(const string& name, EventHandleFunc func) final;
	virtual void registerFunc(const string& name, EventObjectFunc func) final;
	virtual void registerFunc(const string& name, EventFloatFunc func) final;
	virtual void registerFunc(const string& name, EventIntFunc func) final;
	virtual void registerFunc(const string& name, EventStringFunc func) final;

	bool operator()(const string& name);
	bool operator()(const string& name, void* ptr);
	bool operator()(const string& name, void** handle);
	bool operator()(const string& name, Object* obj);
	bool operator()(const string& name, float v);
	bool operator()(const string& name, int v);
	bool operator()(const string& name, string str);
};

#endif // !_EVENTS_H_
