#pragma once
#ifndef _SERIALIZATION_H_
#define _SERIALIZATION_H_

#include "Utility/Utility.h"
#include "Utility/Decimal.h"
#include <filesystem>

class Serializable;
class Serialization;

class SerializationManager
{
public:
	// Using StaticVar class to solve the problem that some SerializationInfo objects
	// will be constructed before this variable constructed
	static StaticVar<map<string, Serialization*>> serializationList;

	static Serialization* getSerialization(const string& type);
};

class SerializationInfo
{
public:
	string type;
	string arrayType;
	string path;
	string name;
	Serialization* serialization;
	map<string, size_t> stringFeild;
	vector<string> stringList;
	map<string, size_t> numFeild;
	vector<Decimal> numList;
	map<string, size_t> subfeilds;
	vector<SerializationInfo> sublists;

	void clear();

	bool isArrayOf(const string& type);
	bool consistKey(const string& name);

	bool add(const string& name, Decimal value);
	bool add(const string& name, const string& value);
	SerializationInfo* add(const string& name);

	void push(Decimal value);
	void push(const string& value);
	SerializationInfo* push();

	void set(const string& name, const string& value);
	void set(const string& name, Decimal value);
	void set(const SerializationInfo& value);
	void set(const string& name, Serializable& value);

	template<class T>
	bool get(const Path& path, T& object) const;
	template<class T>
	bool get(const Path& path, T& object);

	template<class T>
	bool get(const size_t i, T& object) const;
	template<class T>
	bool get(const size_t i, T& object);

	const SerializationInfo* get(const Path& name) const;
	SerializationInfo* get(const Path& name);
	const SerializationInfo* get(const size_t i) const;
	SerializationInfo* get(const size_t i);
};

template<>
inline bool SerializationInfo::get<float>(const Path & path, float & object) const
{
	if (path.empty())
		return false;
	if (path.size() == 1) {
		auto iter = numFeild.find(path[0]);
		if (iter == numFeild.end())
			return false;
		object = numList[iter->second];
		return true;
	}
	else {
		auto _iter = subfeilds.find(path[0]);
		if (_iter == subfeilds.end())
			return false;
		return sublists[_iter->second].get<float>(path.popTop(), object);
	}
}

template<>
inline bool SerializationInfo::get<float>(const Path & path, float & object)
{
	if (path.empty())
		return false;
	if (path.size() == 1) {
		auto iter = numFeild.find(path[0]);
		if (iter == numFeild.end())
			return false;
		object = numList[iter->second];
		return true;
	}
	else {
		auto _iter = subfeilds.find(path[0]);
		if (_iter == subfeilds.end())
			return false;
		return sublists[_iter->second].get<float>(path.popTop(), object);
	}
}

template<>
inline bool SerializationInfo::get<float>(const size_t i, float & object) const
{
	if (i < numList.size()) {
		object = numList[i];
		return true;
	}
	return false;
}

template<>
inline bool SerializationInfo::get<float>(const size_t i, float & object)
{
	if (i < numList.size()) {
		object = numList[i];
		return true;
	}
	return false;
}

template<>
inline bool SerializationInfo::get<int>(const Path& path, int& object) const
{
	if (path.empty())
		return false;
	if (path.size() == 1) {
		auto iter = numFeild.find(path[0]);
		if (iter == numFeild.end())
			return false;
		object = numList[iter->second];
		return true;
	}
	else {
		auto _iter = subfeilds.find(path[0]);
		if (_iter == subfeilds.end())
			return false;
		return sublists[_iter->second].get<int>(path.popTop(), object);
	}
}

template<>
inline bool SerializationInfo::get<int>(const Path& path, int& object)
{
	if (path.empty())
		return false;
	if (path.size() == 1) {
		auto iter = numFeild.find(path[0]);
		if (iter == numFeild.end())
			return false;
		object = numList[iter->second];
		return true;
	}
	else {
		auto _iter = subfeilds.find(path[0]);
		if (_iter == subfeilds.end())
			return false;
		return sublists[_iter->second].get<int>(path.popTop(), object);
	}
}

template<>
inline bool SerializationInfo::get<int>(const size_t i, int& object) const
{
	if (i < numList.size()) {
		object = numList[i];
		return true;
	}
	return false;
}

template<>
inline bool SerializationInfo::get<int>(const size_t i, int& object)
{
	if (i < numList.size()) {
		object = numList[i];
		return true;
	}
	return false;
}

template<>
inline bool SerializationInfo::get<long long>(const Path& path, long long& object) const
{
	if (path.empty())
		return false;
	if (path.size() == 1) {
		auto iter = numFeild.find(path[0]);
		if (iter == numFeild.end())
			return false;
		object = numList[iter->second];
		return true;
	}
	else {
		auto _iter = subfeilds.find(path[0]);
		if (_iter == subfeilds.end())
			return false;
		return sublists[_iter->second].get<long long>(path.popTop(), object);
	}
}

template<>
inline bool SerializationInfo::get<long long>(const Path& path, long long& object)
{
	if (path.empty())
		return false;
	if (path.size() == 1) {
		auto iter = numFeild.find(path[0]);
		if (iter == numFeild.end())
			return false;
		object = numList[iter->second];
		return true;
	}
	else {
		auto _iter = subfeilds.find(path[0]);
		if (_iter == subfeilds.end())
			return false;
		return sublists[_iter->second].get<long long>(path.popTop(), object);
	}
}

template<>
inline bool SerializationInfo::get<long long>(const size_t i, long long& object) const
{
	if (i < numList.size()) {
		object = numList[i];
		return true;
	}
	return false;
}

template<>
inline bool SerializationInfo::get<long long>(const size_t i, long long& object)
{
	if (i < numList.size()) {
		object = numList[i];
		return true;
	}
	return false;
}

template<>
inline bool SerializationInfo::get<string>(const Path & path, string & object) const
{
	if (path.empty())
		return false;
	if (path.size() == 1) {
		auto iter = stringFeild.find(path[0]);
		if (iter == stringFeild.end())
			return false;
		object = stringList[iter->second];
		return true;
	}
	else {
		auto _iter = subfeilds.find(path[0]);
		if (_iter == subfeilds.end())
			return false;
		return sublists[_iter->second].get<string>(path.popTop(), object);
	}
}

template<>
inline bool SerializationInfo::get<string>(const Path & path, string & object)
{
	if (path.empty())
		return false;
	if (path.size() == 1) {
		auto iter = stringFeild.find(path[0]);
		if (iter == stringFeild.end())
			return false;
		object = stringList[iter->second];
		return true;
	}
	else {
		auto _iter = subfeilds.find(path[0]);
		if (_iter == subfeilds.end())
			return false;
		return sublists[_iter->second].get<string>(path.popTop(), object);
	}
}

template<>
inline bool SerializationInfo::get<bool>(const Path& path, bool& object) const
{
	if (path.empty())
		return false;
	string str;
	if (path.size() == 1) {
		auto iter = stringFeild.find(path[0]);
		if (iter == stringFeild.end())
			return false;
		str = stringList[iter->second];
	}
	else {
		auto _iter = subfeilds.find(path[0]);
		if (_iter == subfeilds.end())
			return false;
		if (!sublists[_iter->second].get<string>(path.popTop(), str))
			return false;
	}
	object = str == "true";
	return true;
}

template<>
inline bool SerializationInfo::get<bool>(const Path& path, bool& object)
{
	if (path.empty())
		return false;
	string str;
	if (path.size() == 1) {
		auto iter = stringFeild.find(path[0]);
		if (iter == stringFeild.end())
			return false;
		str = stringList[iter->second];
	}
	else {
		auto _iter = subfeilds.find(path[0]);
		if (_iter == subfeilds.end())
			return false;
		if (!sublists[_iter->second].get<string>(path.popTop(), str))
			return false;
	}
	object = str == "true";
	return true;
}

template<>
inline bool SerializationInfo::get<string>(const size_t i, string & object) const
{
	if (i < stringList.size()) {
		object = stringList[i];
		return true;
	}
	return false;
}

template<>
inline bool SerializationInfo::get<string>(const size_t i, string & object)
{
	if (i < stringList.size()) {
		object = stringList[i];
		return true;
	}
	return false;
}

template<class T>
inline bool SerializationInfo::get(const Path & path, T & object) const
{
	if (path.empty())
		return false;
	if (path.size() == 1) {
		auto iter = subfeilds.find(path[0]);
		if (iter == subfeilds.end())
			return false;
		const SerializationInfo& sub = sublists[iter->second];
		const Serialization* factory = sub.serialization;
		if (factory == NULL) {
			factory = SerializationManager::getSerialization(sub.type);
			if (factory == NULL)
				return false;
		}
		return factory->deserialize(object, sub);
	}
	else {
		auto _iter = subfeilds.find(path[0]);
		if (_iter == subfeilds.end())
			return false;
		return sublists[_iter->second].get<T>(path.popTop(), object);
	}
}

template<class T>
inline bool SerializationInfo::get(const Path & path, T & object)
{
	if (path.empty())
		return false;
	if (path.size() == 1) {
		auto iter = subfeilds.find(path[0]);
		if (iter == subfeilds.end())
			return false;
		SerializationInfo& sub = sublists[iter->second];
		if (sub.serialization == NULL) {
			sub.serialization = SerializationManager::getSerialization(sub.type);
			if (sub.serialization == NULL)
				return false;
		}
		return sub.serialization->deserialize(object, sub);
	}
	else {
		auto _iter = subfeilds.find(path[0]);
		if (_iter == subfeilds.end())
			return false;
		return sublists[_iter->second].get<T>(path.popTop(), object);
	}
}

template<class T>
inline bool SerializationInfo::get(const size_t i, T & object) const
{
	if (i < sublists.size()) {
		if (sublists[i].serialization == NULL)
			return false;
		return sublists[i].serialization->deserialize(object, sublists[i]);
	}
	return false;
}

template<class T>
inline bool SerializationInfo::get(const size_t i, T & object)
{
	if (i < sublists.size()) {
		if (sublists[i].serialization == NULL)
			return false;
		return sublists[i].serialization->deserialize(object, sublists[i]);
	}
	return false;
}

class Serializable
{
public:
	static Serializable* instantiate(const SerializationInfo& from) { return NULL; }
	virtual ~Serializable() {}
	virtual bool deserialize(const SerializationInfo& from) { return false; }
	virtual bool serialize(SerializationInfo& to);
	virtual Serialization& getSerialization() const = 0;
};

class Attribute
{
public:
	const string name;
	const bool canInherit;

	Attribute(const string& name, bool canInherit) : name(name), canInherit(canInherit) {}
	virtual ~Attribute() = default;

	virtual void resolve(Attribute* conflict) { }
};

class SerializationScope
{
public:
	SerializationScope() { lastScope = currentScope; currentScope = this; }
	virtual ~SerializationScope() { currentScope = lastScope; }

	static SerializationScope* getScope() { return currentScope; }

	virtual void serializationConstuction(Serialization* serialization) {}

	virtual void instantiate(Serializable* serializable, const SerializationInfo& from) {}
	virtual void deserialize(Serializable* serializable, const SerializationInfo& from) {}
	virtual void serialize(const Serializable* serializable, const SerializationInfo& from) {}
protected:
	SerializationScope* lastScope = NULL;
	static SerializationScope* currentScope;
};

class Serialization
{
public:
	static map<filesystem::path, SerializationInfo*> serializationInfoByPath;

	static Serialization serialization;
	const string type;
	const string baseType;
	map<string, SerializationInfo*> serializationInfoByName;

	virtual ~Serialization();
	virtual Serializable* instantiate(const SerializationInfo& from) const { return NULL; }
	virtual Serializable* deserialize(const SerializationInfo& from) const { return NULL; }
	virtual bool deserialize(Serializable& object, const SerializationInfo& from) const { return true; }
	virtual bool serialize(Serializable& object, SerializationInfo& to) const { return true; }

	Serialization* getBaseSerialization();
	Serialization* getBaseSerialization() const;

	bool isChildOf(const Serialization& serialization);
	bool isChildOf(const Serialization& serialization) const;

	int getChildren(vector<Serialization*>& children) const;

	int getAttributeCount() const;
	Attribute* getAttribute(int index) const;
	Attribute* getAttribute(const string& name) const;
	template<class T>
	Attribute* getAttribute() const;

	bool addInfo(SerializationInfo& info, bool overwrite = false);
	SerializationInfo* getInfoByName(const string& name);
	static SerializationInfo* getInfoByPath(const string& path);

	Serializable* deserializeByName(const string& name);
	static Serializable* deserializeByPath(const string& name);
	bool serialize(Serializable& object, const string& path, const string& name = "");
	static Serializable* clone(Serializable& object);
protected:
	Serialization* baseSerialization = NULL;
	map<string, Attribute*> attributeNameMap;
	vector<Attribute*> attributes;

	Serialization(const string& type, const string& baseType);
	Serialization(const char* type, const char* baseType);

	virtual void init();
	void addAttribute(const vector<Attribute*>& list);
};

template<class T>
inline Attribute* Serialization::getAttribute() const
{
	for each (auto attr in attributes)
	{
		T* re = dynamic_cast<T*>(attr);
		if (re)
			return res;
	}
	return NULL;
}

#define Serialize(Type, BaseType) class Type##Serialization : public BaseType##::BaseType##Serialization \
{ \
public: \
	typedef BaseType##::BaseType##Serialization BaseSerializationClass; \
	static Type##Serialization serialization; \
	virtual Serializable* instantiate(const SerializationInfo& from) const \
	{ \
		Type* object = dynamic_cast<Type*>(Type::instantiate(from)); \
		return object; \
	} \
	virtual Serializable* deserialize(const SerializationInfo& from) const \
	{ \
		Type* object = dynamic_cast<Type*>(Type::instantiate(from)); \
		if (object == NULL) \
			return NULL; \
		if (!object->deserialize(from)) { \
			delete object; \
			return NULL; \
		} \
		return object; \
	} \
	virtual bool deserialize(Serializable& object, const SerializationInfo& from) const \
	{ \
		Type* pobj = dynamic_cast<Type*>(&object); \
		if (pobj == NULL) \
			return false; \
		return object.deserialize(from); \
	} \
	virtual bool serialize(Serializable& object, SerializationInfo& to) const \
	{ \
		Type* pobj = dynamic_cast<Type*>(&object); \
		if (pobj == NULL) \
			return false; \
		to.type = #Type; \
		return object.serialize(to); \
	} \
protected: \
	Type##Serialization() : BaseSerializationClass(#Type, #BaseType) {} \
	Type##Serialization(const string& type, const string& baseType) : BaseSerializationClass(type, baseType) {} \
	Type##Serialization(const char* type, const char* baseType) : BaseSerializationClass(type, baseType) {} \
	virtual void init(); \
}; \
virtual Serialization& getSerialization() const; \

#define SerializeInstance(Type, Attributes) \
Type::Type##Serialization Type::Type##Serialization::serialization; \
void Type::Type##Serialization::init() \
{ \
	BaseSerializationClass::init(); \
	baseSerialization = &BaseSerializationClass::serialization; \
	addAttribute({ Attributes }); \
} \
Serialization& Type::getSerialization() const \
{ \
	return Type##Serialization::serialization; \
} \

class SerializationInfoParser
{
public:
	enum TokenType
	{
		ENDFILE, ERR,
		NAME, INT, FLOAT, STRING,
		COLON, LBR, RBR, LSBR, RSBR, COMMA,
	};

	enum StateType
	{
		START, INNUM, INNAME, INSTRING, DONE
	};

	bool Error = false;
	string tokenString;
	string backupTokenString;
	string errorString;
	string parseError;
	istream& stream;
	vector<SerializationInfo> infos;
	TokenType token, backupToken;

	string path;

	SerializationInfoParser(istream& is, const string& path = "");
	bool parse();
protected:
	char getNextChar();
	void ungetNextChar();
	TokenType getToken();
	void findError(string msg, bool scannerError = false);
	void match(TokenType expected);
	bool object_sequence();
	bool object(SerializationInfo& info);
	bool array(SerializationInfo& info);
	bool object_array(SerializationInfo& info);
	bool factor(SerializationInfo& info, const string& name);
};

class SerializationInfoWriter
{
public:
	ostream& stream;

	SerializationInfoWriter(ostream& os);
	static string checkName(const string& name);
	void write(const SerializationInfo& info, bool showType = true);

protected:
	void internalWrite(const SerializationInfo& info, bool showType = true);
};

class SVector2f : public Serializable
{
public:
	Serialize(SVector2f,);
	float x = 0, y = 0;

	SVector2f(float x = 0, float y = 0);
	SVector2f(const Vector2f& vec);

	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);

	SVector2f& operator=(const Vector2f& v);
	operator Vector2f();
	operator Vector2f() const;
};

class SVector3f : public Serializable
{
public:
	Serialize(SVector3f,);
	float x = 0, y = 0, z = 0;

	SVector3f(float x = 0, float y = 0, float z = 0);
	SVector3f(const Vector3f& vec);

	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);

	SVector3f& operator=(const Vector3f& v);
	operator Vector3f();
	operator Vector3f() const;
};

class SQuaternionf : public Serializable
{
public:
	Serialize(SQuaternionf,);
	float x = 0, y = 0, z = 0, w = 0;

	SQuaternionf(float x = 0, float y = 0, float z = 0, float w = 0);
	SQuaternionf(const Quaternionf& quat);

	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);

	SQuaternionf& operator=(const Quaternionf& v);
	operator Quaternionf();
	operator Quaternionf() const;
};

class SColor : public Serializable
{
public:
	Serialize(SColor,);
	float r = 0, g = 0, b = 0, a = 0;

	SColor(float r = 0, float g = 0, float b = 0, float a = 0);
	SColor(const Color& color);

	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);

	SColor& operator=(const Color& c);
	operator Color();
	operator Color() const;
};

class SEnum
{
public:
	SEnum(const void* enumPtr) : ptr((int*)enumPtr) { }
	SEnum(const SEnum& e) { ptr = e.ptr; }
	SEnum(SEnum&& e) { ptr = e.ptr; }

	operator int() { return *ptr; }
	SEnum& operator =(const SEnum& e) { *ptr = *e.ptr; return *this; }
	SEnum& operator =(int value) { *ptr = value; return *this; }
protected:
	int* ptr;
};

template<>
inline bool SerializationInfo::get<SEnum>(const Path& path, SEnum& object) const
{
	if (path.empty())
		return false;
	if (path.size() == 1) {
		auto iter = numFeild.find(path[0]);
		if (iter == numFeild.end())
			return false;
		object = numList[iter->second];
		return true;
	}
	else {
		auto _iter = subfeilds.find(path[0]);
		if (_iter == subfeilds.end())
			return false;
		return sublists[_iter->second].get<SEnum>(path.popTop(), object);
	}
}

template<>
inline bool SerializationInfo::get<SEnum>(const Path& path, SEnum& object)
{
	if (path.empty())
		return false;
	if (path.size() == 1) {
		auto iter = numFeild.find(path[0]);
		if (iter == numFeild.end())
			return false;
		object = numList[iter->second];
		return true;
	}
	else {
		auto _iter = subfeilds.find(path[0]);
		if (_iter == subfeilds.end())
			return false;
		return sublists[_iter->second].get<SEnum>(path.popTop(), object);
	}
}

template<>
inline bool SerializationInfo::get<SEnum>(const size_t i, SEnum& object) const
{
	if (i < numList.size()) {
		object = numList[i];
		return true;
	}
	return false;
}

template<>
inline bool SerializationInfo::get<SEnum>(const size_t i, SEnum& object)
{
	if (i < numList.size()) {
		object = numList[i];
		return true;
	}
	return false;
}

#endif // !_SERIALIZATION_H_
