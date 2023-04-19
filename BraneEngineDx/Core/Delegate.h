#pragma once
#ifndef _DELEGATE_H_
#define _DELEGATE_H_

#include "Config.h"

template<typename T>
class Delegate;

class DelegateHandle
{
	template<typename T>
	friend class Delegate;
	struct Hash
	{
		size_t operator()(const DelegateHandle& handle) const
		{
			return handle.id;
		}

		bool operator()(const DelegateHandle& handle0, const DelegateHandle& handle1) const
		{
			return handle0.id == handle1.id;
		}
	};
	size_t id = 0;
};

template<typename R, typename... Args>
class Delegate<R(Args...)>
{
public:
	typedef std::function<R(Args...)> FuncHandle;
	DelegateHandle nextHandle;
	std::unordered_map<DelegateHandle, FuncHandle, DelegateHandle::Hash, DelegateHandle::Hash> funcs;

	Delegate() = default;

	template<typename F>
	Delegate(F&& func)
	{
		add(func);
	}

	template<typename F>
	DelegateHandle add(F&& func)
	{
		DelegateHandle handle = nextHandle;
		funcs.insert(std::make_pair(handle, FuncHandle(func)));
		nextHandle.id++;
		return handle;
	}

	template<typename T, typename F, typename... ExArgs>
	DelegateHandle add(T& obj, F&& func, ExArgs&&... exArgs)
	{
		DelegateHandle handle = nextHandle;
		funcs.insert(std::make_pair(handle, std::bind(func, &obj, exArgs...)));
		nextHandle.id++;
		return handle;
	}

	template<typename T, typename F, typename... ExArgs>
	DelegateHandle add(const T& obj, F&& func, ExArgs&&... exArgs)
	{
		DelegateHandle handle = nextHandle;
		funcs.insert(std::make_pair(handle, std::bind(func, &obj, exArgs...)));
		nextHandle.id++;
		return handle;
	}

	bool remove(const DelegateHandle& handle)
	{
		return funcs.erase(handle);
	}

	void clear()
	{
		funcs.clear();
	}

	template<typename F>
	DelegateHandle operator+=(F&& func)
	{
		return add(func);
	}

	bool operator-=(const DelegateHandle& handle)
	{
		return remove(handle);
	}

	void operator()(Args&&... args)
	{
		for (auto b = funcs.begin(), e = funcs.end(); b != e; b++)
			b->second(args...);
	}

	void operator()(R& result, Args&&... args)
	{
		result = funcs.front().second(args...);
	}

	void operator()(std::vector<R>& results, Args&&... args)
	{
		results.resize(funcs.size());
		int i = 0;
		for (auto b = funcs.begin(), e = funcs.end(); b != e; b++)
			results[i] = b->second(args...);
	}
};

template<typename... Args>
class Delegate<void(Args...)>
{
public:
	typedef std::function<void(Args...)> FuncHandle;
	DelegateHandle nextHandle;
	std::unordered_map<DelegateHandle, FuncHandle, DelegateHandle::Hash, DelegateHandle::Hash> funcs;

	Delegate() = default;

	template<typename F>
	Delegate(F&& func)
	{
		add(func);
	}

	template<typename F>
	DelegateHandle add(F&& func)
	{
		DelegateHandle handle = nextHandle;
		funcs.insert(std::make_pair(handle, FuncHandle(func)));
		nextHandle.id++;
		return handle;
	}

	template<typename T, typename F, typename... ExArgs>
	DelegateHandle add(T& obj, F&& func, ExArgs&&... exArgs)
	{
		DelegateHandle handle = nextHandle;
		funcs.insert(std::make_pair(handle, std::bind(func, &obj, exArgs...)));
		nextHandle.id++;
		return handle;
	}

	template<typename T, typename F, typename... ExArgs>
	DelegateHandle add(const T& obj, F&& func, ExArgs&&... exArgs)
	{
		DelegateHandle handle = nextHandle;
		funcs.insert(std::make_pair(handle, std::bind(func, &obj, exArgs...)));
		nextHandle.id++;
		return handle;
	}

	bool remove(const DelegateHandle& handle)
	{
		return funcs.erase(handle);
	}

	void clear()
	{
		funcs.clear();
	}

	template<typename T>
	DelegateHandle operator+=(T&& func)
	{
		return add(func);
	}

	bool operator-=(const DelegateHandle& handle)
	{
		return remove(handle);
	}

	void operator()(Args... args)
	{
		for (auto b = funcs.begin(), e = funcs.end(); b != e; b++)
			b->second(args...);
	}
};

#endif // !_DELEGATE_H_
