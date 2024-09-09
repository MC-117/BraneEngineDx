#pragma once
#ifndef _SERIALIZATION_H_
#define _SERIALIZATION_H_

#include "Utility/Utility.h"
#include "Utility/Decimal.h"
#include "Utility/Name.h"
#include <filesystem>

class Serializable;
class Serialization;

class ENGINE_API SerializationManager
{
public:
	// Using StaticVar class to solve the problem that some SerializationInfo objects
	// will be constructed before this variable constructed
	static StaticVar<unordered_map<Name, Serialization*>> serializationList;

	static void finalizeSerializtion();

	static Serialization* getSerialization(const Name& type);
};

class ENGINE_API SerializationInfo
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

	bool add(const string& name, const char* value);
	bool add(const string& name, const string& value);
	bool add(const string& name, const Name& value);
	bool add(const string& name, Decimal value);
	SerializationInfo* add(const string& name);
	SerializationInfo* addArray(const string& name, const string& type);
	SerializationInfo* addStringArray(const string& name);
	SerializationInfo* addNumberArray(const string& name);
	template<class T>
	SerializationInfo* addArray(const string& name);

	void push(Decimal value);
	void push(const string& value);
	SerializationInfo* push();

	void set(const string& name, const char* value);
	void set(const string& name, const string& value);
	void set(const string& name, const Name& value);
	void set(const string& name, Decimal value);
	void set(const SerializationInfo& value);
	void set(const string& name, Serializable& value);

	template<class T>
	bool get(const Path& path, T& object) const;

	template<class T>
	bool get(const size_t i, T& object) const;

	const SerializationInfo* get(const Path& name) const;
	SerializationInfo* get(const Path& name);
	const SerializationInfo* get(const size_t i) const;
	SerializationInfo* get(const size_t i);
};

class ENGINE_API Serializable
{
public:
	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from) = 0;
	virtual bool serialize(SerializationInfo& to) = 0;
	virtual const Serialization& getSerialization() const = 0;
protected:
	static void serializeInit(const Serializable* serializable, SerializationInfo& to);
};

class ENGINE_API Attribute
{
public:
	const Name name;
	const bool canInherit;

	Attribute(const Name& name, bool canInherit) : name(name), canInherit(canInherit) {}
	virtual ~Attribute() = default;

	template<class A>
	const A* cast() const;

	virtual void resolve(Attribute* sourceAttribute, Serialization& serialization) { }
	virtual void finalize(Serialization& serialization) { }
};

template<class A>
inline const A* Attribute::cast() const
{
	return dynamic_cast<const A*>(this);
}

class ENGINE_API NamespaceAttribute : public Attribute
{
public:
	string nameSpace;
	NamespaceAttribute(const string& nameSpace);

	virtual void resolve(Attribute* sourceAttribute, Serialization& serialization);
};

class ENGINE_API SerializationScope
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

template<class Type>
const char* getClassName()
{
	static_assert(std::is_base_of_v<Serializable, Type>,
		"Only support classes based on Serializable");
	return Type::serialization().type.c_str();
}

template<class Type>
Type* castTo(Serializable* serializable)
{
	static_assert(std::is_base_of_v<Serializable, Type>,
		"Only support classes based on Serializable");
	return dynamic_cast<Type*>(serializable);
}

template<class Type>
const Type* castTo(const Serializable* serializable)
{
	static_assert(std::is_base_of_v<Serializable, Type>,
		"Only support classes based on Serializable");
	return dynamic_cast<const Type*>(serializable);
}

template<class Type>
Type* castTo(Serializable& serializable)
{
	return castTo<Type>(&serializable);
}

template<class Type>
const Type* castTo(const Serializable& serializable)
{
	return castTo<Type>(&serializable);
}

template<class Type>
Type* instantiate(const SerializationInfo& from)
{
	static_assert(std::is_base_of_v<Serializable, Type>,
		"Only support classes based on Serializable");
	Serializable* serializable = Type::instantiate(from);
	Type* object = castTo<Type>(serializable);
	if (object == NULL)
		delete serializable;
	return object;
}

template<class Type>
Type* deserialize(const SerializationInfo& from)
{
	static_assert(std::is_base_of_v<Serializable, Type>,
		"Only support classes based on Serializable");
	Type* object = instantiate<Type>(from);
	if (object == NULL)
		return NULL;
	if (!object->deserialize(from)) {
		delete object;
		return NULL;
	}
	return object;
}

class ENGINE_API Serialization
{
	friend class SerializationManager;
public:
	static map<filesystem::path, SerializationInfo*> serializationInfoByPath;

	static Serialization serialization;
	const string type;
	const string baseType;
	map<string, SerializationInfo*> serializationInfoByName;

	virtual ~Serialization();
	virtual bool isClassOf(const Serializable&) const { return false; }
	virtual Serializable* instantiate(const SerializationInfo& from) const { return NULL; }
	virtual Serializable* deserialize(const SerializationInfo& from) const { return NULL; }

	Serialization* getBaseSerialization();
	Serialization* getBaseSerialization() const;

	bool isChildOf(const Serialization& serialization);
	bool isChildOf(const Serialization& serialization) const;

	int getChildren(vector<Serialization*>& children) const;

	int getAttributeCount() const;
	const Attribute* getAttribute(int index) const;
	const Attribute* getAttribute(const Name& name) const;
	template<class A>
	const A* getAttribute() const;

	bool addInfo(SerializationInfo& info, bool overwrite = false);
	SerializationInfo* getInfoByName(const string& name);
	static SerializationInfo* getInfoByPath(const string& path);

	Serializable* deserializeByName(const string& name);
	static Serializable* deserializeByPath(const string& name);
	bool serialize(Serializable& object, const string& path, const string& name = "");
	static Serializable* clone(Serializable& object);
protected:
	Serialization* baseSerialization = NULL;
	vector<Attribute*> attributes;
	bool finalized = false;

	Serialization(const char* type, const char* baseType);

	Attribute* getAttribute(const Name& name);
	void addAttribute(initializer_list<Attribute*> list);

	void finalize();
};

template<class A>
inline const A* Serialization::getAttribute() const
{
	for (const auto& attr : attributes)
	{
		const A* res = attr->cast<A>();
		if (res)
			return res;
	}
	return NULL;
}

#define DEF_ATTR(name, ...) (new name##Attribute(__VA_ARGS__))

#define Serialize(Type, BaseType) \
class Type##Serialization : public BaseType##::BaseType##Serialization \
{ \
public: \
	typedef BaseType##::BaseType##Serialization BaseSerializationClass; \
	static Type##Serialization serialization; \
	virtual bool isClassOf(const Serializable& serializable) const { return castTo<Type>(serializable); } \
	virtual Serializable* instantiate(const SerializationInfo& from) const \
	{ \
		return ::instantiate<Type>(from); \
	} \
	virtual Serializable* deserialize(const SerializationInfo& from) const \
	{ \
		return ::deserialize<Type>(from); \
	} \
protected: \
	Type##Serialization() : BaseSerializationClass(#Type, #BaseType) { init(); } \
	Type##Serialization(const char* type, const char* baseType) : BaseSerializationClass(type, baseType) { init(); } \
	void init(); \
}; \
typedef Type##Serialization ThisSerialization; \
typedef Type##Serialization SuperSerialization; \
inline static Serialization& serialization() \
{ \
	return Type##Serialization::serialization; \
} \
virtual const Serialization& getSerialization() const; \

#define SerializeInstance(Type, ...) \
Type::Type##Serialization Type::Type##Serialization::serialization; \
void Type::Type##Serialization::init() \
{ \
	baseSerialization = &BaseSerializationClass::serialization; \
	addAttribute({ __VA_ARGS__ }); \
} \
const Serialization& Type::getSerialization() const \
{ \
	return Type##Serialization::serialization; \
} \

class ENGINE_API SerializationInfoParser
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

class ENGINE_API SerializationInfoWriter
{
public:
	ostream& stream;

	SerializationInfoWriter(ostream& os);
	static string checkName(const string& name);
	void write(const SerializationInfo& info, bool showType = true);

protected:
	void internalWrite(const SerializationInfo& info, bool showType = true);
};

class ENGINE_API SVector2f : public Serializable
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

class ENGINE_API SVector3f : public Serializable
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

class ENGINE_API SVector4f : public Serializable
{
public:
	Serialize(SVector4f,);
	float x = 0, y = 0, z = 0, w = 0;

	SVector4f(float x = 0, float y = 0, float z = 0, float w = 0);
	SVector4f(const Vector4f& vec);

	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);

	SVector4f& operator=(const Vector4f& v);
	operator Vector4f();
	operator Vector4f() const;
};

class ENGINE_API SQuaternionf : public Serializable
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

class ENGINE_API SColor : public Serializable
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

class ENGINE_API SEnum
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

template<class T>
inline SerializationInfo* SerializationInfo::addArray(const string& name)
{
	return addArray(name, getClassName<T>());
}

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
inline bool SerializationInfo::get<float>(const size_t i, float & object) const
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
inline bool SerializationInfo::get<int>(const size_t i, int& object) const
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
inline bool SerializationInfo::get<long long>(const size_t i, long long& object) const
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
inline bool SerializationInfo::get<string>(const size_t i, string & object) const
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
			if (factory == NULL || !factory->isClassOf(object))
				return false;
		}
		return object.deserialize(sub);
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
		const SerializationInfo& sub = sublists[i];
		if (sub.serialization == NULL || !sub.serialization->isClassOf(object))
			return false;
		return object.deserialize(sub);
	}
	return false;
}

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
inline bool SerializationInfo::get<SEnum>(const size_t i, SEnum& object) const
{
	if (i < numList.size()) {
		object = numList[i];
		return true;
	}
	return false;
}

#endif // !_SERIALIZATION_H_
