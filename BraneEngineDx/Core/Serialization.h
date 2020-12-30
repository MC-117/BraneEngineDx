#pragma once
#ifndef _SERIALIZATION_H_
#define _SERIALIZATION_H_

#include "Utility.h"
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
	vector<float> numList;
	map<string, size_t> subfeilds;
	vector<SerializationInfo> sublists;

	void clear();

	bool isArrayOf(const string& type);
	bool consistKey(const string& name);

	bool add(const string& name, float value);
	bool add(const string& name, const string& value);
	SerializationInfo* add(const string& name);

	void push(float value);
	void push(const string& value);
	SerializationInfo* push();

	void set(const string& name, const string& value);
	void set(const string& name, float value);
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
	SerializationInfo* get(const size_t i) ;
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
	virtual bool serialize(SerializationInfo& to) { return false; }
	virtual Serialization& getSerialization() const = 0;
};

class Serialization
{
public:
	static map<filesystem::path, SerializationInfo*> serializationInfoByPath;

	const string type;
	map<string, SerializationInfo*> serializationInfoByName;

	Serialization(const string& type);
	Serialization(const char* type);
	virtual ~Serialization() {}
	virtual Serializable* instantiate(const SerializationInfo& from) const = 0;
	virtual Serializable* deserialize(const SerializationInfo& from) const = 0;
	virtual bool deserialize(Serializable& object, const SerializationInfo& from) const = 0;
	virtual bool serialize(Serializable& object, SerializationInfo& to) const = 0;

	bool addInfo(SerializationInfo& info, bool overwrite = false);
	SerializationInfo* getInfoByName(const string& name);
	static SerializationInfo* getInfoByPath(const string& path);

	Serializable* deserializeByName(const string& name);
	static Serializable* deserializeByPath(const string& name);
	bool serialize(Serializable& object, const string& path, const string& name = "");
	Serializable* clone(Serializable& object);
};

#define Serialize(Type) \
class Type##Serialization : public Serialization \
{ \
public: \
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
	Type##Serialization() : Serialization(#Type) {} \
}; \
virtual Serialization& getSerialization() const; \

#define SerializeInstance(Type) Type::Type##Serialization Type::Type##Serialization::serialization; \
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
		NAME, NUM, STRING,
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

	SerializationInfoParser(istream& is);
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
};

class SVector2f : public Serializable
{
public:
	Serialize(SVector2f);
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
	Serialize(SVector3f);
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
	Serialize(SQuaternionf);
	float x = 0, y = 0, z = 0, w = 0;

	SQuaternionf(float x = 0, float y = 0, float z = 0, float w = 0);
	SQuaternionf(const Quaternionf& quat);

	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);

	SQuaternionf& operator=(const Quaternionf& v);
	operator Quaternionf();
	operator Quaternionf() const;
};


#endif // !_SERIALIZATION_H_
