#include "PythonScript.h"
#include <fstream>
#include "../Utility/Utility.h"
#include "../Console.h"
#include "../Asset.h"

SerializeInstance(PythonScript);

PythonScript::PythonScript()
{
}

PythonScript::PythonScript(const string & file)
{
	load(file);
}

bool PythonScript::isValid() const
{
	return pytype != NULL;
}

bool PythonScript::load(const string & file)
{
	filesystem::path p = file;
	string ext = p.extension().generic_string();
	string name = p.filename().generic_string();
	name = name.substr(0, name.size() - ext.size());
	ifstream f(file);
	if (f.fail())
		return false;
	ostringstream sin;
	sin << f.rdbuf();
	sourceCode = sin.str();
	f.close();
	f.clear();
	this->name = name;
	codePath = file;
	if (!refresh())
		return false;
	return true;
}

PythonScript::Type PythonScript::getType() const
{
	return type;
}

bool PythonScript::ErrorFetch(const char * funcName)
{
	bool occurred = false;
	while (PyErr_Occurred()) {
		PyObject* extype, * exvalue, * extraceback;
		PyErr_Fetch(&extype, &exvalue, &extraceback);
		PyErr_NormalizeException(&extype, &exvalue, &extraceback);
		if (extype != NULL) {
			PyObject* extypestr = PyObject_Str(extype);
			PyObject* valuestr = PyObject_Str(exvalue);
			const char* type = extypestr->ob_type->tp_name;
			const char* value = PyUnicode_AsUTF8(valuestr);
			Console::error(
				"Traceback(most recent call last)\n"
				"\tFile \"%s\", method \"%s\", in __main__\n"
				"%s: %s", codePath.c_str(),
				funcName, type, value);
			Console::pyError(
				"Traceback(most recent call last)\n"
				"\tFile \"%s\", method \"%s\", in __main__\n"
				"%s: %s", codePath.c_str(),
				funcName, type, value);
			occurred = true;
		}
	}
	PyErr_Clear();
	return occurred;
}

const string& PythonScript::getName() const
{
	return name;
}

const string& PythonScript::getCodePath() const
{
	return codePath;
}

const string& PythonScript::getSourceCode() const
{
	return sourceCode;
}

bool PythonScript::setSourceCode(const string & code)
{
	if (code.empty())
		return false;
	else {
		sourceCode = code;
		return true;
	}
}

bool PythonScript::refresh()
{
	if (sourceCode.empty())
		return false;
	if (PyRun_SimpleString(sourceCode.c_str()) == -1)
		return false;
	if (ErrorFetch(("class" + name).c_str()))
		return false;
	PyObject* obj = PyObject_GetAttrString(PythonManager::getMainModule(), name.c_str());
	if (obj == NULL || !PyObject_TypeCheck(obj, &PyType_Type))
		return false;
	if (PyType_IsSubtype((PyTypeObject*)obj, &BehaviorPy::Type)) {
		type = Behavior;
	}
	else if (PyType_IsSubtype((PyTypeObject*)obj, &ObjectPy::Type)) {
		type = Wrapper;
	}
	else {
		return false;
	}

	if (pytype != NULL)
		Py_DECREF(pytype);
	pytype = (PyTypeObject*)obj;
	Py_INCREF(pytype);

	for (auto b = instances.begin(), e = instances.end(); b != e; b++) {
		if ((*b)->isReady()) {
			if ((*b)->pyobject != NULL) {
				Py_DECREF((*b)->pyobject);
				(*b)->pyobject = NULL;
				(*b)->setup();
			}
		}
	}

	return true;
}

bool PythonScript::saveSourceCode()
{
	ofstream f(codePath);
	if (!f.fail()) {
		f << sourceCode;
		f.close();
	}
	return true;
}

PyObject* PythonScript::construct()
{
	if (pytype == NULL)
		refresh();
	if (!isValid())
		return NULL;
	PyObject* obj = PyObject_CallObject((PyObject*)pytype, NULL);
	if (obj == NULL || !PyObject_TypeCheck(obj, pytype))
		return NULL;
	return obj;
}

PyObject * PythonScript::construct(void* ptr)
{
	if (pytype == NULL)
		refresh();
	if (!isValid())
		return NULL;
	PyObject *obj = PyObject_CallMethod(PythonManager::getMainModule(), name.c_str(), "L", (long long)ptr);
	if (obj == NULL || !PyObject_TypeCheck(obj, pytype))
		return NULL;
	return obj;
}

Serializable* PythonScript::instantiate(const SerializationInfo& from)
{
	return new PythonScript();
}

PythonRuntimeObject::PythonRuntimeObject()
{
}

PythonRuntimeObject::PythonRuntimeObject(void * ptr) : ptr(ptr)
{
}

PythonRuntimeObject::PythonRuntimeObject(void * ptr, PythonScript & script) : ptr(ptr), script(&script)
{
	setup();
}

PythonRuntimeObject::~PythonRuntimeObject()
{
	removeScript();
}

bool PythonRuntimeObject::isValid()
{
	return script != NULL && script->isValid();
}

bool PythonRuntimeObject::isReady()
{
	return pyobject != NULL;
}

PythonScript * PythonRuntimeObject::getScript()
{
	return script;
}

void PythonRuntimeObject::setScript(PythonScript & script)
{
	if (this->script == &script)
		return;
	if (pyobject != NULL) {
		Py_DECREF(pyobject);
		pyobject = NULL;
	}
	this->script = &script;
	setup();
}

void PythonRuntimeObject::removeScript()
{
	if (script != NULL)
		script->instances.erase(this);
	if (pyobject != NULL) {
		Py_DECREF(pyobject);
		pyobject = NULL;
	}
	script = NULL;
}

void * PythonRuntimeObject::getBindedObject()
{
	return ptr;
}

void PythonRuntimeObject::bindObject(void* ptr)
{
	if (this->ptr == ptr)
		return;
	if (pyobject != NULL) {
		Py_DECREF(pyobject);
		pyobject = NULL;
	}
	this->ptr = ptr;
	setup();
}

bool PythonRuntimeObject::setup()
{
	if (!isValid())
		return false;
	if (pyobject == NULL) {
		switch (script->getType())
		{
		case PythonScript::Behavior:
			pyobject = script->construct();
			break;
		case PythonScript::Wrapper:
			if (ptr == NULL)
				return false;
			pyobject = script->construct(ptr);
			break;
		default:
			return false;
		}
	}
	if (pyobject == NULL)
		return false;
	script->instances.insert(this);
	ErrorFetch(NULL);
	return true;
}

bool PythonRuntimeObject::ErrorFetch(const char* funcName)
{
	if (script != NULL)
		return script->ErrorFetch(funcName);
	return true;
}

bool PythonRuntimeObject::call(const string& funcName)
{
	if (pyobject == NULL)
		return false;
	PyObject* func = PyObject_GetAttrString(pyobject, funcName.c_str());
	PyObject_CallFunction(func, NULL);
	return !ErrorFetch(funcName.c_str());
}

bool PythonRuntimeObject::call(const string& funcName, const string arglist, ...)
{
	if (pyobject == NULL)
		return false;
	PyObject* func = PyObject_GetAttrString(pyobject, funcName.c_str());
	va_list ap;
	va_start(ap, arglist);
	PyObject* args = Py_VaBuildValue(arglist.c_str(), ap);
	va_end(ap);
	if (args == NULL) {
		return false;
	}
	PyObject_CallObject(func, args);
	Py_DECREF(args);
	return !ErrorFetch(funcName.c_str());
}

int PythonRuntimeObject::getCallables(vector<PythonFunctionObject>& funcs)
{
	if (pyobject == NULL)
		return 0;
	PyObject* dir = pyobject->ob_type->tp_dict;
	Py_INCREF(pyobject->ob_type->tp_dict);
	if (dir == NULL) {
		ErrorFetch("PythonRuntimeObject::getCallables");
		return 0;
	}
	Console::log("Dict: %d", PyDict_GET_SIZE(dir));
	PyObject* key, * value;
	Py_ssize_t pos = 0;
	int num = 0;
	while (PyDict_Next(dir, &pos, &key, &value)) {
		if (PyCallable_Check(value)) {
			PyObject* fc = PyObject_GetAttrString(value, "__code__");
			if (fc) {
				PyObject* ac = PyObject_GetAttrString(fc, "co_argcount");
				if (ac) {
					const int count = PyLong_AsLong(ac);
					num++;
					PythonFunctionObject& func = funcs.emplace_back();
					func.func = value;
					func.object = this;
					func.name = PyUnicode_AsUTF8(key);
					func.argNum = count;
					Py_DECREF(ac);
				}
				Py_DECREF(fc);
			}
		}
	}
	Py_DECREF(dir);
	return num;
}

bool PythonFunctionObject::call()
{
	if (object == NULL || func == NULL || argNum > 1)
		return false;
	if (argNum == 0) {
		PyObject_CallFunctionObjArgs(func, NULL);
		return object->ErrorFetch(name.c_str());
	}
	else
		return object->call(name);
}

SerializeInstance(PythonObjectBehavior);

string PythonObjectBehavior::getName() const
{
	return getSerialization().type;
}

bool PythonObjectBehavior::setup()
{
	if (!PythonRuntimeObject::setup())
		return false;
	beginFunc = PyObject_GetAttrString(pyobject, "begin");
	tickFunc = PyObject_GetAttrString(pyobject, "tick");
	afterTickFunc = PyObject_GetAttrString(pyobject, "afterTick");
	endFunc = PyObject_GetAttrString(pyobject, "end");
	ErrorFetch(NULL);
	return true;
}

bool PythonObjectBehavior::init(Object* object)
{
	ObjectBehavior::init(object);
	bindObject(object);
	return true;
}

void PythonObjectBehavior::begin()
{
	if (object == NULL || pyobject == NULL || !beginFunc)
		return;
	PyObject_CallFunction(beginFunc, NULL);
	ErrorFetch("begin");
}

void PythonObjectBehavior::tick(float deltaTime)
{
	if (object == NULL || pyobject == NULL || !tickFunc)
		return;
	PyObject_CallFunction(tickFunc, "f", deltaTime);
	ErrorFetch("tick");
}

void PythonObjectBehavior::afterTick()
{
	if (object == NULL || pyobject == NULL || !afterTickFunc)
		return;
	PyObject_CallFunction(afterTickFunc, NULL);
	ErrorFetch("afterTick");
}

void PythonObjectBehavior::end()
{
	if (object == NULL || pyobject == NULL || !endFunc)
		return;
	PyObject_CallFunction(endFunc, NULL);
	ErrorFetch("end");
}

Serializable* PythonObjectBehavior::instantiate(const SerializationInfo& from)
{
	return new PythonObjectBehavior();
}

bool PythonObjectBehavior::deserialize(const SerializationInfo& from)
{
	ObjectBehavior::deserialize(from);
	string scriptPath;
	from.get("scriptPath", scriptPath);
	if (!scriptPath.empty()) {
		PythonScript* script = getAssetByPath<PythonScript>(scriptPath);
		if (script != NULL)
			setScript(*script);
	}
	return true;
}

bool PythonObjectBehavior::serialize(SerializationInfo& to)
{
	ObjectBehavior::serialize(to);
	PythonScript* script = getScript();
	if (script != NULL) {
		to.set("scriptPath", script->getCodePath());
	}
	return true;
}