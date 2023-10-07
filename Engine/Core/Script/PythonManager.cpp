#include "PythonManager.h"
#include <pythonrun.h>
#include "PythonFreeze.h"
#include "../Engine.h"
#include "../Physics/SphericalConstraint.h"
#include "../Physics/FixedConstraint.h"
#include "../Physics/D6Constraint.h"
#include "../MeshActor.h"
#include "../SkeletonMeshActor.h"

vector<PyMethodDef> PythonManager::methods;
bool PythonManager::isInit = false;
PyObject* PythonManager::pyMainMod = NULL;
vector<pair<const char*, PyTypeObject*>> PythonManager::typeObjects;
map<PyTypeObject*, set<PyObject*>> PythonManager::runtimeObjects;
map<const char*, long> PythonManager::constInt;
PyModuleDef PythonManager::engineMod = { PyModuleDef_HEAD_INIT, "BraneEngine", NULL, -1, NULL };

void PythonManager::start()
{
	isInit = true;
	Python_Init_Freeze();
	methods.push_back({ NULL, NULL, 0, NULL });
	PyImport_AppendInittab("BraneEngine", []()->PyObject* {
		PyObject *m = PyModule_Create(&PythonManager::engineMod);
		if (m == NULL)
			return NULL;

		PyModule_AddFunctions(m, methods.data());

		for (auto b = PythonManager::typeObjects.begin(), e = PythonManager::typeObjects.end(); b != e; b++) {
			if (PyType_Ready(b->second) < 0)
				return NULL;
			//Py_INCREF(b->second);
			PyModule_AddObject(m, b->first, (PyObject *)b->second);
		}

		for (auto b = PythonManager::constInt.begin(), e = PythonManager::constInt.end(); b != e; b++) {
			if (PyModule_AddIntConstant(m, b->first, b->second) == -1)
				Console::error("PythonManager: Cannot add const int '%s' = %d", b->first, b->second);
		}
		return m;
	});
	Py_Initialize();
	isInit = Py_IsInitialized();
	if (isInit) {
		PyImport_ImportModule("BraneEngineLog");
		PyImport_ImportModule("BraneEngineErr");
		PyRun_SimpleString("from BraneEngine import *");
		pyMainMod = PyImport_ImportModule("__main__");
		PyObject* mod = PyState_FindModule(&engineMod);
	}
	else {
		Console::error("Python initialize failed");
	}
}

void PythonManager::run(const string & code)
{
	if (isInit)
		PyRun_SimpleString(code.c_str());
	else
		Console::error("Python not initialized");
}

void PythonManager::end()
{
	if (isInit)
		Py_Finalize();
}

PyObject * PythonManager::getMainModule()
{
	return pyMainMod;
}

void PythonManager::regist(PyObject * obj)
{
	auto iter = runtimeObjects.find(obj->ob_type);
	set<PyObject*>* list;
	if (iter == runtimeObjects.end())
		list = &runtimeObjects.emplace(make_pair(obj->ob_type, set<PyObject*>())).first->second;
	else
		list = &iter->second;
	list->insert(obj);
}

void PythonManager::logoff(PyObject * obj)
{
	auto iter = runtimeObjects.find(obj->ob_type);
	if (iter != runtimeObjects.end())
		iter->second.erase(obj);
}

UtilityPy UtilityPy::instance;

PyObject * UtilityPy::getWorld(PyObject * self, PyObject * args)
{
	(void)self;
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_BadArgument();
		return NULL;
	}
	World* w = Engine::getCurrentWorld();
	if (w == NULL) {
		PyErr_SetString(PyExc_RuntimeError, "no world is load");
		return NULL;
	}
	PyObject* obj = PyObject_CallFunction((PyObject*)&WorldPy::Type, "L", w);
	if (obj == NULL)
		PyErr_SetString(PyExc_RuntimeError, "construct World failed");
	return obj;
}

PyObject * UtilityPy::getInput(PyObject * self, PyObject * args)
{
	(void)self;
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_BadArgument();
		return NULL;
	}
	World* w = Engine::getCurrentWorld();
	if (w == NULL) {
		PyErr_SetString(PyExc_RuntimeError, "no world is load");
		return NULL;
	}
	PyObject* obj = PyObject_CallFunction((PyObject*)&InputPy::Type, "L", &Engine::getInput());
	if (obj == NULL)
		PyErr_SetString(PyExc_RuntimeError, "construct Input failed");
	return obj;
}

PyObject * UtilityPy::destroyObject(PyObject * self, PyObject * args)
{
	char* name = NULL;
	if (!PyArg_ParseTuple(args, "s", &name)) {
		PyErr_BadArgument();
		return NULL;
	}
	World& world = *Engine::getCurrentWorld();
	world.destroyObject(name);
	Py_RETURN_NONE;
}

PyObject * UtilityPy::getAllAssetName(PyObject * self, PyObject * args)
{
	(void)self;
	char* type = NULL;
	if (!PyArg_ParseTuple(args, "s", &type))
		return NULL;
	if (type == NULL) {
		PyErr_BadArgument();
		return NULL;
	}
	PyObject* list = PyList_New(0);
	AssetInfo* info = AssetManager::getAssetInfo(type);
	if (info == NULL) {
		return list;
	}
	for (auto b = info->assets.begin(), e = info->assets.end(); b != e; b++) {
		PyList_Append(list, PyUnicode_FromString(b->first.c_str()));
	}
	return list;
}

PyObject * UtilityPy::spawnMeshActor(PyObject * self, PyObject * args)
{
	(void)self;
	char* meshName = NULL;
	char* matName = NULL;
	char* name = NULL;
	char* collision = NULL;
	float mass = 0;
	int ptype = 0;
	PyObject* _pos = NULL;
	PyObject* _sca = NULL;
	int s = PyTuple_Size(args);
	if (s == 4) {
		if (!PyArg_ParseTuple(args, "ssOs", &meshName, &matName, &_pos, &name))
			return NULL;
	}
	else if (s == 5) {
		if (!PyArg_ParseTuple(args, "ssOss", &meshName, &matName, &_pos, &name, &collision))
			return NULL;
	}
	else if (s == 6) {
		if (!PyArg_ParseTuple(args, "ssOsfi", &meshName, &matName, &_pos, &name, &mass, &ptype))
			return NULL;
	}
	else if (s == 7) {
		if (!PyArg_ParseTuple(args, "ssOOsfi", &meshName, &matName, &_pos, &_sca, &name, &mass, &ptype))
			return NULL;
	}
	else
		return NULL;
	if (meshName == NULL || matName == NULL || name == NULL) {
		PyErr_BadArgument();
		return PyLong_FromLong(0);
	}
	if (strlen(name) == 0) {
		PyErr_SetString(PyExc_TypeError, "Empty name");
		return PyLong_FromLong(0);
	}
	Mesh* m = getAsset<Mesh>("Mesh", meshName);
	if (m == NULL) {
		PyErr_SetObject(PyExc_TypeError, PyUnicode_FromFormat("Mesh '%s' is not found", meshName));
		return PyLong_FromLong(0);
	}
	Material* mat = getAsset<Material>("Material", matName);
	if (mat == NULL) {
		PyErr_SetObject(PyExc_TypeError, PyUnicode_FromFormat("Material '%s' is not found", matName));
		return PyLong_FromLong(0);
	}
	Vec3Py::Vec3* sca = NULL;
	if (PyObject_TypeCheck(_sca, &Vec3Py::Type)) {
		sca = (Vec3Py::Vec3*)_sca;
	}
	MeshActor* ma;
	if (collision != NULL) {
		Mesh* cm = getAsset<Mesh>("Mesh", collision);
		if (cm == NULL) {
			PyErr_SetObject(PyExc_TypeError, PyUnicode_FromFormat("Mesh '%s' is not found", collision));
			return PyLong_FromLong(0);
		}
		ma = new MeshActor(*m, *mat, { 100, DYNAMIC }, *cm, name);
	}
	else
		ma = new MeshActor(*m, *mat, { mass, (PhysicalType)ptype }, *m, name, SIMPLE,
			sca == NULL ? Vector3f::Ones() : Vector3f{ sca->x, sca->y, sca->z });
	World& world = *Engine::getCurrentWorld();
	world += ma;
	if (PyObject_TypeCheck(_pos, &Vec3Py::Type)) {
		Vec3Py::Vec3* pos = (Vec3Py::Vec3*)_pos;
		ma->setPosition({ pos->x, pos->y, pos->z });
	}
	return PyLong_FromLongLong((long long)ma);
}

PyObject* UtilityPy::spawnPrefab(PyObject* self, PyObject* args)
{
	(void)self;
	char* prefabPath = NULL;
	char* objectName = NULL;
	if (!PyArg_ParseTuple(args, "ss", &prefabPath, &objectName)) {
		PyErr_SetString(PyExc_TypeError, "spawnPrefab(path: string, name: string)");
		Py_RETURN_NONE;
	}
	if (prefabPath == NULL) {
		PyErr_BadArgument();
		Py_RETURN_NONE;
	}
	SerializationInfo* prefab = getAssetByPath<SerializationInfo>(prefabPath);
	if (prefab == NULL) {
		PyErr_SetObject(PyExc_ResourceWarning, PyUnicode_FromFormat("Prefab '%s' is not found", prefabPath));
		Py_RETURN_NONE;
	}
	if (prefab->serialization == NULL) {
		PyErr_SetObject(PyExc_TypeError, PyUnicode_FromFormat("Prefab '%s' type unknown", prefabPath));
		Py_RETURN_NONE;
	}
	else if (!prefab->serialization->isChildOf(Object::ObjectSerialization::serialization)) {
		PyErr_SetObject(PyExc_TypeError, PyUnicode_FromFormat("Prefab '%s' is not Object", prefabPath));
		Py_RETURN_NONE;
	}
	Serializable* ser = prefab->serialization->instantiate(*prefab);
	Object* object = dynamic_cast<Object*>(ser);
	object->name = objectName;
	if (object == NULL) {
		if (ser != NULL)
			delete ser;
		PyErr_SetObject(PyExc_RuntimeError, PyUnicode_FromFormat("Prefab '%s' instantiate failed", prefabPath));
		Py_RETURN_NONE;
	}
	if (!object->deserialize(*prefab)) {
		PyErr_SetObject(PyExc_RuntimeWarning, PyUnicode_FromFormat("Prefab '%s' deserialize failed", prefabPath));
	}
	Engine::getCurrentWorld()->addObject((Object*)object);
	return PyLong_FromLongLong((long long)object);
}

PyObject * UtilityPy::setGravity(PyObject * self, PyObject * args)
{
	int size = PyTuple_Size(args);
	Vector3f v;
	if (size == 1) {
		PyObject* g = PyTuple_GetItem(args, 0);
		if (!PyObject_TypeCheck(g, &Vec3Py::Type)) {
			PyErr_SetString(PyExc_TypeError, "setGravity(Vec3) or setGravity(num, num, num)");
			Py_RETURN_NONE;
		}
		v = { ((Vec3Py::Vec3*)g)->x, ((Vec3Py::Vec3*)g)->y, ((Vec3Py::Vec3*)g)->z };
	}
	else if (size == 3) {
		if (!PyArg_ParseTuple(args, "fff", &v.x(), &v.y(), &v.z())) {
			PyErr_SetString(PyExc_TypeError, "setGravity(Vec3) or setGravity(num, num, num)");
			Py_RETURN_NONE;
		}
	}
	else {
		PyErr_SetString(PyExc_TypeError, "setGravity(Vec3) or setGravity(num, num, num)");
		Py_RETURN_NONE;
	}
	World& world = *Engine::getCurrentWorld();
	world.physicalWorld.setGravity(v);
	Py_RETURN_NONE;
}

PyObject * UtilityPy::msgBox(PyObject * self, PyObject * args)
{
	string str;
	char* title = NULL;
	Py_ssize_t size = PyTuple_Size(args);
	if (size > 0) {
		PyObject* t = PyTuple_GetItem(args, 0);
		title = t->ob_type->tp_str != 0 ? PyUnicode_AsUTF8(t->ob_type->tp_str(t)) : "";
	}
	else {
		PyErr_SetString(PyExc_TypeError, "msgBox(title, ...)");
		return NULL;
	}
	for (int i = 1; i < size; i++) {
		PyObject* obj = PyTuple_GetItem(args, i);
		str += obj->ob_type->tp_str != 0 ? PyUnicode_AsUTF8(obj->ob_type->tp_str(obj)) : "";
		if (i != size - 1)
			str += ' ';
	}
	MessageBoxA(Engine::windowContext.hwnd, str.c_str(), title, MB_OK);
	Py_RETURN_NONE;
}

UtilityPy::UtilityPy()
{
	PythonManager::methods.push_back({ "getWorld", UtilityPy::getWorld, METH_VARARGS, "get World pointer" });
	PythonManager::methods.push_back({ "getInput", UtilityPy::getInput, METH_VARARGS, "get input object of the world" });
	PythonManager::methods.push_back({ "destroyObject", UtilityPy::destroyObject, METH_VARARGS, "destroy object by name" });
	PythonManager::methods.push_back({ "getAllAssetName", UtilityPy::getAllAssetName, METH_VARARGS, "return all asset name of a type as a list" });
	PythonManager::methods.push_back({ "spawnMeshActor", UtilityPy::spawnMeshActor, METH_VARARGS, "spawn a MeshActor in the world" });
	PythonManager::methods.push_back({ "spawnPrefab", UtilityPy::spawnPrefab, METH_VARARGS, "spawn a Prefab in the world" });
	PythonManager::methods.push_back({ "setGravity", UtilityPy::setGravity, METH_VARARGS, "set gravity of current world" });
	PythonManager::methods.push_back({ "msgBox", UtilityPy::msgBox, METH_VARARGS, "popup windows standard messagebox" });
}

PyModuleDef ConsolePy::logMod = { PyModuleDef_HEAD_INIT, "BraneEngine.Log", NULL, -1, ConsolePy::logMethods };
PyMethodDef ConsolePy::logMethods[3] = {
	{ "write", ConsolePy::pyLogWrite, METH_VARARGS, "write" },
	{ "flush", ConsolePy::pyLogFlush, METH_VARARGS, "flush" },
	{ NULL }
};
PyModuleDef ConsolePy::errorMod = { PyModuleDef_HEAD_INIT, "BraneEngine.Err", NULL, -1, ConsolePy::errorMethods };
PyMethodDef ConsolePy::errorMethods[3] = {
	{ "write", ConsolePy::pyErrWrite, METH_VARARGS, "write" },
	{ "flush", ConsolePy::pyErrFlush, METH_VARARGS, "flush" },
	{ NULL }
};
ConsolePy ConsolePy::instance;

PyObject * ConsolePy::log(PyObject * self, PyObject * args)
{
	string str;
	Py_ssize_t size = PyTuple_Size(args);
	for (int i = 0; i < size; i++) {
		PyObject* obj = PyTuple_GetItem(args, i);
		str += obj->ob_type->tp_str != 0 ? PyUnicode_AsUTF8(obj->ob_type->tp_str(obj)) : "";
		if (i != size - 1)
			str += ' ';
	}
	if (!str.empty())
		Console::log(str.c_str());
	Py_RETURN_NONE;
}

PyObject * ConsolePy::warn(PyObject * self, PyObject * args)
{
	string str;
	Py_ssize_t size = PyTuple_Size(args);
	for (int i = 0; i < size; i++) {
		PyObject* obj = PyTuple_GetItem(args, i);
		str += obj->ob_type->tp_str != 0 ? PyUnicode_AsUTF8(obj->ob_type->tp_str(obj)) : "";
		if (i != size - 1)
			str += ' ';
	}
	if (!str.empty())
		Console::warn(str.c_str());
	Py_RETURN_NONE;
}

PyObject * ConsolePy::error(PyObject * self, PyObject * args)
{
	string str;
	Py_ssize_t size = PyTuple_Size(args);
	for (int i = 0; i < size; i++) {
		PyObject* obj = PyTuple_GetItem(args, i);
		str += obj->ob_type->tp_str != 0 ? PyUnicode_AsUTF8(obj->ob_type->tp_str(obj)) : "";
		if (i != size - 1)
			str += ' ';
	}
	if (!str.empty())
		Console::error(str.c_str());
	Py_RETURN_NONE;
}

PyObject * ConsolePy::pyLogWrite(PyObject * self, PyObject * args)
{
	(void)self;
	char *what;
	if (!PyArg_ParseTuple(args, "s", &what))
	{
		return NULL;
	}
	Console::pyLogWrite(what);
	Py_RETURN_NONE;
}

PyObject * ConsolePy::pyLogFlush(PyObject * self, PyObject * args)
{
	(void)self;
	(void)args;
	Console::pyLogFlush();
	Py_RETURN_NONE;
}

PyObject * ConsolePy::pyErrWrite(PyObject * self, PyObject * args)
{
	(void)self;
	char *what;
	if (!PyArg_ParseTuple(args, "s", &what))
	{
		return NULL;
	}
	Console::pyErrWrite(what);
	Py_RETURN_NONE;
}

PyObject * ConsolePy::pyErrFlush(PyObject * self, PyObject * args)
{
	(void)self;
	(void)args;
	Console::pyErrFlush();
	Py_RETURN_NONE;
}

ConsolePy::ConsolePy()
{
	PythonManager::methods.push_back({ "log", ConsolePy::log, METH_VARARGS, "log text to Console" });
	PythonManager::methods.push_back({ "warn", ConsolePy::warn, METH_VARARGS, "warn text to Console" });
	PythonManager::methods.push_back({ "error", ConsolePy::error, METH_VARARGS, "error text to Console" });
	PyImport_AppendInittab("BraneEngineLog", []()->PyObject* {
		PyObject *m = PyModule_Create(&logMod);
		if (m == NULL)
			return NULL;
		PySys_SetObject("stdout", m);
		return m;
	});
	PyImport_AppendInittab("BraneEngineErr", []()->PyObject* {
		PyObject *m = PyModule_Create(&errorMod);
		if (m == NULL)
			return NULL;
		PySys_SetObject("stderr", m);
		return m;
	});
}

PyTypeObject BehaviorPy::Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"BraneEngine.Behavior",                         /* tp_name */
	sizeof(BehaviorPy::Behavior),                       /* tp_basicsize */
	0,                                          /* tp_itemsize */
	(destructor)BehaviorPy::__dealloc__,           /* tp_dealloc */
	(printfunc)0,                               /* tp_print */
	0,                                          /* tp_getattr */
	0,                                          /* tp_setattr */
	0,                                          /* tp_reserved */
	0,                                          /* tp_repr */
	0,                 /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	(reprfunc)BehaviorPy::__str__,                 /* tp_str */
	0,                                          /* tp_getattro */
	0,                                          /* tp_setattro */
	0,                                          /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE,                         /* tp_flags */
	"Behavior Class",                               /* tp_doc */
	0,                                          /* tp_traverse */
	0,                                          /* tp_clear */
	0,                                          /* tp_richcompare */
	0,                                          /* tp_weaklistoffset */
	0,                                          /* tp_iter */
	0,                                          /* tp_iternext */
	0,                                          /* tp_methods */
	0,                     /* tp_members */
	0,                                          /* tp_getset */
	&PyBaseObject_Type,                         /* tp_base */
	0,                                          /* tp_dict */
	0,                                          /* tp_descr_get */
	0,                                          /* tp_descr_set */
	0,                                          /* tp_dictoffset */
	(initproc)BehaviorPy::__init__,                /* tp_init */
	0,                                          /* tp_alloc */
	(newfunc)BehaviorPy::__new__,                  /* tp_new */
};

BehaviorPy BehaviorPy::instance;

void BehaviorPy::__dealloc__(Behavior* self)
{
}

PyObject* BehaviorPy::__new__(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	Behavior* self;

	self = (Behavior*)type->tp_alloc(type, 0);
	if (self == NULL) {
		Console::pyError("Behavior New error");
	}

	return (PyObject*)self;
}

int BehaviorPy::__init__(Behavior* self, PyObject* args, PyObject* kwds)
{
	if (self == NULL) {
		Console::pyError("Behavior Init error");
	}

	return 0;
}

PyObject* BehaviorPy::__str__(Behavior* behavior)
{
	return PyUnicode_FromFormat("Behavior()");
}

BehaviorPy::BehaviorPy()
{
	PythonManager::typeObjects.push_back(make_pair("Behavior", &instance.Type));
}

PyNumberMethods Vec3Py::NumMethods = {
	(binaryfunc)Vec3Py::__add__, // nb_add
	(binaryfunc)Vec3Py::__sub__, // nb_subtract
	(binaryfunc)0, // nb_multiply
	(binaryfunc)0, // nb_remainder
	(binaryfunc)0, // nb_divmod
	(ternaryfunc)0, // nb_power
	(unaryfunc)0, // nb_negative
	(unaryfunc)0, // nb_positive
	(unaryfunc)0, // nb_absolute
	(inquiry)0, // nb_bool
	(unaryfunc)0, // nb_invert
	(binaryfunc)0, // nb_lshift
	(binaryfunc)0, // nb_rshift
	(binaryfunc)0, // nb_and
	(binaryfunc)0, // nb_xor
	(binaryfunc)0, // nb_or
	(unaryfunc)0, // nb_int
	(void *)0, // nb_reserved
	(unaryfunc)0, // nb_float

	(binaryfunc)0, // nb_inplace_add
	(binaryfunc)0, // nb_inplace_subtract
	(binaryfunc)0, // nb_inplace_multiply
	(binaryfunc)0, // nb_inplace_remainder
	(ternaryfunc)0, // nb_inplace_power
	(binaryfunc)0, // nb_inplace_lshift
	(binaryfunc)0, // nb_inplace_rshift
	(binaryfunc)0, // nb_inplace_and
	(binaryfunc)0, // nb_inplace_xor
	(binaryfunc)0, // nb_inplace_or

	(binaryfunc)0, // nb_floor_divide
	(binaryfunc)0, // nb_true_divide
	(binaryfunc)0, // nb_inplace_floor_divide
	(binaryfunc)0, // nb_inplace_true_divide

	(unaryfunc)0, // nb_index

	(binaryfunc)0, // nb_matrix_multiply
	(binaryfunc)0 // nb_inplace_matrix_multiply
};

PyMemberDef Vec3Py::Members[4] = {
	{ "x", T_FLOAT, offsetof(Vec3Py::Vec3, x), 0, "x value" },
	{ "y", T_FLOAT, offsetof(Vec3Py::Vec3, y), 0, "y value" },
	{ "z", T_FLOAT, offsetof(Vec3Py::Vec3, z), 0, "z value" },
	{ NULL }
};

PyMethodDef Vec3Py::Methods[6] = {
	{ "dot", Vec3Py::dot, METH_VARARGS, "dot(Vec3)" },
	{ "cross", Vec3Py::cross, METH_VARARGS, "cross(Vec3)" },
	{ "norm", Vec3Py::norm, METH_VARARGS, "norm() return length of Vec3" },
	{ "normalize", Vec3Py::normalize, METH_VARARGS, "normalize() return new normalized Vec3" },
	{ "normalized", Vec3Py::normalized, METH_VARARGS, "normalized() return this normalized Vec3" },
	{ NULL }
};

PyTypeObject Vec3Py::Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"BraneEngine.Vec3",                         /* tp_name */
	sizeof(Vec3Py::Vec3),                       /* tp_basicsize */
	0,                                          /* tp_itemsize */
	(destructor)Vec3Py::Vec3_dealloc,           /* tp_dealloc */
	(printfunc)0,                               /* tp_print */
	0,                                          /* tp_getattr */
	0,                                          /* tp_setattr */
	0,                                          /* tp_reserved */
	0,                                          /* tp_repr */
	&Vec3Py::NumMethods,                 /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	(reprfunc)Vec3Py::__str__,                 /* tp_str */
	0,                                          /* tp_getattro */
	0,                                          /* tp_setattro */
	0,                                          /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE,                         /* tp_flags */
	"Vec3 Class",                               /* tp_doc */
	0,                                          /* tp_traverse */
	0,                                          /* tp_clear */
	0,                                          /* tp_richcompare */
	0,                                          /* tp_weaklistoffset */
	0,                                          /* tp_iter */
	0,                                          /* tp_iternext */
	Vec3Py::Methods,                                          /* tp_methods */
	Vec3Py::Members,                     /* tp_members */
	0,                                          /* tp_getset */
	&PyBaseObject_Type,                         /* tp_base */
	0,                                          /* tp_dict */
	0,                                          /* tp_descr_get */
	0,                                          /* tp_descr_set */
	0,                                          /* tp_dictoffset */
	(initproc)Vec3Py::Vec3_init,                /* tp_init */
	0,                                          /* tp_alloc */
	(newfunc)Vec3Py::Vec3_new,                  /* tp_new */
};

Vec3Py Vec3Py::instance;

void Vec3Py::Vec3_dealloc(Vec3 * self)
{
	Py_TYPE(self)->tp_free((PyObject*)self);
}

PyObject * Vec3Py::Vec3_new(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
	Vec3 *self;

	self = (Vec3 *)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->x = 0;
		self->y = 0;
		self->z = 0;
	}
	else {
		Console::pyError("Vec3 New error");
	}

	return (PyObject *)self;
}

int Vec3Py::Vec3_init(Vec3 * self, PyObject * args, PyObject * kwds)
{
	if (self == NULL) {
		Console::pyError("Vec3 Init error");
	}

	static char *kwlist[] = { "x", "y", "z", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "fff", kwlist,
		&self->x, &self->y, &self->z))
		return -1;

	return 0;
}

PyObject * Vec3Py::__str__(Vec3 * obj)
{
	char str[1024];
	sprintf_s(str, "Vec3(%f, %f, %f)", obj->x, obj->y, obj->z);
	return PyUnicode_FromFormat(str);
}

Vec3Py::Vec3 * Vec3Py::__add__(Vec3 * a, Vec3 * b)
{
	Vec3 *self;
	self = (Vec3*)Type.tp_alloc(&Type, 0);
	if (self != NULL) {
		self->x = a->x + b->x;
		self->y = a->y + b->y;
		self->z = a->z + b->z;
	}
	return self;
}

Vec3Py::Vec3 * Vec3Py::__sub__(Vec3 * a, Vec3 * b)
{
	Vec3 *self;
	self = (Vec3*)Type.tp_alloc(&Type, 0);
	if (self != NULL) {
		self->x = a->x - b->x;
		self->y = a->y - b->y;
		self->z = a->z - b->z;
	}
	return self;
}

PyObject * Vec3Py::dot(PyObject * self, PyObject * args)
{
	PyObject* obj = NULL;
	if (!PyArg_ParseTuple(args, "O", &obj)) {
		PyErr_SetString(PyExc_TypeError, "dot(Vec3)");
		return NULL;
	}
	Vec3* a = (Vec3*)self;
	Vec3* b = cast(obj);
	if (b == NULL) {
		PyErr_SetString(PyExc_TypeError, "dot(Vec3)");
		return NULL;
	}
	return PyFloat_FromDouble(a->x * b->x + a->y * b->y + a->z * b->z);
}

PyObject * Vec3Py::cross(PyObject * self, PyObject * args)
{
	PyObject* obj = NULL;
	if (!PyArg_ParseTuple(args, "O", &obj)) {
		PyErr_SetString(PyExc_TypeError, "cross(Vec3)");
		return NULL;
	}
	Vec3* a = (Vec3*)self;
	Vec3* b = cast(obj);
	if (b == NULL) {
		PyErr_SetString(PyExc_TypeError, "cross(Vec3)");
		return NULL;
	}
	Vec3* c = (Vec3*)Type.tp_alloc(&Type, 0);
	if (c != NULL) {
		c->x = a->y * b->z - a->z * b->y;
		c->y = a->z * b->x - a->x * b->z;
		c->z = a->x * b->y - a->y * b->x;
	}
	return (PyObject*)c;
}

PyObject * Vec3Py::norm(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "norm()");
		return NULL;
	}
	Vec3* a = (Vec3*)self;
	return PyFloat_FromDouble(sqrt(a->x * a->x + a->y * a->y + a->z * a->z));
}

PyObject * Vec3Py::normalize(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "normalize()");
		return NULL;
	}
	Vec3* a = (Vec3*)self;
	Vec3* b = (Vec3*)Type.tp_alloc(&Type, 0);
	float len = sqrt(a->x * a->x + a->y * a->y + a->z * a->z);
	if (b != NULL) {
		b->x = a->x / len;
		b->y = a->y / len;
		b->z = a->z / len;
	}
	return (PyObject*)b;
}

PyObject * Vec3Py::normalized(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "normalized()");
		return NULL;
	}
	Vec3* a = (Vec3*)self;
	float len = sqrt(a->x * a->x + a->y * a->y + a->z * a->z);
	a->x /= len;
	a->y /= len;
	a->z /= len;
	Py_RETURN_NONE;
}

Vec3Py::Vec3Py()
{
	PythonManager::typeObjects.push_back(make_pair("Vec3", &instance.Type));
}

PyObject * Vec3Py::New(float x, float y, float z)
{
	PyObject *obj = PyObject_CallFunction((PyObject*)&Type, "fff", x, y, z);
	if (obj == NULL) {
		Console::pyError("Vec3 instantiate failed");
	}
	return obj;
}

PyObject * Vec3Py::New(const Vector3f & v)
{
	PyObject *obj = PyObject_CallFunction((PyObject*)&Type, "fff", v.x(), v.y(), v.z());
	if (obj == NULL) {
		Console::pyError("Vec3 instantiate failed");
	}
	return obj;
}

Vec3Py::Vec3 * Vec3Py::cast(PyObject * pobj)
{
	if (!PyObject_TypeCheck(pobj, &Vec3Py::Type))
		return NULL;
	return (Vec3*)pobj;
}

PyNumberMethods QuatPy::NumMethods = {
	(binaryfunc)0, // nb_add
	(binaryfunc)0, // nb_subtract
	(binaryfunc)QuatPy::__mul__, // nb_multiply
	(binaryfunc)0, // nb_remainder
	(binaryfunc)0, // nb_divmod
	(ternaryfunc)0, // nb_power
	(unaryfunc)0, // nb_negative
	(unaryfunc)0, // nb_positive
	(unaryfunc)0, // nb_absolute
	(inquiry)0, // nb_bool
	(unaryfunc)0, // nb_invert
	(binaryfunc)0, // nb_lshift
	(binaryfunc)0, // nb_rshift
	(binaryfunc)0, // nb_and
	(binaryfunc)0, // nb_xor
	(binaryfunc)0, // nb_or
	(unaryfunc)0, // nb_int
	(void *)0, // nb_reserved
	(unaryfunc)0, // nb_float

	(binaryfunc)0, // nb_inplace_add
	(binaryfunc)0, // nb_inplace_subtract
	(binaryfunc)0, // nb_inplace_multiply
	(binaryfunc)0, // nb_inplace_remainder
	(ternaryfunc)0, // nb_inplace_power
	(binaryfunc)0, // nb_inplace_lshift
	(binaryfunc)0, // nb_inplace_rshift
	(binaryfunc)0, // nb_inplace_and
	(binaryfunc)0, // nb_inplace_xor
	(binaryfunc)0, // nb_inplace_or

	(binaryfunc)0, // nb_floor_divide
	(binaryfunc)0, // nb_true_divide
	(binaryfunc)0, // nb_inplace_floor_divide
	(binaryfunc)0, // nb_inplace_true_divide

	(unaryfunc)0, // nb_index

	(binaryfunc)0, // nb_matrix_multiply
	(binaryfunc)0 // nb_inplace_matrix_multiply
};

PyMemberDef QuatPy::Members[5] = {
	{ "x", T_FLOAT, offsetof(QuatPy::Quat, x), 0, "x value" },
	{ "y", T_FLOAT, offsetof(QuatPy::Quat, y), 0, "y value" },
	{ "z", T_FLOAT, offsetof(QuatPy::Quat, z), 0, "z value" },
	{ "w", T_FLOAT, offsetof(QuatPy::Quat, w), 0, "w value" },
	{ NULL }
};

PyMethodDef QuatPy::Methods[4] = {
	{ "dot", QuatPy::dot, METH_VARARGS, "dot(Quat)" },
	{ "slerp", QuatPy::slerp, METH_VARARGS, "slerp(Quat, float)" },
	{ "toEular", QuatPy::toEular, METH_VARARGS, "toEular() return Vec3" },
	{ NULL }
};

PyTypeObject QuatPy::Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"BraneEngine.Quat",                         /* tp_name */
	sizeof(QuatPy::Quat),                       /* tp_basicsize */
	0,                                          /* tp_itemsize */
	(destructor)QuatPy::__dealloc__,           /* tp_dealloc */
	(printfunc)0,                               /* tp_print */
	0,                                          /* tp_getattr */
	0,                                          /* tp_setattr */
	0,                                          /* tp_reserved */
	0,                                          /* tp_repr */
	&QuatPy::NumMethods,                 /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	(reprfunc)QuatPy::__str__,                 /* tp_str */
	0,                                          /* tp_getattro */
	0,                                          /* tp_setattro */
	0,                                          /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE,                         /* tp_flags */
	"Quat Class",                               /* tp_doc */
	0,                                          /* tp_traverse */
	0,                                          /* tp_clear */
	0,                                          /* tp_richcompare */
	0,                                          /* tp_weaklistoffset */
	0,                                          /* tp_iter */
	0,                                          /* tp_iternext */
	QuatPy::Methods,                                          /* tp_methods */
	QuatPy::Members,                     /* tp_members */
	0,                                          /* tp_getset */
	&PyBaseObject_Type,                         /* tp_base */
	0,                                          /* tp_dict */
	0,                                          /* tp_descr_get */
	0,                                          /* tp_descr_set */
	0,                                          /* tp_dictoffset */
	(initproc)QuatPy::__init__,                /* tp_init */
	0,                                          /* tp_alloc */
	(newfunc)QuatPy::__new__,                  /* tp_new */
};

QuatPy QuatPy::instance;

void QuatPy::__dealloc__(Quat * self)
{
	Py_TYPE(self)->tp_free((PyObject*)self);
}

PyObject * QuatPy::__new__(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
	Quat *self;

	self = (Quat *)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->x = 0;
		self->y = 0;
		self->z = 0;
		self->w = 0;
	}
	else {
		Console::pyError("Quat New error");
	}

	return (PyObject *)self;
}

int QuatPy::__init__(Quat * self, PyObject * args, PyObject * kwds)
{
	if (self == NULL) {
		Console::pyError("Quat Init error");
	}

	static char *kwlist[] = { "x", "y", "z", "w", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "ffff", kwlist,
		&self->x, &self->y, &self->z, &self->w))
		return -1;

	return 0;
}

PyObject * QuatPy::__str__(Quat * obj)
{
	char str[1024];
	sprintf_s(str, "Quat(%f, %f, %f, %f)", obj->x, obj->y, obj->z, obj->w);
	return PyUnicode_FromFormat(str);
	return PyUnicode_FromFormat("Quat(%f, %f, %f, %f)", obj->x, obj->y, obj->z, obj->w);
}

QuatPy::Quat * QuatPy::__mul__(Quat * a, Quat * b)
{
	Quat *self;
	self = (Quat*)Type.tp_alloc(&Type, 0);
	if (self != NULL) {
		self->w = a->w * b->w - a->x * b->x - a->y * b->y - a->z * b->z;
		self->x = a->w * b->x + a->x * b->w + a->y * b->z - a->z * b->y;
		self->y = a->w * b->y - a->x * b->z + a->y * b->w + a->z * b->x;
		self->z = a->w * b->z + a->x * b->y - a->y * b->x + a->z * b->w;
	}
	return self;
}

PyObject * QuatPy::dot(PyObject * self, PyObject * args)
{
	PyObject* obj;
	if (!PyArg_ParseTuple(args, "O", &obj)) {
		PyErr_SetString(PyExc_TypeError, "dot(Quat)");
		return NULL;
	}
	Quat* a = (Quat*)self;
	Quat* b = cast(obj);
	if (b == NULL) {
		PyErr_SetString(PyExc_TypeError, "dot(Quat)");
		return NULL;
	}
	return PyFloat_FromDouble(a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w);
}

PyObject * QuatPy::slerp(PyObject * self, PyObject * args)
{
	PyObject* obj;
	float value = 0;
	if (!PyArg_ParseTuple(args, "Of", &obj, &value)) {
		PyErr_SetString(PyExc_TypeError, "slerp(Quat, float)");
		return NULL;
	}
	Quat* a = (Quat*)self;
	Quat* b = cast(obj);
	if (b == NULL) {
		PyErr_SetString(PyExc_TypeError, "slerp(Quat, float)");
		return NULL;
	}
	Quaternionf aq = Quaternionf(a->w, a->x, a->y, a->z);
	Quaternionf bq = Quaternionf(b->w, b->x, b->y, b->z);
	aq = aq.slerp(value, bq);
	Quat* c = (Quat*)Type.tp_alloc(&Type, 0);
	if (c != NULL) {
		c->x = aq.x();
		c->y = aq.y();
		c->z = aq.z();
		c->w = aq.w();
	}
	return (PyObject*)c;
}

PyObject * QuatPy::toEular(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "toEular()");
		return NULL;
	}
	Quat* a = (Quat*)self;
	Quaternionf aq = Quaternionf(a->w, a->x, a->y, a->z);
	Vector3f vec3 = aq.toRotationMatrix().eulerAngles() / PI * 180;
	Vec3Py::Vec3* c = (Vec3Py::Vec3*)Vec3Py::Type.tp_alloc(&Vec3Py::Type, 0);
	if (c != NULL) {
		c->x = vec3.x();
		c->y = vec3.y();
		c->z = vec3.z();
	}
	return (PyObject*)c;
}

QuatPy::QuatPy()
{
	PythonManager::typeObjects.push_back(make_pair("Quat", &instance.Type));
}

PyObject * QuatPy::New(float x, float y, float z, float w)
{
	PyObject *obj = PyObject_CallFunction((PyObject*)&Type, "ffff", x, y, z, w);
	if (obj == NULL) {
		Console::pyError("Quat instantiate failed");
	}
	return obj;
}

PyObject * QuatPy::New(const Quaternionf & q)
{
	PyObject *obj = PyObject_CallFunction((PyObject*)&Type, "ffff", q.x(), q.y(), q.z(), q.w());
	if (obj == NULL) {
		Console::pyError("Quat instantiate failed");
	}
	return obj;
}

QuatPy::Quat * QuatPy::cast(PyObject * pobj)
{
	if (!PyObject_TypeCheck(pobj, &QuatPy::Type))
		return NULL;
	return (Quat*)pobj;
}

PyMemberDef ShapePy::PyShape_Members[4] = {
	{ "minPos", T_OBJECT, offsetof(ShapePy::_Shape, minPos), 0, "min Position" },
	{ "maxPos", T_OBJECT, offsetof(ShapePy::_Shape, maxPos), 0, "max Position" },
	{ "type", T_OBJECT, offsetof(ShapePy::_Shape, type), 0, "shape type" },
	{ NULL }
};
PyTypeObject ShapePy::PyShape_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"BraneEngine.Shape",                        /* tp_name */
	sizeof(ShapePy::_Shape),                     /* tp_basicsize */
	0,                                          /* tp_itemsize */
	(destructor)ShapePy::Shape_dealloc,         /* tp_dealloc */
	(printfunc)0,                               /* tp_print */
	0,                                          /* tp_getattr */
	0,                                          /* tp_setattr */
	0,                                          /* tp_reserved */
	0,                                          /* tp_repr */
	0,                                          /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	(reprfunc)ShapePy::Shape_str,               /* tp_str */
	0,                                          /* tp_getattro */
	0,                                          /* tp_setattro */
	0,                                          /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | 
	Py_TPFLAGS_BASETYPE,                         /* tp_flags */
	"Shape Class",                              /* tp_doc */
	0,                                          /* tp_traverse */
	0,                                          /* tp_clear */
	0,                                          /* tp_richcompare */
	0,                                          /* tp_weaklistoffset */
	0,                                          /* tp_iter */
	0,                                          /* tp_iternext */
	0,                                          /* tp_methods */
	ShapePy::PyShape_Members,                   /* tp_members */
	0,                                          /* tp_getset */
	&PyBaseObject_Type,                         /* tp_base */
	0,                                          /* tp_dict */
	0,                                          /* tp_descr_get */
	0,                                          /* tp_descr_set */
	0,                                          /* tp_dictoffset */
	(initproc)ShapePy::Shape_init,              /* tp_init */
	0,                                          /* tp_alloc */
	(newfunc)ShapePy::Shape_new,                /* tp_new */
};
ShapePy ShapePy::instance;

void ShapePy::Shape_dealloc(_Shape * self)
{
	Py_XDECREF(self->minPos);
	Py_XDECREF(self->maxPos);
	Py_XDECREF(self->type);
	Py_TYPE(self)->tp_free((PyObject*)self);
}

PyObject * ShapePy::Shape_new(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
	_Shape *self;

	self = (_Shape *)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->minPos = (Vec3Py::Vec3*)Vec3Py::Type.tp_alloc(&Vec3Py::Type, 0);
		if (self->minPos == NULL) {
			Py_DECREF(self);
			return NULL;
		}
		self->minPos->x = 0;
		self->minPos->y = 0;
		self->minPos->z = 0;
		self->maxPos = (Vec3Py::Vec3*)Vec3Py::Type.tp_alloc(&Vec3Py::Type, 0);
		if (self->maxPos == NULL) {
			Py_DECREF(self);
			return NULL;
		}
		self->maxPos->x = 0;
		self->maxPos->y = 0;
		self->maxPos->z = 0;

		self->type = PyUnicode_FromFormat("box");
		if (self->type == NULL) {
			Py_DECREF(self);
			return NULL;
		}
	}

	return (PyObject *)self;
}

int ShapePy::Shape_init(_Shape * self, PyObject * args, PyObject * kwds)
{
	PyObject *minP = NULL, *maxP = NULL, *type = NULL, *tmp;

	static char *kwlist[] = { "minPos", "maxPos", "type", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OOO", kwlist,
		&minP, &maxP, &type))
		return -1;

	if (minP) {
		if (minP->ob_type != &Vec3Py::Type)
			return -1;
		tmp = (PyObject*)self->minPos;
		Py_INCREF(minP);
		self->minPos = (Vec3Py::Vec3*)minP;
		Py_XDECREF(tmp);
	}

	if (maxP) {
		if (maxP->ob_type != &Vec3Py::Type)
			return -1;
		tmp = (PyObject*)self->maxPos;
		Py_INCREF(maxP);
		self->maxPos = (Vec3Py::Vec3*)maxP;
		Py_XDECREF(tmp);
	}

	if (type) {
		tmp = self->type;
		Py_INCREF(type);
		self->type = type;
		Py_XDECREF(tmp);
	}

	return 0;
}

PyObject * ShapePy::Shape_str(_Shape * obj)
{
	return PyUnicode_FromFormat("Shape(%s)", PyUnicode_AsUTF8(obj->type));
}

ShapePy::ShapePy()
{
	PythonManager::typeObjects.push_back(make_pair("Shape", &instance.PyShape_Type));
}

PyMemberDef ObjectPy::Members[2] = {
	{ "nativeHandle", T_INT, offsetof(PyCPointer, nativeHandle), 0, "Cpp Native Pointer" },
	{ NULL }
};

PyMethodDef ObjectPy::Methods[6] = {
	{ "castTo", ObjectPy::castTo, METH_VARARGS, "up cast Object to other class" },
	{ "getName", ObjectPy::getName, METH_VARARGS, "get name of this object" },
	{ "getChild", ObjectPy::getChild, METH_VARARGS, "get child of this object by index or name" },
	{ "getChildCount", ObjectPy::getChildCount, METH_VARARGS, "get the number of childern of this object" },
	{ "destroy", ObjectPy::destroy, METH_VARARGS, "destroy this object" },
	{ NULL }
};

PyTypeObject ObjectPy::Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"BraneEngine.Object",            /* tp_name */
	sizeof(PyCPointer),                    /* tp_basicsize */
	0,                                          /* tp_itemsize */
	(destructor)&ObjectPy::__dealloc__,											/* tp_dealloc */
	0,                               /* tp_print */
	0,                                          /* tp_getattr */
	0,                                          /* tp_setattr */
	0,                                          /* tp_reserved */
	0,                                          /* tp_repr */
	0,                                          /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	(reprfunc)ObjectPy::__str__,               /* tp_str */
	0,                                          /* tp_getattro */
	0,                                          /* tp_setattro */
	0,                                          /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | 
	Py_TPFLAGS_BASETYPE,                         /* tp_flags */
	"SkeletonMeshActor Class",                              /* tp_doc */
	0,                                          /* tp_traverse */
	0,                                          /* tp_clear */
	0,                                          /* tp_richcompare */
	0,                                          /* tp_weaklistoffset */
	0,                                          /* tp_iter */
	0,                                          /* tp_iternext */
	ObjectPy::Methods,                                          /* tp_methods */
	ObjectPy::Members,										    /* tp_members */
	0,                                          /* tp_getset */
	&PyBaseObject_Type,                         /* tp_base */
	0,                                          /* tp_dict */
	0,                                          /* tp_descr_get */
	0,                                          /* tp_descr_set */
	0,                                          /* tp_dictoffset */
	(initproc)&ObjectPy::__init__,										       /* tp_init */
	0,                                          /* tp_alloc */
	(newfunc)&ObjectPy::__new__,											  /* tp_new */
};

ObjectPy ObjectPy::instance;

void ObjectPy::__dealloc__(PyObject * self)
{
	PythonManager::logoff(self);
	Py_TYPE(self)->tp_free(self);
}

PyObject * ObjectPy::__new__(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
	PyCPointer *self;

	self = (PyCPointer *)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->nativeHandle = 0;
		PythonManager::regist((PyObject*)self);
	}

	return (PyObject *)self;
}

int ObjectPy::__init__(PyCPointer * self, PyObject * args, PyObject * kwds)
{
	static char *kwlist[] = { "ptr", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "L", kwlist,
		&self->nativeHandle))
		return -1;

	return 0;
}

PyObject * ObjectPy::__str__(PyCPointer * obj)
{
	char str[1024];
	sprintf_s(str, "Object(%s)", ((Object*)obj->nativeHandle)->name.c_str());
	return PyUnicode_FromFormat(str);
}

PyObject * ObjectPy::castTo(PyObject * self, PyObject * args)
{
	PyObject* classType = NULL;
	if (!PyArg_ParseTuple(args, "O", &classType)) {
		PyErr_SetString(PyExc_TypeError, "castTo(Class)");
		return NULL;
	}
	if (!PyObject_TypeCheck(classType, &PyType_Type)) {
		PyErr_SetString(PyExc_TypeError, "castTo(Class)");
		return NULL;
	}
	if ((PyTypeObject*)classType != &Type && !PyType_IsSubtype((PyTypeObject*)classType, &Type)) {
		PyErr_SetString(PyExc_TypeError, "only subclass of BraneEngine.Object");
		return NULL;
	}
	Object* ptr = (Object*)((PyCPointer*)self)->nativeHandle;
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	PyObject *obj = PyObject_CallFunction(classType, "L", ptr);
	if (obj == NULL) {
		PyErr_SetString(PyExc_TypeError, "cast failed");
	}
	return obj;
}

PyObject * ObjectPy::getName(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "getName()");
		return NULL;
	}
	Object* ptr = (Object*)((PyCPointer*)self)->nativeHandle;
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	return PyUnicode_FromString(ptr->name.c_str());
}

PyObject * ObjectPy::getChild(PyObject * self, PyObject * args)
{
	int index = 0;
	char* name = NULL;
	if (PyArg_ParseTuple(args, "i", &index)) { }
	else if (PyArg_ParseTuple(args, "s", &name)) { }
	else {
		PyErr_SetString(PyExc_TypeError, "getChild(int) or getChild(string)");
		return NULL;
	}
	Object* ptr = (Object*)((PyCPointer*)self)->nativeHandle;
	Object* cobj = NULL;
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	if (name == NULL) {
		if (index < ptr->children.size())
			cobj = ptr->children[index];
	}
	else {
		for (auto b = ptr->children.begin(), e = ptr->children.end(); b != e; b++) {
			if ((*b)->name == name)
				cobj = *b;
		}
	}
	if (cobj == NULL)
		Py_RETURN_NONE;
	PyObject *obj = PyObject_CallFunction((PyObject*)&Type, "L", cobj);
	if (obj == NULL) {
		PyErr_SetString(PyExc_TypeError, "instantiate failed");
	}
	return obj;
}

PyObject * ObjectPy::getChildCount(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "getChildCount()");
		return NULL;
	}
	Object* ptr = (Object*)((PyCPointer*)self)->nativeHandle;
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	return PyLong_FromLong(ptr->children.size());
}

PyObject * ObjectPy::destroy(PyObject * self, PyObject * args)
{
	int v = 0;
	if (!PyArg_ParseTuple(args, "|p", &v)) {
		PyErr_SetString(PyExc_TypeError, "destroy(bool applyToChild = false)");
		return NULL;
	}
	Object* ptr = (Object*)((PyCPointer*)self)->nativeHandle;
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	ptr->destroy(v);
	Py_RETURN_NONE;
}

PyObject * ObjectPy::New(PyTypeObject * type, void * ptr)
{
	PyObject *obj = PyObject_CallFunction((PyObject *)type, "L", (long long)ptr);
	return obj;
}

ObjectPy::ObjectPy()
{
	PythonManager::typeObjects.push_back(make_pair("Object", &instance.Type));
}

PyMethodDef InputPy::Methods[14] = {
	{ "getKeyUp", InputPy::getKeyUp, METH_VARARGS, "getKeyUp(char)" },
	{ "getKeyDown", InputPy::getKeyDown, METH_VARARGS, "getKeyDown(char)" },
	{ "getKeyPress", InputPy::getKeyPress, METH_VARARGS, "getKeyPress(char)" },
	{ "getKeyRelease", InputPy::getKeyRelease, METH_VARARGS, "getKeyRelease(char)" },
	{ "getMousePos", InputPy::getMousePos, METH_VARARGS, "getMousePos()" },
	{ "getCursorPos", InputPy::getCursorPos, METH_VARARGS, "getCursorPos()" },
	{ "getMouseMove", InputPy::getMouseMove, METH_VARARGS, "getMouseMove()" },
	{ "getCursorMove", InputPy::getCursorMove, METH_VARARGS, "getCursorMove()" },
	{ "getMouseWheelValue", InputPy::getMouseWheelValue, METH_VARARGS, "getMouseWheelValue()" },
	{ "getMouseButtonUp", InputPy::getMouseButtonUp, METH_VARARGS, "getMouseButtonUp(char)" },
	{ "getMouseButtonDown", InputPy::getMouseButtonDown, METH_VARARGS, "getMouseButtonDown(char)" },
	{ "getMouseButtonPress", InputPy::getMouseButtonPress, METH_VARARGS, "getMouseButtonPress(char)" },
	{ "getMouseButtonRelease", InputPy::getMouseButtonRelease, METH_VARARGS, "getMouseButtonRelease(char)" },
	{ NULL }
};

PyTypeObject InputPy::Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"BraneEngine.Input",            /* tp_name */
	sizeof(PyCPointer),                    /* tp_basicsize */
	0,                                          /* tp_itemsize */
	0,											/* tp_dealloc */
	0,                               /* tp_print */
	0,                                          /* tp_getattr */
	0,                                          /* tp_setattr */
	0,                                          /* tp_reserved */
	0,                                          /* tp_repr */
	0,                                          /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	(reprfunc)InputPy::__str__,               /* tp_str */
	0,                                          /* tp_getattro */
	0,                                          /* tp_setattro */
	0,                                          /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE,                         /* tp_flags */
	"Input Class",                              /* tp_doc */
	0,                                          /* tp_traverse */
	0,                                          /* tp_clear */
	0,                                          /* tp_richcompare */
	0,                                          /* tp_weaklistoffset */
	0,                                          /* tp_iter */
	0,                                          /* tp_iternext */
	InputPy::Methods,                                          /* tp_methods */
	0,										    /* tp_members */
	0,                                          /* tp_getset */
	&PyBaseObject_Type,                         /* tp_base */
	0,                                          /* tp_dict */
	0,                                          /* tp_descr_get */
	0,                                          /* tp_descr_set */
	0,                                          /* tp_dictoffset */
	0,										       /* tp_init */
	0,                                          /* tp_alloc */
	0,											  /* tp_new */
};

InputPy InputPy::instance;

void InputPy::__dealloc__(PyObject* self)
{
	PythonManager::logoff(self);
	Py_TYPE(self)->tp_free(self);
}

PyObject* InputPy::__new__(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	PyCPointer* self;

	self = (PyCPointer*)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->nativeHandle = 0;
		PythonManager::regist((PyObject*)self);
	}

	return (PyObject*)self;
}

int InputPy::__init__(PyCPointer* self, PyObject* args, PyObject* kwds)
{
	static char* kwlist[] = { "ptr", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "L", kwlist,
		&self->nativeHandle))
		return -1;

	return 0;
}

PyObject * InputPy::__str__(PyCPointer * obj)
{
	char str[1024];
	sprintf_s(str, "Input(%s)", ((Object*)obj->nativeHandle)->name.c_str());
	return PyUnicode_FromFormat(str);
}

PyObject * InputPy::getKeyUp(PyObject * self, PyObject * args)
{
	char k;
	if (!PyArg_ParseTuple(args, "b", &k)) {
		PyErr_SetString(PyExc_TypeError, "getKeyUp(char)");
		return NULL;
	}
	Input* input = ObjectPy::cast<Input>(self);
	if (input == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	if (input->getKeyUp(k))
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
}

PyObject * InputPy::getKeyDown(PyObject * self, PyObject * args)
{
	char k;
	if (!PyArg_ParseTuple(args, "b", &k)) {
		PyErr_SetString(PyExc_TypeError, "getKeyDown(char)");
		return NULL;
	}
	Input* input = ObjectPy::cast<Input>(self);
	if (input == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	if (input->getKeyDown(k))
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
}

PyObject * InputPy::getKeyPress(PyObject * self, PyObject * args)
{
	char k;
	if (!PyArg_ParseTuple(args, "b", &k)) {
		PyErr_SetString(PyExc_TypeError, "getKeyPress(char)");
		return NULL;
	}
	Input* input = ObjectPy::cast<Input>(self);
	if (input == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	if (input->getKeyPress(k))
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
}

PyObject * InputPy::getKeyRelease(PyObject * self, PyObject * args)
{
	char k;
	if (!PyArg_ParseTuple(args, "b", &k)) {
		PyErr_SetString(PyExc_TypeError, "getKeyRelease(char)");
		return NULL;
	}
	Input* input = ObjectPy::cast<Input>(self);
	if (input == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	if (input->getKeyRelease(k))
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
}

PyObject * InputPy::getMousePos(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "getMousePos()");
		return NULL;
	}
	Input* input = ObjectPy::cast<Input>(self);
	if (input == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	Unit2Di pos = input->getMousePos();
	return Vec3Py::New(pos.x, pos.y, 0);
}

PyObject * InputPy::getCursorPos(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "getCursorPos()");
		return NULL;
	}
	Input* input = ObjectPy::cast<Input>(self);
	if (input == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	Unit2Di pos = input->getCursorPos();
	return Vec3Py::New(pos.x, pos.y, 0);
}

PyObject * InputPy::getMouseMove(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "getCursorPos()");
		return NULL;
	}
	Input* input = ObjectPy::cast<Input>(self);
	if (input == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	Unit2Di pos = input->getCursorPos();
	return Vec3Py::New(pos.x, pos.y, 0);
}

PyObject * InputPy::getCursorMove(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "getCursorMove()");
		return NULL;
	}
	Input* input = ObjectPy::cast<Input>(self);
	if (input == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	Unit2Di pos = input->getCursorMove();
	return Vec3Py::New(pos.x, pos.y, 0);
}

PyObject * InputPy::getMouseWheelValue(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "getMouseWheelValue()");
		return NULL;
	}
	Input* input = ObjectPy::cast<Input>(self);
	if (input == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	return PyLong_FromLong(input->getMouseWheelValue());
}

PyObject * InputPy::getMouseButtonUp(PyObject * self, PyObject * args)
{
	char k;
	if (!PyArg_ParseTuple(args, "b", &k)) {
		PyErr_SetString(PyExc_TypeError, "getMouseButtonUp(char)");
		return NULL;
	}
	Input* input = ObjectPy::cast<Input>(self);
	if (input == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	char m = log2f(k);
	if (m < 3 && input->getMouseButtonUp((MouseButtonEnum)m))
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
}

PyObject * InputPy::getMouseButtonDown(PyObject * self, PyObject * args)
{
	char k;
	if (!PyArg_ParseTuple(args, "b", &k)) {
		PyErr_SetString(PyExc_TypeError, "getMouseButtonDown(char)");
		return NULL;
	}
	Input* input = ObjectPy::cast<Input>(self);
	if (input == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	char m = log2f(k);
	if (m < 3 && input->getMouseButtonUp((MouseButtonEnum)m))
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
}

PyObject * InputPy::getMouseButtonPress(PyObject * self, PyObject * args)
{
	char k;
	if (!PyArg_ParseTuple(args, "b", &k)) {
		PyErr_SetString(PyExc_TypeError, "getMouseButtonPress(char)");
		return NULL;
	}
	Input* input = ObjectPy::cast<Input>(self);
	if (input == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	char m = log2f(k);
	if (m < 3 && input->getMouseButtonUp((MouseButtonEnum)m))
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
}

PyObject * InputPy::getMouseButtonRelease(PyObject * self, PyObject * args)
{
	char k;
	if (!PyArg_ParseTuple(args, "b", &k)) {
		PyErr_SetString(PyExc_TypeError, "getMouseButtonRelease(char)");
		return NULL;
	}
	Input* input = ObjectPy::cast<Input>(self);
	if (input == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	char m = log2f(k);
	if (m < 3 && input->getMouseButtonUp((MouseButtonEnum)m))
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
}

InputPy::InputPy()
{
	PythonManager::typeObjects.push_back(make_pair("Input", &instance.Type));
	PythonManager::constInt.insert(make_pair("M_LBN", VK_LBUTTON));
	PythonManager::constInt.insert(make_pair("M_RBN", VK_RBUTTON));
	PythonManager::constInt.insert(make_pair("M_MBN", VK_MBUTTON));
	PythonManager::constInt.insert(make_pair("M_XBN1", VK_XBUTTON1));
	PythonManager::constInt.insert(make_pair("M_XBN2", VK_XBUTTON2));
	PythonManager::constInt.insert(make_pair("K_CTRL", VK_CONTROL));
	PythonManager::constInt.insert(make_pair("K_SHIFT", VK_SHIFT));
	PythonManager::constInt.insert(make_pair("K_ENTER", VK_RETURN));
	PythonManager::constInt.insert(make_pair("K_DELETE", VK_DELETE));
	PythonManager::constInt.insert(make_pair("K_BACK", VK_BACK));
	PythonManager::constInt.insert(make_pair("K_ESC", VK_ESCAPE));
	PythonManager::constInt.insert(make_pair("K_TAB", VK_TAB));
	PythonManager::constInt.insert(make_pair("K_SPACE", VK_SPACE));
	PythonManager::constInt.insert(make_pair("K_UP", VK_UP));
	PythonManager::constInt.insert(make_pair("K_DOWN", VK_DOWN));
	PythonManager::constInt.insert(make_pair("K_LEFT", VK_LEFT));
	PythonManager::constInt.insert(make_pair("K_RIGHT", VK_RIGHT));
	PythonManager::constInt.insert(make_pair("K_PAGEUP", VK_PRIOR));
	PythonManager::constInt.insert(make_pair("K_PAGEDOWN", VK_NEXT));
}

PyMethodDef TransformPy::Methods[16] = {
	{ "getPosition", TransformPy::getPosition, METH_VARARGS, "getPosition(int = 2(RELATE))" },
	{ "getEulerAngle", TransformPy::getEulerAngle, METH_VARARGS, "getEulerAngle(int = 2(RELATE))" },
	{ "getScale", TransformPy::getScale, METH_VARARGS, "getScale(int = 2(RELATE))" },
	{ "getForward", TransformPy::getForward, METH_VARARGS, "getForward(int = 2(RELATE))" },
	{ "getLeftward", TransformPy::getLeftward, METH_VARARGS, "getLeftward(int = 2(RELATE))" },
	{ "getUpward", TransformPy::getUpward, METH_VARARGS, "getUpward(int = 2(RELATE))" },
	{ "setPosition", TransformPy::setPosition, METH_VARARGS, "setPosition(Vec3, int = 2(RELATE)) or setPosition(float, float, float, int = 2(RELATE))" },
	{ "setRotation", TransformPy::setRotation, METH_VARARGS, "setRotation(Vec3, int = 2(RELATE)) or setRotation(float, float, float, int = 2(RELATE))" },
	{ "setScale", TransformPy::setScale, METH_VARARGS, "setScale(Vec3) or setScale(float, float, float)" },
	{ "translate", TransformPy::translate, METH_VARARGS, "translate(Vec3, int = 2(RELATE)) or translate(float, float, float, int = 2(RELATE))" },
	{ "rotate", TransformPy::rotate, METH_VARARGS, "rotate(Vec3, int = 2(RELATE)) or rotate(float, float, float, int = 2(RELATE))" },
	{ "scale", TransformPy::scale, METH_VARARGS, "scale(Vec3) or scale(float, float, float)" },
	{ "makeSphereConstraint", TransformPy::makeSphereConstraint, METH_VARARGS, "makeSphereConstraint(Transform, [Vec3], [Vec3])" },
	{ "makeFixedConstraint", TransformPy::makeFixedConstraint, METH_VARARGS, "makeFixedConstraint(Transform, [Vec3], [Vec3])" },
	{ "makeD6Constraint", TransformPy::makeD6Constraint, METH_VARARGS, "makeD6Constraint(Transform, [Vec3], [Vec3])" },
	{ NULL }
};

PyTypeObject TransformPy::Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"BraneEngine.Transform",            /* tp_name */
	sizeof(PyCPointer),                    /* tp_basicsize */
	0,                                          /* tp_itemsize */
	0,											/* tp_dealloc */
	0,                               /* tp_print */
	0,                                          /* tp_getattr */
	0,                                          /* tp_setattr */
	0,                                          /* tp_reserved */
	0,                                          /* tp_repr */
	0,                                          /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	(reprfunc)TransformPy::__str__,               /* tp_str */
	0,                                          /* tp_getattro */
	0,                                          /* tp_setattro */
	0,                                          /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | 
	Py_TPFLAGS_BASETYPE,                         /* tp_flags */
	"Transform Class",                              /* tp_doc */
	0,                                          /* tp_traverse */
	0,                                          /* tp_clear */
	0,                                          /* tp_richcompare */
	0,                                          /* tp_weaklistoffset */
	0,                                          /* tp_iter */
	0,                                          /* tp_iternext */
	TransformPy::Methods,                                          /* tp_methods */
	0,										    /* tp_members */
	0,                                          /* tp_getset */
	&ObjectPy::Type,                         /* tp_base */
	0,                                          /* tp_dict */
	0,                                          /* tp_descr_get */
	0,                                          /* tp_descr_set */
	0,                                          /* tp_dictoffset */
	0,										       /* tp_init */
	0,                                          /* tp_alloc */
	0,											  /* tp_new */
};

TransformPy TransformPy::instance;

PyObject * TransformPy::__str__(PyCPointer * obj)
{
	char str[1024];
	sprintf_s(str, "Transform(%s)", ((::Transform*)obj->nativeHandle)->name.c_str());
	return PyUnicode_FromFormat(str);
}

PyObject * TransformPy::getPosition(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	TransformSpace space = TransformSpace::RELATE;
	if (s == 1) {
		int i = 2;
		PyArg_ParseTuple(args, "i", &i);
		if (i >= 0 && i <= 2)
			space = (TransformSpace)i;
	}
	else if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "getPosition(int = 2(RELATE))");
		return NULL;
	}
	::Transform* tran = ObjectPy::cast<::Transform>(self);
	if (tran == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	return Vec3Py::New(tran->getPosition(space));
}

PyObject * TransformPy::getEulerAngle(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	TransformSpace space = TransformSpace::RELATE;
	if (s == 1) {
		int i = 2;
		PyArg_ParseTuple(args, "i", &i);
		if (i >= 0 && i <= 2)
			space = (TransformSpace)i;
	}
	else if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "getEulerAngle(int = 2(RELATE))");
		return NULL;
	}
	::Transform* tran = ObjectPy::cast<::Transform>(self);
	if (tran == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	return Vec3Py::New(tran->getEulerAngle(space));
}

PyObject * TransformPy::getScale(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	TransformSpace space = TransformSpace::RELATE;
	if (s == 1) {
		int i = 2;
		PyArg_ParseTuple(args, "i", &i);
		if (i >= 0 && i <= 2)
			space = (TransformSpace)i;
	}
	else if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "getScale(int = 2(RELATE))");
		return NULL;
	}
	::Transform* tran = ObjectPy::cast<::Transform>(self);
	if (tran == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	return Vec3Py::New(tran->scale);
}

PyObject * TransformPy::getForward(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	TransformSpace space = TransformSpace::RELATE;
	if (s == 1) {
		int i = 2;
		PyArg_ParseTuple(args, "i", &i);
		if (i >= 0 && i <= 2)
			space = (TransformSpace)i;
	}
	else if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "getForward(int = 2(RELATE))");
		return NULL;
	}
	::Transform* tran = ObjectPy::cast<::Transform>(self);
	if (tran == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	return Vec3Py::New(tran->getForward(space));
}

PyObject * TransformPy::getLeftward(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	TransformSpace space = TransformSpace::RELATE;
	if (s == 1) {
		int i = 2;
		PyArg_ParseTuple(args, "i", &i);
		if (i >= 0 && i <= 2)
			space = (TransformSpace)i;
	}
	else if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "getLeftward(int = 2(RELATE))");
		return NULL;
	}
	::Transform* tran = ObjectPy::cast<::Transform>(self);
	if (tran == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	return Vec3Py::New(tran->getLeftward(space));
}

PyObject * TransformPy::getUpward(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	TransformSpace space = TransformSpace::RELATE;
	if (s == 1) {
		int i = 2;
		PyArg_ParseTuple(args, "i", &i);
		if (i >= 0 && i <= 2)
			space = (TransformSpace)i;
	}
	else if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "getUpward(int = 2(RELATE))");
		return NULL;
	}
	::Transform* tran = ObjectPy::cast<::Transform>(self);
	if (tran == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	return Vec3Py::New(tran->getUpward(space));
}

PyObject * TransformPy::setPosition(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	float x = 0, y = 0, z = 0;
	TransformSpace space = TransformSpace::RELATE;
	bool err = false;
	if (s <= 2) {
		PyObject* hobj = NULL;
		int i = 2;
		if (!PyArg_ParseTuple(args, "O|i", &hobj, &i))
			err = true;
		else {
			Vec3Py::Vec3* vec = Vec3Py::cast(hobj);
			if (vec == NULL)
				err = true;
			else {
				if (i >= 0 && i <= 2)
					space = (TransformSpace)i;
				x = vec->x;
				y = vec->y;
				z = vec->z;
			}
		}
	}
	else if (s == 3 || s == 4) {
		int i = 2;
		if (!PyArg_ParseTuple(args, "fff|i", &x, &y, &z, &i))
			err = true;
		else if (i >= 0 && i <= 2)
			space = (TransformSpace)i;
	}
	else
		err = true;
	if (err) {
		PyErr_SetString(PyExc_TypeError, "setPosition(Vec3, int = 2(RELATE)) or setPosition(float, float, float, int = 2(RELATE))");
		return NULL;
	}
	::Transform* tran = ObjectPy::cast<::Transform>(self);
	if (tran == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	tran->setPosition(x, y, z, space);
	Py_RETURN_NONE;
}

PyObject * TransformPy::setRotation(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	float x = 0, y = 0, z = 0;
	TransformSpace space = TransformSpace::RELATE;
	bool err = false;
	if (s <= 2) {
		PyObject* hobj = NULL;
		int i = 2;
		if (!PyArg_ParseTuple(args, "O|i", &hobj, &i))
			err = true;
		else {
			Vec3Py::Vec3* vec = Vec3Py::cast(hobj);
			if (vec == NULL)
				err = true;
			else {
				if (i >= 0 && i <= 2)
					space = (TransformSpace)i;
				x = vec->x;
				y = vec->y;
				z = vec->z;
			}
		}
	}
	else if (s == 3 || s == 4) {
		int i = 2;
		if (!PyArg_ParseTuple(args, "fff|i", &x, &y, &z, &i))
			err = true;
		else if (i >= 0 && i <= 2)
			space = (TransformSpace)i;
	}
	else
		err = true;
	if (err) {
		PyErr_SetString(PyExc_TypeError, "setRotation(Vec3, int = 2(RELATE)) or setRotation(float, float, float, int = 2(RELATE))");
		return NULL;
	}
	::Transform* tran = ObjectPy::cast<::Transform>(self);
	if (tran == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	tran->setRotation(x, y, z, space);
	Py_RETURN_NONE;
}

PyObject * TransformPy::setScale(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	float x = 0, y = 0, z = 0;
	TransformSpace space = TransformSpace::RELATE;
	bool err = false;
	if (s == 1) {
		PyObject* hobj = NULL;
		if (!PyArg_ParseTuple(args, "O", &hobj))
			err = true;
		else {
			Vec3Py::Vec3* vec = Vec3Py::cast(hobj);
			if (vec == NULL)
				err = true;
			else {
				x = vec->x;
				y = vec->y;
				z = vec->z;
			}
		}
	}
	else if (s == 3) {
		err = !PyArg_ParseTuple(args, "fff", &x, &y, &z);
	}
	else
		err = true;
	if (err) {
		PyErr_SetString(PyExc_TypeError, "setScale(Vec3) or setScale(float, float, float)");
		return NULL;
	}
	::Transform* tran = ObjectPy::cast<::Transform>(self);
	if (tran == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	tran->setScale(x, y, z);
	Py_RETURN_NONE;
}

PyObject * TransformPy::translate(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	float x = 0, y = 0, z = 0;
	TransformSpace space = TransformSpace::RELATE;
	bool err = false;
	if (s <= 2) {
		PyObject* hobj = NULL;
		int i = 2;
		if (!PyArg_ParseTuple(args, "O|i", &hobj, &i))
			err = true;
		else {
			Vec3Py::Vec3* vec = Vec3Py::cast(hobj);
			if (vec == NULL)
				err = true;
			else {
				if (i >= 0 && i <= 2)
					space = (TransformSpace)i;
				x = vec->x;
				y = vec->y;
				z = vec->z;
			}
		}
	}
	else if (s == 3 || s == 4) {
		int i = 2;
		if (!PyArg_ParseTuple(args, "fff|i", &x, &y, &z, &i))
			err = true;
		else if (i >= 0 && i <= 2)
			space = (TransformSpace)i;
	}
	else
		err = true;
	if (err) {
		PyErr_SetString(PyExc_TypeError, "translate(Vec3, int = 2(RELATE)) or translate(float, float, float, int = 2(RELATE))");
		return NULL;
	}
	::Transform* tran = ObjectPy::cast<::Transform>(self);
	if (tran == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	tran->translate(x, y, z, space);
	Py_RETURN_NONE;
}

PyObject * TransformPy::rotate(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	float x = 0, y = 0, z = 0;
	TransformSpace space = TransformSpace::RELATE;
	bool err = false;
	if (s <= 2) {
		PyObject* hobj = NULL;
		int i = 2;
		if (!PyArg_ParseTuple(args, "O|i", &hobj, &i))
			err = true;
		else {
			Vec3Py::Vec3* vec = Vec3Py::cast(hobj);
			if (vec == NULL)
				err = true;
			else {
				if (i >= 0 && i <= 2)
					space = (TransformSpace)i;
				x = vec->x;
				y = vec->y;
				z = vec->z;
			}
		}
	}
	else if (s == 3 || s == 4) {
		int i = 2;
		if (!PyArg_ParseTuple(args, "fff|i", &x, &y, &z, &i))
			err = true;
		else if (i >= 0 && i <= 2)
			space = (TransformSpace)i;
	}
	else
		err = true;
	if (err) {
		PyErr_SetString(PyExc_TypeError, "rotate(Vec3, int = 2(RELATE)) or rotate(float, float, float, int = 2(RELATE))");
		return NULL;
	}
	::Transform* tran = ObjectPy::cast<::Transform>(self);
	if (tran == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	tran->rotate(x, y, z, space);
	Py_RETURN_NONE;
}

PyObject * TransformPy::scale(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	float x = 0, y = 0, z = 0;
	TransformSpace space = TransformSpace::RELATE;
	bool err = false;
	if (s == 1) {
		PyObject* hobj = NULL;
		if (!PyArg_ParseTuple(args, "O", &hobj))
			err = true;
		else {
			Vec3Py::Vec3* vec = Vec3Py::cast(hobj);
			if (vec == NULL)
				err = true;
			else {
				x = vec->x;
				y = vec->y;
				z = vec->z;
			}
		}
	}
	else if (s == 3) {
		err = !PyArg_ParseTuple(args, "fff", &x, &y, &z);
	}
	else
		err = true;
	if (err) {
		PyErr_SetString(PyExc_TypeError, "scale(Vec3) or scale(float, float, float)");
		return NULL;
	}
	::Transform* tran = ObjectPy::cast<::Transform>(self);
	if (tran == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	tran->scaling(x, y, z);
	Py_RETURN_NONE;
}

PyObject* TransformPy::makeSphereConstraint(PyObject* self, PyObject* args)
{
	int s = PyTuple_Size(args);
	PyObject* tobj = NULL;
	PyObject* vec3obj1 = NULL;
	PyObject* vec3obj2 = NULL;
	bool err = false;

	if (s == 1) {
		err = !PyArg_ParseTuple(args, "O", &tobj);
	}
	else if (s == 2) {
		err = !PyArg_ParseTuple(args, "OO", &tobj, &vec3obj1);
	}
	else if (s == 3) {
		err = !PyArg_ParseTuple(args, "OOO", &tobj, &vec3obj1, &vec3obj2);
	}
	if (err) {
		PyErr_SetString(PyExc_TypeError, "makeSphereConstraint(Transform) or makeSphereConstraint(Transform, Vec3)");
		return NULL;
	}
	Transform* selftran = ObjectPy::cast<::Transform>(self);
	Transform* tran = ObjectPy::cast<::Transform>(tobj);
	if (tran == NULL || selftran == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	if (selftran->rigidBody == NULL || tran->rigidBody == NULL) {
		PyErr_SetString(PyExc_RuntimeWarning, "make sure both Transform has rigidbody");
		return NULL;
	}
	SphericalConstraint* constraint = new SphericalConstraint(*tran->rigidBody, *selftran->rigidBody);
	if (vec3obj1 != NULL) {
		Vec3Py::Vec3* offvec3 = Vec3Py::cast(vec3obj1);
		if (offvec3 != NULL) {
			constraint->frameTransform1 = toPTransform(Vector3f(offvec3->x, offvec3->y, offvec3->z));
		}
	}
	if (vec3obj2 != NULL) {
		Vec3Py::Vec3* offvec3 = Vec3Py::cast(vec3obj2);
		if (offvec3 != NULL) {
			constraint->frameTransform2 = toPTransform(Vector3f(offvec3->x, offvec3->y, offvec3->z));
		}
	}

	selftran->addConstraint(constraint);

	Py_RETURN_NONE;
}

PyObject* TransformPy::makeFixedConstraint(PyObject* self, PyObject* args)
{
	int s = PyTuple_Size(args);
	PyObject* tobj = NULL;
	PyObject* vec3obj1 = NULL;
	PyObject* vec3obj2 = NULL;
	bool err = false;

	if (s == 1) {
		err = !PyArg_ParseTuple(args, "O", &tobj);
	}
	else if (s == 2) {
		err = !PyArg_ParseTuple(args, "OO", &tobj, &vec3obj1);
	}
	else if (s == 3) {
		err = !PyArg_ParseTuple(args, "OOO", &tobj, &vec3obj1, &vec3obj2);
	}
	if (err) {
		PyErr_SetString(PyExc_TypeError, "makeFixedConstraint(Transform, [Vec3], [Vec3])");
		return NULL;
	}
	Transform* selftran = ObjectPy::cast<::Transform>(self);
	Transform* tran = ObjectPy::cast<::Transform>(tobj);
	if (tran == NULL || selftran == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	if (selftran->rigidBody == NULL || tran->rigidBody == NULL) {
		PyErr_SetString(PyExc_RuntimeWarning, "make sure both Transform has rigidbody");
		return NULL;
	}
	FixedConstraint* constraint = new FixedConstraint(*tran->rigidBody, *selftran->rigidBody);
	if (vec3obj1 != NULL) {
		Vec3Py::Vec3* offvec3 = Vec3Py::cast(vec3obj1);
		if (offvec3 != NULL) {
			constraint->frameTransform1 = toPTransform(Vector3f(offvec3->x, offvec3->y, offvec3->z));
		}
	}
	if (vec3obj2 != NULL) {
		Vec3Py::Vec3* offvec3 = Vec3Py::cast(vec3obj2);
		if (offvec3 != NULL) {
			constraint->frameTransform2 = toPTransform(Vector3f(offvec3->x, offvec3->y, offvec3->z));
		}
	}

	selftran->addConstraint(constraint);

	Py_RETURN_NONE;
}

PyObject* TransformPy::makeD6Constraint(PyObject* self, PyObject* args)
{
	int s = PyTuple_Size(args);
	PyObject* tobj = NULL;
	PyObject* vec3obj1 = NULL;
	PyObject* vec3obj2 = NULL;
	bool err = false;

	if (s == 1) {
		err = !PyArg_ParseTuple(args, "O", &tobj);
	}
	else if (s == 2) {
		err = !PyArg_ParseTuple(args, "OO", &tobj, &vec3obj1);
	}
	else if (s == 3) {
		err = !PyArg_ParseTuple(args, "OOO", &tobj, &vec3obj1, &vec3obj2);
	}
	if (err) {
		PyErr_SetString(PyExc_TypeError, "makeD6Constraint(Transform, [Vec3], [Vec3])");
		return NULL;
	}
	Transform* selftran = ObjectPy::cast<::Transform>(self);
	Transform* tran = ObjectPy::cast<::Transform>(tobj);
	if (tran == NULL || selftran == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	if (selftran->rigidBody == NULL || tran->rigidBody == NULL) {
		PyErr_SetString(PyExc_RuntimeWarning, "make sure both Transform has rigidbody");
		return NULL;
	}
	D6Constraint* constraint = new D6Constraint(*tran->rigidBody, *selftran->rigidBody);
	if (vec3obj1 != NULL) {
		Vec3Py::Vec3* offvec3 = Vec3Py::cast(vec3obj1);
		if (offvec3 != NULL) {
			constraint->frameTransform1 = toPTransform(Vector3f(offvec3->x, offvec3->y, offvec3->z));
		}
	}
	if (vec3obj2 != NULL) {
		Vec3Py::Vec3* offvec3 = Vec3Py::cast(vec3obj2);
		if (offvec3 != NULL) {
			constraint->frameTransform2 = toPTransform(Vector3f(offvec3->x, offvec3->y, offvec3->z));
		}
	}

	selftran->addConstraint(constraint);

	Py_RETURN_NONE;
}

TransformPy::TransformPy()
{
	PythonManager::typeObjects.push_back(make_pair("Transform", &instance.Type));
}

PyMethodDef CameraPy::Methods[8] = {
	{ "getDistance", CameraPy::getDistance, METH_VARARGS, "getDistance() return camera distance from origin" },
	{ "getFov", CameraPy::getFov, METH_VARARGS, "getFov()" },
	{ "getMode", CameraPy::getMode, METH_VARARGS, "getMode() 0: perspective 1: orthotropic" },
	{ "setDistance", CameraPy::setDistance, METH_VARARGS, "setDistance(float)set camera distance from origin" },
	{ "setFov", CameraPy::setFov, METH_VARARGS, "setFov(float)" },
	{ "setMode", CameraPy::setMode, METH_VARARGS, "setMode(int) 0: perspective 1: orthotropic" },
	{ "isActive", CameraPy::isActive, METH_VARARGS, "isActive() return if this camera is current main camera in the world" },
	{ NULL }
};

PyTypeObject CameraPy::Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"BraneEngine.Camera",            /* tp_name */
	sizeof(PyCPointer),                    /* tp_basicsize */
	0,                                          /* tp_itemsize */
	0,											/* tp_dealloc */
	0,                               /* tp_print */
	0,                                          /* tp_getattr */
	0,                                          /* tp_setattr */
	0,                                          /* tp_reserved */
	0,                                          /* tp_repr */
	0,                                          /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	(reprfunc)CameraPy::__str__,               /* tp_str */
	0,                                          /* tp_getattro */
	0,                                          /* tp_setattro */
	0,                                          /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE,                         /* tp_flags */
	"Camera Class",                              /* tp_doc */
	0,                                          /* tp_traverse */
	0,                                          /* tp_clear */
	0,                                          /* tp_richcompare */
	0,                                          /* tp_weaklistoffset */
	0,                                          /* tp_iter */
	0,                                          /* tp_iternext */
	CameraPy::Methods,                                          /* tp_methods */
	0,										    /* tp_members */
	0,                                          /* tp_getset */
	&TransformPy::Type,                         /* tp_base */
	0,                                          /* tp_dict */
	0,                                          /* tp_descr_get */
	0,                                          /* tp_descr_set */
	0,                                          /* tp_dictoffset */
	0,										       /* tp_init */
	0,                                          /* tp_alloc */
	0,											  /* tp_new */
};

CameraPy CameraPy::instance;

PyObject * CameraPy::__str__(PyCPointer * obj)
{
	return PyUnicode_FromFormat("Camera(%s)", ((Camera*)obj->nativeHandle)->name.c_str());
}

PyObject * CameraPy::getDistance(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "getDistance()");
		return NULL;
	}
	Camera* ptr = ObjectPy::cast<Camera>(self);
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	return PyFloat_FromDouble(ptr->distance);
}

PyObject * CameraPy::getFov(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "getFov()");
		return NULL;
	}
	Camera* ptr = ObjectPy::cast<Camera>(self);
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	return PyFloat_FromDouble(ptr->fov);
}

PyObject * CameraPy::getMode(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "getMode()");
		return NULL;
	}
	Camera* ptr = ObjectPy::cast<Camera>(self);
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	return PyLong_FromLong(ptr->mode);
}

PyObject * CameraPy::setDistance(PyObject * self, PyObject * args)
{
	float dis = 0;
	if (!PyArg_ParseTuple(args, "f", &dis)) {
		PyErr_SetString(PyExc_TypeError, "setDistance(float)");
		return NULL;
	}
	Camera* ptr = ObjectPy::cast<Camera>(self);
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	ptr->distance = dis;
	Py_RETURN_NONE;
}

PyObject * CameraPy::setFov(PyObject * self, PyObject * args)
{
	float fov = 0;
	if (!PyArg_ParseTuple(args, "f", &fov)) {
		PyErr_SetString(PyExc_TypeError, "setFov(float)");
		return NULL;
	}
	Camera* ptr = ObjectPy::cast<Camera>(self);
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	ptr->fov = fov;
	Py_RETURN_NONE;
}

PyObject * CameraPy::setMode(PyObject * self, PyObject * args)
{
	int mode = 0;
	if (!PyArg_ParseTuple(args, "i", &mode)) {
		PyErr_SetString(PyExc_TypeError, "setMode(int)");
		return NULL;
	}
	if (mode != 0 && mode != 1) {
		PyErr_SetString(PyExc_TypeError, "mode 0: perspective; mode 1: orthotropic");
		return NULL;
	}
	Camera* ptr = ObjectPy::cast<Camera>(self);
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	ptr->mode = (Camera::CameraMode)mode;
	Py_RETURN_NONE;
}

PyObject * CameraPy::isActive(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "isActive()");
		return NULL;
	}
	Camera* ptr = ObjectPy::cast<Camera>(self);
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	if (ptr->isActive())
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
}

CameraPy::CameraPy()
{
	PythonManager::typeObjects.push_back(make_pair("Camera", &instance.Type));
}

PyMethodDef WorldPy::Methods[9] = {
	{ "getCurrentCamera", WorldPy::getCurrentCamera, METH_VARARGS, "getCurrentCamera() return current camera" },
	{ "getDefaultCamera", WorldPy::getDefaultCamera, METH_VARARGS, "getDefaultCamera() return default camera" },
	{ "getInput", WorldPy::getInput, METH_VARARGS, "getInput() return input object" },
	{ "switchCamera", WorldPy::switchCamera, METH_VARARGS, "switchCamera(Camera)" },
	{ "switchToDefaultCamera", WorldPy::switchToDefaultCamera, METH_VARARGS, "switchToDefaultCamera()" },
	{ "setMainVolume", WorldPy::setMainVolume, METH_VARARGS, "setMainVolume(float(0.0 - 1.0))" },
	{ "quit", WorldPy::quit, METH_VARARGS, "quit() quit the engine next frame" },
	{ "willQuit", WorldPy::willQuit, METH_VARARGS, "willQuit() return if the engine will quit" },
	{ NULL }
};

PyTypeObject WorldPy::Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"BraneEngine.World",            /* tp_name */
	sizeof(PyCPointer),                    /* tp_basicsize */
	0,                                          /* tp_itemsize */
	0,											/* tp_dealloc */
	0,                               /* tp_print */
	0,                                          /* tp_getattr */
	0,                                          /* tp_setattr */
	0,                                          /* tp_reserved */
	0,                                          /* tp_repr */
	0,                                          /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	(reprfunc)WorldPy::__str__,               /* tp_str */
	0,                                          /* tp_getattro */
	0,                                          /* tp_setattro */
	0,                                          /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE,                         /* tp_flags */
	"World Class",                              /* tp_doc */
	0,                                          /* tp_traverse */
	0,                                          /* tp_clear */
	0,                                          /* tp_richcompare */
	0,                                          /* tp_weaklistoffset */
	0,                                          /* tp_iter */
	0,                                          /* tp_iternext */
	WorldPy::Methods,                                          /* tp_methods */
	0,										    /* tp_members */
	0,                                          /* tp_getset */
	&TransformPy::Type,                         /* tp_base */
	0,                                          /* tp_dict */
	0,                                          /* tp_descr_get */
	0,                                          /* tp_descr_set */
	0,                                          /* tp_dictoffset */
	0,										       /* tp_init */
	0,                                          /* tp_alloc */
	0,											  /* tp_new */
};

WorldPy WorldPy::instance;

PyObject * WorldPy::__str__(PyCPointer * obj)
{
	return PyUnicode_FromFormat("World(%s)", ((World*)obj->nativeHandle)->name.c_str());
}

PyObject * WorldPy::getCurrentCamera(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "getCurrentCamera()");
		return NULL;
	}
	World* ptr = ObjectPy::cast<World>(self);
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	PyObject* obj = PyObject_CallFunction((PyObject*)&CameraPy::Type, "L", &ptr->getCurrentCamera());
	if (obj == NULL)
		PyErr_SetString(PyExc_RuntimeError, "construct Camera failed");
	return obj;
}

PyObject * WorldPy::getDefaultCamera(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "getDefaultCamera()");
		return NULL;
	}
	World* ptr = ObjectPy::cast<World>(self);
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	PyObject* obj = PyObject_CallFunction((PyObject*)&CameraPy::Type, "L", &ptr->getDefaultCamera());
	if (obj == NULL)
		PyErr_SetString(PyExc_RuntimeError, "construct Camera failed");
	return obj;
}

PyObject * WorldPy::getInput(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "getDefaultCamera()");
		return NULL;
	}
	World* ptr = ObjectPy::cast<World>(self);
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	PyObject* obj = PyObject_CallFunction((PyObject*)&InputPy::Type, "L", &Engine::getInput());
	if (obj == NULL)
		PyErr_SetString(PyExc_RuntimeError, "construct Input failed");
	return obj;
}

PyObject * WorldPy::switchCamera(PyObject * self, PyObject * args)
{
	PyObject* pycam = 0;
	if (!PyArg_ParseTuple(args, "O", &pycam)) {
		PyErr_SetString(PyExc_TypeError, "switchCamera(Camera)");
		return NULL;
	}
	if (!PyType_Check(pycam, &CameraPy::Type)) {
		PyErr_SetString(PyExc_TypeError, "switchCamera(Camera)");
		return NULL;
	}
	World* ptr = ObjectPy::cast<World>(self);
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	Camera* cam = ObjectPy::cast<Camera>(pycam);
	if (cam == NULL) {
		PyErr_SetString(PyExc_MemoryError, "cast c pointer to Camera failed");
		return NULL;
	}
	ptr->switchCamera(*cam);
	Py_RETURN_NONE;
}

PyObject * WorldPy::switchToDefaultCamera(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "switchToDefaultCamera()");
		return NULL;
	}
	World* ptr = ObjectPy::cast<World>(self);
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	ptr->switchToDefaultCamera();
	Py_RETURN_NONE;
}

PyObject * WorldPy::setMainVolume(PyObject * self, PyObject * args)
{
	float vol = 0;
	if (!PyArg_ParseTuple(args, "f", &vol)) {
		PyErr_SetString(PyExc_TypeError, "setMainVolume(float)");
		return NULL;
	}
	World* ptr = ObjectPy::cast<World>(self);
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	ptr->setMainVolume(vol);
	Py_RETURN_NONE;
}

PyObject * WorldPy::quit(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "quit()");
		return NULL;
	}
	World* ptr = ObjectPy::cast<World>(self);
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	ptr->quit();
	Py_RETURN_NONE;
}

PyObject * WorldPy::willQuit(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "willQuit()");
		return NULL;
	}
	World* ptr = ObjectPy::cast<World>(self);
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	if (ptr->willQuit())
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
}

WorldPy::WorldPy()
{
	PythonManager::typeObjects.push_back(make_pair("World", &instance.Type));
}

PyMethodDef ActorPy::Methods[6] = {
	{ "isHidden", ActorPy::isHidden, METH_VARARGS, "check actor is hidden" },
	{ "setHidden", ActorPy::setHidden, METH_VARARGS, "set the value of actor's hidden" },
	{ "playAudio", ActorPy::playAudio, METH_VARARGS, "play audio of this actor" },
	{ "pauseAudio", ActorPy::pauseAudio, METH_VARARGS, "pause audio of this actor" },
	{ "stopAudio", ActorPy::stopAudio, METH_VARARGS, "stop audio of this actor" },
	{ NULL }
};

PyTypeObject ActorPy::Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"BraneEngine.Actor",            /* tp_name */
	sizeof(PyCPointer),                    /* tp_basicsize */
	0,                                          /* tp_itemsize */
	0,											/* tp_dealloc */
	0,                               /* tp_print */
	0,                                          /* tp_getattr */
	0,                                          /* tp_setattr */
	0,                                          /* tp_reserved */
	0,                                          /* tp_repr */
	0,                                          /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	(reprfunc)ActorPy::__str__,               /* tp_str */
	0,                                          /* tp_getattro */
	0,                                          /* tp_setattro */
	0,                                          /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | 
	Py_TPFLAGS_BASETYPE,                         /* tp_flags */
	"Actor Class",                              /* tp_doc */
	0,                                          /* tp_traverse */
	0,                                          /* tp_clear */
	0,                                          /* tp_richcompare */
	0,                                          /* tp_weaklistoffset */
	0,                                          /* tp_iter */
	0,                                          /* tp_iternext */
	ActorPy::Methods,                                          /* tp_methods */
	0,										    /* tp_members */
	0,                                          /* tp_getset */
	&TransformPy::Type,                         /* tp_base */
	0,                                          /* tp_dict */
	0,                                          /* tp_descr_get */
	0,                                          /* tp_descr_set */
	0,                                          /* tp_dictoffset */
	0,										       /* tp_init */
	0,                                          /* tp_alloc */
	0,											  /* tp_new */
};

ActorPy ActorPy::instance;

PyObject * ActorPy::__str__(PyCPointer * obj)
{
	char str[1024];
	sprintf_s(str, "Actor(%s)", ((Actor*)obj->nativeHandle)->name.c_str());
	return PyUnicode_FromFormat(str);
}

PyObject * ActorPy::isHidden(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "isHidden()");
		return NULL;
	}
	Actor* ptr = ObjectPy::cast<Actor>(self);
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	return ptr->isHidden() ? Py_True : Py_False;
}

PyObject * ActorPy::setHidden(PyObject * self, PyObject * args)
{
	int v = 1;
	if (!PyArg_ParseTuple(args, "|p", &v)) {
		PyErr_SetString(PyExc_TypeError, "setHidden(bool = true)");
		return NULL;
	}
	Actor* ptr = ObjectPy::cast<Actor>(self);
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	ptr->setHidden(v);
	Py_RETURN_NONE;
}

PyObject * ActorPy::playAudio(PyObject * self, PyObject * args)
{
	int index = 0;
	char* name = NULL;
	if (PyArg_ParseTuple(args, "|i", &index)) {
	}
	else if (PyArg_ParseTuple(args, "s", &name)) {
	}
	else {
		PyErr_SetString(PyExc_TypeError, "playAudio(int = 0) or playAudio(String)");
		return NULL;
	}
	Actor* ptr = ObjectPy::cast<Actor>(self);
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	if (name == NULL) {
		if (index < ptr->audioSources.size()) {
			ptr->audioSources[index]->play();
			Py_RETURN_TRUE;
		}
	}
	else {
		for (int i = 0; i < ptr->audioSources.size(); i++) {
			if (ptr->audioSources[i]->audioData->name == name) {
				ptr->audioSources[i]->play();
				Py_RETURN_TRUE;
			}
		}
	}
	Py_RETURN_FALSE;
}

PyObject * ActorPy::pauseAudio(PyObject * self, PyObject * args)
{
	int index = 0;
	char* name = NULL;
	if (PyArg_ParseTuple(args, "|i", &index)) {
	}
	else if (PyArg_ParseTuple(args, "s", &name)) {
	}
	else {
		PyErr_SetString(PyExc_TypeError, "pauseAudio(int = 0) or pauseAudio(String)");
		return NULL;
	}
	Actor* ptr = ObjectPy::cast<Actor>(self);
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	if (name == NULL) {
		if (index < ptr->audioSources.size()) {
			ptr->audioSources[index]->pause();
			Py_RETURN_TRUE;
		}
	}
	else {
		for (int i = 0; i < ptr->audioSources.size(); i++) {
			if (ptr->audioSources[i]->audioData->name == name) {
				ptr->audioSources[i]->pause();
				Py_RETURN_TRUE;
			}
		}
	}
	Py_RETURN_FALSE;
}

PyObject * ActorPy::stopAudio(PyObject * self, PyObject * args)
{
	int index = 0;
	char* name = NULL;
	if (PyArg_ParseTuple(args, "|i", &index)) {
	}
	else if (PyArg_ParseTuple(args, "s", &name)) {
	}
	else {
		PyErr_SetString(PyExc_TypeError, "stopAudio(int = 0) or stopAudio(String)");
		return NULL;
	}
	Actor* ptr = ObjectPy::cast<Actor>(self);
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	if (name == NULL) {
		if (index < ptr->audioSources.size()) {
			ptr->audioSources[index]->stop();
			Py_RETURN_TRUE;
		}
	}
	else {
		for (int i = 0; i < ptr->audioSources.size(); i++) {
			if (ptr->audioSources[i]->audioData->name == name) {
				ptr->audioSources[i]->stop();
				Py_RETURN_TRUE;
			}
		}
	}
	Py_RETURN_FALSE;
}

ActorPy::ActorPy()
{
	PythonManager::typeObjects.push_back(make_pair("Actor", &instance.Type));
}

PyMethodDef MeshActorPy::Methods[1] = {
	{ NULL }
};

PyTypeObject MeshActorPy::Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"BraneEngine.MeshActor",            /* tp_name */
	sizeof(PyCPointer),                    /* tp_basicsize */
	0,                                          /* tp_itemsize */
	0,											/* tp_dealloc */
	0,                               /* tp_print */
	0,                                          /* tp_getattr */
	0,                                          /* tp_setattr */
	0,                                          /* tp_reserved */
	0,                                          /* tp_repr */
	0,                                          /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	(reprfunc)MeshActorPy::__str__,               /* tp_str */
	0,                                          /* tp_getattro */
	0,                                          /* tp_setattro */
	0,                                          /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE,                         /* tp_flags */
	"MeshActor Class",                              /* tp_doc */
	0,                                          /* tp_traverse */
	0,                                          /* tp_clear */
	0,                                          /* tp_richcompare */
	0,                                          /* tp_weaklistoffset */
	0,                                          /* tp_iter */
	0,                                          /* tp_iternext */
	MeshActorPy::Methods,                                          /* tp_methods */
	0,										    /* tp_members */
	0,                                          /* tp_getset */
	&ActorPy::Type,                         /* tp_base */
	0,                                          /* tp_dict */
	0,                                          /* tp_descr_get */
	0,                                          /* tp_descr_set */
	0,                                          /* tp_dictoffset */
	0,										       /* tp_init */
	0,                                          /* tp_alloc */
	0,											  /* tp_new */
};

MeshActorPy MeshActorPy::instance;

PyObject * MeshActorPy::__str__(PyCPointer * obj)
{
	return PyUnicode_FromFormat("MeshActor(%s)", ((MeshActor*)obj->nativeHandle)->name.c_str());
}

MeshActorPy::MeshActorPy()
{
	PythonManager::typeObjects.push_back(make_pair("MeshActor", &instance.Type));
}

PyMethodDef SkeletonMeshActorPy::Methods[5] = {
	{ "playAnimation", SkeletonMeshActorPy::playAnimation, METH_VARARGS, "play animation of this actor" },
	{ "pauseAnimation", SkeletonMeshActorPy::pauseAnimation, METH_VARARGS, "pause animation of this actor" },
	{ "stopAnimation", SkeletonMeshActorPy::stopAnimation, METH_VARARGS, "stop animation of this actor" },
	{ "setBlendSpaceWeight", SkeletonMeshActorPy::setBlendSpaceWeight, METH_VARARGS, "set weight of blendspace" },
	{ NULL }
};

PyTypeObject SkeletonMeshActorPy::Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"BraneEngine.SkeletonMeshActor",            /* tp_name */
	sizeof(PyCPointer),                    /* tp_basicsize */
	0,                                          /* tp_itemsize */
	0,											/* tp_dealloc */
	0,                               /* tp_print */
	0,                                          /* tp_getattr */
	0,                                          /* tp_setattr */
	0,                                          /* tp_reserved */
	0,                                          /* tp_repr */
	0,                                          /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	(reprfunc)SkeletonMeshActorPy::__str__,               /* tp_str */
	0,                                          /* tp_getattro */
	0,                                          /* tp_setattro */
	0,                                          /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | 
	Py_TPFLAGS_BASETYPE,                         /* tp_flags */
	"SkeletonMeshActor Class",                              /* tp_doc */
	0,                                          /* tp_traverse */
	0,                                          /* tp_clear */
	0,                                          /* tp_richcompare */
	0,                                          /* tp_weaklistoffset */
	0,                                          /* tp_iter */
	0,                                          /* tp_iternext */
	SkeletonMeshActorPy::Methods,                                          /* tp_methods */
	0,										    /* tp_members */
	0,                                          /* tp_getset */
	&ActorPy::Type,                         /* tp_base */
	0,                                          /* tp_dict */
	0,                                          /* tp_descr_get */
	0,                                          /* tp_descr_set */
	0,                                          /* tp_dictoffset */
	0,										       /* tp_init */
	0,                                          /* tp_alloc */
	0,											  /* tp_new */
};

SkeletonMeshActorPy SkeletonMeshActorPy::instance;

PyObject * SkeletonMeshActorPy::__str__(PyCPointer * obj)
{
	char str[1024];
	sprintf_s(str, "SkeletonMeshActor(%s)", ((SkeletonMeshActor*)obj->nativeHandle)->name.c_str());
	return PyUnicode_FromFormat(str);
}

PyObject * SkeletonMeshActorPy::playAnimation(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	int index = 0;
	char* name = NULL;
	SkeletonMeshActor* ptr = ObjectPy::cast<SkeletonMeshActor>(self);
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	if (s == 0 && ptr->animationClip != NULL) {
		ptr->animationClip->play();
		Py_RETURN_TRUE;
	}
	else if (PyArg_ParseTuple(args, "i", &index)) {
	}
	else if (PyArg_ParseTuple(args, "s", &name)) {
	}
	else {
		PyErr_SetString(PyExc_TypeError, "playAnimation(int) or playAnimation(String)");
		return NULL;
	}
	if (name == NULL) {
		if (ptr->activeAnimationClip(index)) {
			ptr->animationClip->play();
			Py_RETURN_TRUE;
		}
	}
	else {
		for (int i = 0; i < ptr->audioSources.size(); i++) {
			if (ptr->activeAnimationClip(name)) {
				ptr->animationClip->play();
				Py_RETURN_TRUE;
			}
		}
	}
	Py_RETURN_FALSE;
}

PyObject * SkeletonMeshActorPy::pauseAnimation(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "pauseAnimation()");
		return NULL;
	}
	SkeletonMeshActor* ptr = ObjectPy::cast<SkeletonMeshActor>(self);
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	if (ptr->animationClip != NULL)
		ptr->animationClip->pause();
	Py_RETURN_NONE;
}

PyObject * SkeletonMeshActorPy::stopAnimation(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	if (s != 0) {
		PyErr_SetString(PyExc_TypeError, "stopAnimation()");
		return NULL;
	}
	SkeletonMeshActor* ptr = ObjectPy::cast<SkeletonMeshActor>(self);
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	if (ptr->animationClip != NULL)
		ptr->animationClip->stop();
	Py_RETURN_NONE;
}

PyObject * SkeletonMeshActorPy::setBlendSpaceWeight(PyObject * self, PyObject * args)
{
	int s = PyTuple_Size(args);
	int index = 0;
	char* name = NULL;
	PyObject* hobj = NULL;
	BlendSpaceAnimation* bsa = NULL;
	Vec3Py::Vec3* vec = NULL;
	SkeletonMeshActor* ptr = ObjectPy::cast<SkeletonMeshActor>(self);
	if (ptr == NULL) {
		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
		return NULL;
	}
	if (ptr->animationClip != NULL && PyArg_ParseTuple(args, "O", &hobj)) {
		bsa = dynamic_cast<BlendSpaceAnimation*>(ptr->animationClip);
	}
	else if (PyArg_ParseTuple(args, "iO", &index, &hobj)) {
	}
	else if (PyArg_ParseTuple(args, "sO", &name, &hobj)) {
	}
	else {
		PyErr_SetString(PyExc_TypeError, "setBlendSpaceWeight(Vec3), setBlendSpaceWeight(int, Vec3) or setBlendSpaceWeight(String, Vec3)");
		return NULL;
	}
	if (bsa == NULL) {
		if (name == NULL) {
			bsa = dynamic_cast<BlendSpaceAnimation*>(ptr->animationClips[index]);
		}
		else {
			auto iter = ptr->animationClipList.find(name);
			if (iter != ptr->animationClipList.end()) {
				bsa = dynamic_cast<BlendSpaceAnimation*>(ptr->animationClips[iter->second]);
			}
		}
	}
	vec = Vec3Py::cast(hobj);
	if (bsa != NULL && vec != NULL) {
		bsa->setBlendWeight({ vec->x, vec->y });
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

SkeletonMeshActorPy::SkeletonMeshActorPy()
{
	PythonManager::typeObjects.push_back(make_pair("SkeletonMeshActor", &instance.Type));
}

//PyMethodDef CharacterPy::Methods[4] = {
//	{ "isFly", CharacterPy::isFly, METH_VARARGS, "test charater if in the air" },
//	{ "move", CharacterPy::move, METH_VARARGS, "move charater by vec3" },
//	{ "jump", CharacterPy::jump, METH_VARARGS, "toggle jump" },
//	{ NULL }
//};
//
//PyTypeObject CharacterPy::Type = {
//	PyVarObject_HEAD_INIT(&PyType_Type, 0)
//	"BraneEngine.Character",            /* tp_name */
//	sizeof(PyCPointer),                    /* tp_basicsize */
//	0,                                          /* tp_itemsize */
//	0,											/* tp_dealloc */
//	0,                               /* tp_print */
//	0,                                          /* tp_getattr */
//	0,                                          /* tp_setattr */
//	0,                                          /* tp_reserved */
//	0,                                          /* tp_repr */
//	0,                                          /* tp_as_number */
//	0,                                          /* tp_as_sequence */
//	0,                                          /* tp_as_mapping */
//	0,                                          /* tp_hash */
//	0,                                          /* tp_call */
//	(reprfunc)CharacterPy::__str__,               /* tp_str */
//	0,                                          /* tp_getattro */
//	0,                                          /* tp_setattro */
//	0,                                          /* tp_as_buffer */
//	Py_TPFLAGS_DEFAULT |
//	Py_TPFLAGS_BASETYPE,                         /* tp_flags */
//	"Character Class",                              /* tp_doc */
//	0,                                          /* tp_traverse */
//	0,                                          /* tp_clear */
//	0,                                          /* tp_richcompare */
//	0,                                          /* tp_weaklistoffset */
//	0,                                          /* tp_iter */
//	0,                                          /* tp_iternext */
//	CharacterPy::Methods,                                          /* tp_methods */
//	0,										    /* tp_members */
//	0,                                          /* tp_getset */
//	&SkeletonMeshActorPy::Type,                         /* tp_base */
//	0,                                          /* tp_dict */
//	0,                                          /* tp_descr_get */
//	0,                                          /* tp_descr_set */
//	0,                                          /* tp_dictoffset */
//	0,										       /* tp_init */
//	0,                                          /* tp_alloc */
//	0,											  /* tp_new */
//};
//
//CharacterPy CharacterPy::instance;
//
//PyObject * CharacterPy::__str__(PyCPointer * obj)
//{
//	char str[1024];
//	sprintf_s(str, "SkeletonMeshActor(%s)", ((Character*)obj->nativeHandle)->name.c_str());
//	return PyUnicode_FromFormat(str);
//}
//
//PyObject * CharacterPy::isFly(PyObject * self, PyObject * args)
//{
//	int s = PyTuple_Size(args);
//	if (s != 0) {
//		PyErr_SetString(PyExc_TypeError, "isFly()");
//		return NULL;
//	}
//	Character* ptr = ObjectPy::cast<Character>(self);
//	if (ptr == NULL) {
//		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
//		return NULL;
//	}
//	if (ptr->isFly())
//		Py_RETURN_TRUE;
//	else
//		Py_RETURN_FALSE;
//}
//
//PyObject * CharacterPy::move(PyObject * self, PyObject * args)
//{
//	int s = PyTuple_Size(args);
//	float x = 0, y = 0, z = 0;
//	bool err = false;
//	if (s == 1) {
//		PyObject* hobj = NULL;
//		if (!PyArg_Parse(args, "O", &hobj))
//			err = true;
//		else {
//			Vec3Py::Vec3* vec = Vec3Py::cast(hobj);
//			if (vec == NULL)
//				err = true;
//			else {
//				x = vec->x;
//				y = vec->y;
//				z = vec->z;
//			}
//		}
//	}
//	else if (s == 3) {
//		err = !PyArg_ParseTuple(args, "fff", &x, &y, &z);
//	}
//	else
//		err = true;
//	if (err) {
//		PyErr_SetString(PyExc_TypeError, "move(Vec3) or move(float, float, float)");
//		return NULL;
//	}
//	Character* ptr = ObjectPy::cast<Character>(self);
//	if (ptr == NULL) {
//		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
//		return NULL;
//	}
//	ptr->move({ x, y, z });
//	Py_RETURN_NONE;
//}
//
//PyObject * CharacterPy::jump(PyObject * self, PyObject * args)
//{
//	int s = PyTuple_Size(args);
//	float impulse = 0;
//	bool err = false;
//	if (s == 1) {
//		err = !PyArg_ParseTuple(args, "f", &impulse);
//	}
//	else
//		err = true;
//	if (err) {
//		PyErr_SetString(PyExc_TypeError, "move(Vec3) or move(float, float, float)");
//		return NULL;
//	}
//	Character* ptr = ObjectPy::cast<Character>(self);
//	if (ptr == NULL) {
//		PyErr_SetString(PyExc_MemoryError, "access c pointer failed");
//		return NULL;
//	}
//	ptr->jump(impulse);
//	Py_RETURN_NONE;
//}
//
//CharacterPy::CharacterPy()
//{
//	PythonManager::typeObjects.push_back(make_pair("Character", &instance.Type));
//}
