#pragma once
#ifndef _BRANE_H_
#define _BRANE_H_

#include <iostream>
#include <map>
#include <list>
#include <typeinfo>

typedef unsigned int TypeID;

static class Brane
{
public:
	static bool born(TypeID type, void* ptr);
	static bool born(TypeID type, void* ptr, const std::string& name);
	static bool vanish(TypeID type, void* ptr);
	static void* find(TypeID type, const std::string& name);
	static std::list<void*> findAll(TypeID type, const std::string& name);
protected:
	static std::map<void*, std::pair<TypeID, std::list<void*>*>> entityBook;
	static std::map<TypeID, std::map<std::string, std::list<void*>>> nameBook;
};

#endif // !_BRANE_H_
