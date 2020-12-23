#include "Brane.h"

std::map<void*, std::pair<TypeID, std::list<void*>*>> Brane::entityBook = std::map<void*, std::pair<TypeID, std::list<void*>*>>();
std::map<TypeID, std::map<std::string, std::list<void*>>> Brane::nameBook = std::map<TypeID, std::map<std::string, std::list<void*>>>();

bool Brane::born(TypeID type, void* ptr)
{
	return entityBook.insert(std::pair<void*, std::pair<TypeID, std::list<void*>*>>(ptr, std::pair<TypeID, std::list<void*>*>(type, NULL))).second;
}

bool Brane::born(TypeID type, void* ptr, const std::string& name)
{
	auto t = entityBook.insert(std::pair<void*, std::pair<TypeID, std::list<void*>*>>(ptr, std::pair<TypeID, std::list<void*>*>(type, NULL)));
	if (!t.second)
		return false;
	std::list<void*>*& v = t.first->second.second;
	auto iter = nameBook.find(type);
	if (iter == nameBook.end())
		(v = &(nameBook.insert(std::pair<int, std::map<std::string, std::list<void*>>>(type, std::map<std::string, std::list<void*>>())).first->second.insert(std::pair<std::string, std::list<void*>>(name, std::list<void*>())).first->second))->push_back(ptr);
	else {
		auto _iter = iter->second.find(name);
		if (_iter == iter->second.end())
			(v = &(iter->second.insert(std::pair<std::string, std::list<void*>>(name, std::list<void*>())).first->second))->push_back(ptr);
		else
			(v = &(_iter->second))->push_back(ptr);
	}
	return true;
}

bool Brane::vanish(TypeID type, void* ptr)
{
	auto iter = entityBook.find(ptr);
	if (iter == entityBook.end())
		return false;
	if (iter->second.second) {
		iter->second.second->remove(ptr);
	}
	entityBook.erase(iter);
	return true;
}

void* Brane::find(TypeID type, const std::string& name)
{
	auto iter = nameBook.find(type);
	if (iter != nameBook.end()) {
		auto _iter = iter->second.find(name);
		if (_iter != iter->second.end())
			if (_iter->second.size() != 0)
				return *(_iter->second.begin());
	}
	return NULL;
}

std::list<void*> Brane::findAll(TypeID type, const std::string& name)
{
	auto iter = nameBook.find(type);
	if (iter != nameBook.end()) {
		auto _iter = iter->second.find(name);
		if (_iter != iter->second.end())
			return _iter->second;
	}
	return std::list<void*>();
}
