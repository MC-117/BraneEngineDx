#pragma once
#ifndef _PYTHONSCRIPT_H_
#define _PYTHONSCRIPT_H_

#include "PythonManager.h"
#include "../Object.h"
#include "ScriptBase.h"

class PythonRuntimeObject;

class ENGINE_API PythonScript : public ScriptBase
{
	friend class PythonRuntimeObject;
public:
	Serialize(PythonScript, ScriptBase);
	
	enum Type {
		None, Wrapper, Behavior
	};

	PythonScript();
	PythonScript(const string& file);

	virtual bool isValid() const;
	virtual bool load(const string& file);

	Type getType() const;

	bool ErrorFetch(const char* funcName);

	virtual const string& getName() const;
	virtual const string& getCodePath() const;
	virtual const string& getSourceCode() const;

	virtual bool setSourceCode(const string& code);
	virtual bool refresh();

	virtual bool saveSourceCode();

	PyObject* construct();
	PyObject* construct(void* ptr);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Type type = None;
	PyTypeObject* pytype = NULL;
	string sourceCode;
	string codePath;
	string name;

	set<PythonRuntimeObject*> instances;
};

struct ENGINE_API PythonFunctionObject
{
	PyObject* func;
	PythonRuntimeObject* object;
	string name;
	int argNum;

	bool call();
};

class ENGINE_API PythonRuntimeObject
{
	friend class PythonScript;
	friend class PythonFunctionObject;
public:
	PythonRuntimeObject();
	PythonRuntimeObject(void* ptr);
	PythonRuntimeObject(void* ptr, PythonScript& script);
	virtual ~PythonRuntimeObject();

	bool isValid();
	bool isReady();
	PythonScript* getScript();
	void setScript(PythonScript& script);
	void removeScript();

	void* getBindedObject();
	void bindObject(void* ptr);

	virtual bool setup();
	bool ErrorFetch(const char* funcName);

	bool call(const string& funcName);
	bool call(const string& funcName, const string arglist, ...);

	int getCallables(vector<PythonFunctionObject>& funcs);
protected:
	PythonScript* script = NULL;
	void* ptr = NULL;
	PyObject* pyobject = NULL;
};

class ENGINE_API PythonObjectBehavior : public ObjectBehavior, public PythonRuntimeObject
{
public:
	Serialize(PythonObjectBehavior, ObjectBehavior);

	PythonObjectBehavior() = default;
	virtual ~PythonObjectBehavior() = default;

	virtual string getName() const;

	virtual bool setup();

	virtual bool init(Object* object);

	virtual void begin();
	virtual void tick(float deltaTime);
	virtual void afterTick();
	virtual void end();

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	PyObject* beginFunc = NULL;
	PyObject* tickFunc = NULL;
	PyObject* afterTickFunc = NULL;
	PyObject* endFunc = NULL;
};

#endif // !_PYTHONSCRIPT_H_
