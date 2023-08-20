#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include <D3Dcompiler.h>

#include <dxgi.h>
#include <iostream>
#include <wrl.h>
#include <unordered_map>
#include "../Core/GraphicType.h"
#include "../Core/Utility/hash.h"
#include "../Core/Unit.h"

template <class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

template<class DescType, class ViewType>
struct DXViewSet
{
	std::unordered_map<size_t, ComPtr<ViewType>> viewMap;

	inline ComPtr<ViewType> getView(const DescType& desc)
	{
		size_t hash = hash_bytes((unsigned char*)&desc, sizeof(desc));
		auto iter = viewMap.find(hash);
		if (iter == viewMap.end())
			return NULL;
		return iter->second;
	}

	inline void setView(const DescType& desc, ComPtr<ViewType> view)
	{
		size_t hash = hash_bytes((unsigned char*)&desc, sizeof(desc));
		viewMap[hash] = view;
	}

	inline void clear()
	{
		viewMap.clear();
	}
};