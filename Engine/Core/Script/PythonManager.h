#pragma once
#ifndef _PYTHONMANAGER_H_
#define _PYTHONMANAGER_H_

#include "../Unit.h"
#include <Python.h>
#include <structmember.h>

class ENGINE_API PythonManager
{
private:
	static bool isInit;
	static PyObject* pyMainMod;
public:
	static vector<PyMethodDef> methods;
	static vector<pair<const char*, PyTypeObject*>> typeObjects;
	static map<PyTypeObject*, set<PyObject*>> runtimeObjects;
	static map<const char*, long> constInt;
	static PyModuleDef engineMod;
	static void start();
	static void run(const string& code);
	static void end();

	static PyObject* getMainModule();
	static void regist(PyObject* obj);
	static void logoff(PyObject* obj);
};

class UtilityPy
{
public:
	static PyObject* getWorld(PyObject *self, PyObject *args);
	static PyObject* getInput(PyObject *self, PyObject *args);
	static PyObject* destroyObject(PyObject *self, PyObject *args);
	static PyObject* getAllAssetName(PyObject *self, PyObject *args);
	static PyObject* spawnMeshActor(PyObject* self, PyObject* args);
	static PyObject* spawnPrefab(PyObject *self, PyObject *args);
	static PyObject* setGravity(PyObject *self, PyObject *args);
	static PyObject* msgBox(PyObject *self, PyObject *args);

	static UtilityPy instance;
	UtilityPy();
};

class ConsolePy
{
public:
	static PyModuleDef logMod;
	static PyMethodDef logMethods[3];
	static PyModuleDef errorMod;
	static PyMethodDef errorMethods[3];
	static ConsolePy instance;
	static PyObject* log(PyObject *self, PyObject *args);
	static PyObject* warn(PyObject *self, PyObject *args);
	static PyObject* error(PyObject *self, PyObject *args);
	static PyObject* pyLogWrite(PyObject *self, PyObject *args);
	static PyObject* pyLogFlush(PyObject *self, PyObject *args);
	static PyObject* pyErrWrite(PyObject *self, PyObject *args);
	static PyObject* pyErrFlush(PyObject *self, PyObject *args);

	ConsolePy();
};

class BehaviorPy
{
public:
	struct Behavior
	{
		PyObject_HEAD;
	};

	static PyTypeObject Type;
	static BehaviorPy instance;

	static void __dealloc__(Behavior* self);
	static PyObject* __new__(PyTypeObject* type, PyObject* args, PyObject* kwds);
	static int __init__(Behavior* self, PyObject* args, PyObject* kwds);
	static PyObject* __str__(Behavior* behavior);

	BehaviorPy();
};

class Vec3Py
{
public:
	struct Vec3
	{
		PyObject_HEAD;
		float x, y, z;
	};
	static PyNumberMethods NumMethods;
	static PyMemberDef Members[4];
	static PyMethodDef Methods[6];
	static PyTypeObject Type;
	static Vec3Py instance;

	static void Vec3_dealloc(Vec3* self);
	static PyObject* Vec3_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
	static int Vec3_init(Vec3 *self, PyObject *args, PyObject *kwds);
	static PyObject* __str__(Vec3 *obj);
	static Vec3* __add__(Vec3* a, Vec3* b);
	static Vec3* __sub__(Vec3* a, Vec3* b);

	static PyObject* dot(PyObject * self, PyObject * args);
	static PyObject* cross(PyObject * self, PyObject * args);
	static PyObject* norm(PyObject * self, PyObject * args);
	static PyObject* normalize(PyObject * self, PyObject * args);
	static PyObject* normalized(PyObject * self, PyObject * args);

	Vec3Py();

	static PyObject* New(float x = 0, float y = 0, float z = 0);
	static PyObject* New(const Vector3f& v);

	static Vec3* cast(PyObject* pobj);
};

class QuatPy
{
public:
	struct Quat
	{
		PyObject_HEAD;
		float x, y, z, w;
	};
	static PyNumberMethods NumMethods;
	static PyMemberDef Members[5];
	static PyMethodDef Methods[4];
	static PyTypeObject Type;
	static QuatPy instance;

	static void __dealloc__(Quat* self);
	static PyObject* __new__(PyTypeObject *type, PyObject *args, PyObject *kwds);
	static int __init__(Quat *self, PyObject *args, PyObject *kwds);
	static PyObject* __str__(Quat *obj);
	static Quat* __mul__(Quat* a, Quat* b);

	static PyObject* dot(PyObject * self, PyObject * args);
	static PyObject* slerp(PyObject * self, PyObject * args);
	static PyObject* toEular(PyObject * self, PyObject * args);

	QuatPy();

	static PyObject* New(float x = 0, float y = 0, float z = 0, float w = 1);
	static PyObject* New(const Quaternionf& q);

	static Quat* cast(PyObject* pobj);
};

class ShapePy
{
public:
	struct _Shape
	{
		PyObject_HEAD;
		Vec3Py::Vec3* minPos;
		Vec3Py::Vec3* maxPos;
		PyObject* type;
	};
	static PyMemberDef PyShape_Members[4];
	static PyTypeObject PyShape_Type;
	static ShapePy instance;

	static void Shape_dealloc(_Shape* self);
	static PyObject* Shape_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
	static int Shape_init(_Shape *self, PyObject *args, PyObject *kwds);
	static PyObject* Shape_str(_Shape *obj);

	ShapePy();
};

struct PyCPointer
{
	PyObject_HEAD;
	void* nativeHandle;
};

class MeshPy
{
public:
	static PyMemberDef PyMesh_Members[4];
	static PyTypeObject PyMesh_Type;
	static MeshPy instance;

	static void Mesh_dealloc(PyCPointer* self);
	static PyObject* Mesh_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
	static int Mesh_init(PyCPointer *self, PyObject *args, PyObject *kwds);
	static PyObject* Mesh_str(PyCPointer *obj);
};

class ObjectPy
{
public:
	static PyMemberDef Members[2];
	static PyMethodDef Methods[6];
	static PyTypeObject Type;
	static ObjectPy instance;

	static void __dealloc__(PyObject* self);
	static PyObject* __new__(PyTypeObject *type, PyObject *args, PyObject *kwds);
	static int __init__(PyCPointer *self, PyObject *args, PyObject *kwds);
	static PyObject* __str__(PyCPointer *obj);

	static PyObject * castTo(PyObject * self, PyObject * args);
	static PyObject * getName(PyObject * self, PyObject * args);
	static PyObject * getChild(PyObject * self, PyObject * args);
	static PyObject * getChildCount(PyObject * self, PyObject * args);
	static PyObject * destroy(PyObject * self, PyObject * args);

	template<class T>
	static T* cast(PyObject* pyCPtr)
	{
		return dynamic_cast<T*>((Object*)((PyCPointer*)pyCPtr)->nativeHandle);
	}

	static PyObject* New(PyTypeObject* type, void* ptr);

	ObjectPy();
};

class InputPy
{
public:
	static PyMethodDef Methods[14];
	static PyTypeObject Type;
	static InputPy instance;

	static void __dealloc__(PyObject* self);
	static PyObject* __new__(PyTypeObject* type, PyObject* args, PyObject* kwds);
	static int __init__(PyCPointer* self, PyObject* args, PyObject* kwds);
	static PyObject* __str__(PyCPointer *obj);

	static PyObject* getKeyUp(PyObject * self, PyObject * args);
	static PyObject* getKeyDown(PyObject * self, PyObject * args);
	static PyObject* getKeyPress(PyObject * self, PyObject * args);
	static PyObject* getKeyRelease(PyObject * self, PyObject * args);
	static PyObject* getMousePos(PyObject * self, PyObject * args);
	static PyObject* getCursorPos(PyObject * self, PyObject * args);
	static PyObject* getMouseMove(PyObject * self, PyObject * args);
	static PyObject* getCursorMove(PyObject * self, PyObject * args);
	static PyObject* getMouseWheelValue(PyObject * self, PyObject * args);
	static PyObject* getMouseButtonUp(PyObject * self, PyObject * args);
	static PyObject* getMouseButtonDown(PyObject * self, PyObject * args);
	static PyObject* getMouseButtonPress(PyObject * self, PyObject * args);
	static PyObject* getMouseButtonRelease(PyObject * self, PyObject * args);

	InputPy();
};

class TransformPy
{
public:
	static PyMethodDef Methods[16];
	static PyTypeObject Type;
	static TransformPy instance;

	static PyObject* __str__(PyCPointer *obj);

	static PyObject* getPosition(PyObject * self, PyObject * args);
	static PyObject* getEulerAngle(PyObject * self, PyObject * args);
	static PyObject* getScale(PyObject * self, PyObject * args);
	static PyObject* getForward(PyObject * self, PyObject * args);
	static PyObject* getLeftward(PyObject * self, PyObject * args);
	static PyObject* getUpward(PyObject * self, PyObject * args);
	static PyObject* setPosition(PyObject * self, PyObject * args);
	static PyObject* setRotation(PyObject * self, PyObject * args);
	static PyObject* setScale(PyObject * self, PyObject * args);
	static PyObject* translate(PyObject * self, PyObject * args);
	static PyObject* rotate(PyObject * self, PyObject * args);
	static PyObject* scale(PyObject* self, PyObject* args);
	static PyObject* makeSphereConstraint(PyObject* self, PyObject* args);
	static PyObject* makeFixedConstraint(PyObject* self, PyObject* args);
	static PyObject* makeD6Constraint(PyObject * self, PyObject * args);

	TransformPy();
};

class CameraPy
{
public:
	static PyMethodDef Methods[8];
	static PyTypeObject Type;
	static CameraPy instance;

	static PyObject* __str__(PyCPointer *obj);

	static PyObject* getDistance(PyObject * self, PyObject * args);
	static PyObject* getFov(PyObject * self, PyObject * args);
	static PyObject* getMode(PyObject * self, PyObject * args);
	static PyObject* setDistance(PyObject * self, PyObject * args);
	static PyObject* setFov(PyObject * self, PyObject * args);
	static PyObject* setMode(PyObject * self, PyObject * args);
	static PyObject* isActive(PyObject * self, PyObject * args);

	CameraPy();
};

class WorldPy
{
public:
	static PyMethodDef Methods[9];
	static PyTypeObject Type;
	static WorldPy instance;

	static PyObject* __str__(PyCPointer *obj);

	static PyObject* getCurrentCamera(PyObject * self, PyObject * args);
	static PyObject* getDefaultCamera(PyObject * self, PyObject * args);
	static PyObject* getInput(PyObject * self, PyObject * args);
	static PyObject* switchCamera(PyObject * self, PyObject * args);
	static PyObject* switchToDefaultCamera(PyObject * self, PyObject * args);
	static PyObject* setMainVolume(PyObject * self, PyObject * args);
	static PyObject* quit(PyObject * self, PyObject * args);
	static PyObject* willQuit(PyObject * self, PyObject * args);
	
	WorldPy();
};

class ActorPy
{
public:
	static PyMethodDef Methods[6];
	static PyTypeObject Type;
	static ActorPy instance;

	static PyObject* __str__(PyCPointer *obj);

	static PyObject * isHidden(PyObject * self, PyObject * args);
	static PyObject * setHidden(PyObject * self, PyObject * args);

	static PyObject * playAudio(PyObject * self, PyObject * args);
	static PyObject * pauseAudio(PyObject * self, PyObject * args);
	static PyObject * stopAudio(PyObject * self, PyObject * args);

	ActorPy();
};

class MeshActorPy
{
public:
	static PyMethodDef Methods[1];
	static PyTypeObject Type;
	static MeshActorPy instance;

	static PyObject* __str__(PyCPointer *obj);

	MeshActorPy();
};

class SkeletonMeshActorPy
{
public:
	static PyMethodDef Methods[5];
	static PyTypeObject Type;
	static SkeletonMeshActorPy instance;

	static PyObject* __str__(PyCPointer *obj);

	static PyObject * playAnimation(PyObject * self, PyObject * args);
	static PyObject * pauseAnimation(PyObject * self, PyObject * args);
	static PyObject * stopAnimation(PyObject * self, PyObject * args);
	static PyObject * setBlendSpaceWeight(PyObject * self, PyObject * args);

	SkeletonMeshActorPy();
};

//class CharacterPy
//{
//public:
//	static PyMethodDef Methods[4];
//	static PyTypeObject Type;
//	static CharacterPy instance;
//
//	static PyObject* __str__(PyCPointer *obj);
//
//	static PyObject * isFly(PyObject * self, PyObject * args);
//	static PyObject * move(PyObject * self, PyObject * args);
//	static PyObject * jump(PyObject * self, PyObject * args);
//
//	CharacterPy();
//};

#endif // !_PYTHONMANAGER_H_
