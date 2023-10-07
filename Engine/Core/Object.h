#pragma once
#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "Delegate.h"
#include "Events.h"
#include "Render.h"
#include "Base.h"

class Render;
struct RenderInfo;
class RenderCommandList;
#if ENABLE_PHYSICS
class PhysicalWorld;
class PhysicalBody;
class PhysicalConstraint;
#endif

class Object;

class ENGINE_API ObjectBehavior : public Base
{
	friend class Object;
public:
	Serialize(ObjectBehavior, Base);

	ObjectBehavior() = default;
	virtual ~ObjectBehavior() = default;

	virtual string getName() const;

	virtual bool init(Object* object);

	virtual void begin();
	virtual void tick(float deltaTime);
	virtual void afterTick();
	virtual void prerender(SceneRenderData& sceneData);
	virtual void end();

	virtual void onAttacted(Object& parent);
	virtual void onDistacted(Object& parent);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Object* object = NULL;
};

class ENGINE_API Object : public Base
{
public:
	Serialize(Object, Base);

	enum struct DestroyFlag : unsigned char {
		None, Self, All
	};
	string name;
	Object* parent = NULL;
	vector<Object*> children;
	Events events;
	vector<ObjectBehavior*> behaviors;

	Object(string name = "Object");
	virtual ~Object();

	virtual void begin();
	virtual void tick(float deltaTime);
	virtual void afterTick();
	virtual void prerender(SceneRenderData& sceneData);
	virtual void end();

	virtual void onAttacted(Object& parent);
	virtual void onDistacted(Object& parent);

	virtual bool addBehavior(ObjectBehavior* behavior);
	virtual bool removeBehavior(ObjectBehavior* behavior);
	int getBehaviorCount() const;
	ObjectBehavior* getBehavior(int index) const;
	ObjectBehavior* getBehavior(const string& name) const;
	ObjectBehavior* getBehavior(Serialization& type) const;

	int getObjectID() const;
	virtual Render* getRender();
	virtual unsigned int getRenders(vector<Render*>& renders);
#if ENABLE_PHYSICS
	virtual PhysicalBody* getPhysicalBody();
	virtual PhysicalConstraint* getPhysicalConstraint();
	virtual void setupPhysics(PhysicalWorld& physicalWorld);
	virtual void releasePhysics(PhysicalWorld& physicalWorld);
#endif

	Object* getRoot() const;
	Object* getSibling() const;
	Object* getChild() const;
	Object* findChild(const string& name) const;
	Object* findChildRecursively(const string& name) const;
	virtual void setParent(Object& parent);
	virtual void unparent();
	virtual void addChild(Object& child);
	void clearChild();

	virtual void destroy(bool applyToChild = false);
	bool isDestroy();
	DestroyFlag getDestroyFlag() const;
	bool isinitialized();
	bool isInternal();

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	bool initialized = false;
	DestroyFlag destroyFlag = DestroyFlag::None;
	bool internalNode = false; // internal node: can not change its hierarchy
	int siblingIdx = -1;
	int objectID = -1;

	virtual void addInternalNode(Object& object);
};

class ENGINE_API ObjectIterator
{
public:
	ObjectIterator(Object* root);

	bool next();
	Object& current();
	Object* unparentCurrent();
	void reset(Object* root = NULL);
protected:
	Object* root = NULL, * curObj = NULL;
	bool delay = false;
};

class ENGINE_API ObjectConstIterator
{
public:
	ObjectConstIterator(const Object* root);

	bool next();
	Object& current();
	void reset(const Object* root = NULL);
protected:
	const Object* root = NULL;
	Object* curObj = NULL;
};

enum InstanceRuleEnum
{
	IR_Default, IR_WorldUniqueName, IR_ExistUniqueName
};

ENGINE_API void ChildrenInstantiateObject(const SerializationInfo& from, Object* pObj, InstanceRuleEnum rule = IR_Default);

#define ChildrenInstantiate(Type, from, pObj) ChildrenInstantiate##Type(from, pObj);

#endif // !_OBJECT_H_
