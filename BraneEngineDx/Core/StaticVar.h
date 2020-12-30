#pragma once
#ifndef _STATICVAR_H_
#define _STATICVAR_H_

template<class T>
struct StaticVar
{
public:
	StaticVar();
	~StaticVar();

	T* get();
	T* operator->();
	operator T* ();
private:
	T* _this = nullptr;
	char _data[sizeof(T)];
	T* allocate();
};

template<class T>
inline StaticVar<T>::StaticVar()
{
}

template<class T>
inline StaticVar<T>::~StaticVar()
{
	if (_this)
		_this->T::~T();
}

template<class T>
inline T* StaticVar<T>::get()
{
	return allocate();
}

template<class T>
inline T* StaticVar<T>::operator->()
{
	return allocate();
}

template<class T>
inline StaticVar<T>::operator T* ()
{
	return allocate();
}

template<class T>
inline T* StaticVar<T>::allocate()
{
	if (_this == nullptr)
		_this = new(_data) T;
	return _this;
}

#endif // !_STATICVAR_H_
