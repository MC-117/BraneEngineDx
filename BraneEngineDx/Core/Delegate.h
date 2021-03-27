#pragma once
#ifndef _DELEGATE_H_
#define _DELEGATE_H_

#include "Config.h"

template<class ...Args>
class Delegate
{
public:
	typedef void(*FuncType)(Args...);
	vector<FuncType> funcs;

	void add(FuncType func)
	{
		funcs.push_back(func);
	}
	
	void clear()
	{
		funcs.clear();
	}

	Delegate& operator+=(FuncType func)
	{
		funcs.push_back(func);
		return *this;
	}

	void operator()(Args...args)
	{
		for (auto b = funcs.begin(), e = funcs.end(); b != e; b++)
			(*b)(args...);
	}
};

#endif // !_DELEGATE_H_
