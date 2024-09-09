#include "Object.h"
#include "Asset.h"
#include "Brane.h"
#include "Graph/ObjectGraph.h"

SerializeInstance(ObjectBehavior);

string ObjectBehavior::getName() const
{
	return getSerialization().type;
}

bool ObjectBehavior::init(Object* object)
{
	this->object = object;
	return true;
}

void ObjectBehavior::begin()
{
}

void ObjectBehavior::tick(float deltaTime)
{
}

void ObjectBehavior::afterTick()
{
}

void ObjectBehavior::prerender(SceneRenderData& sceneData)
{
}

void ObjectBehavior::end()
{
}

void ObjectBehavior::onAttacted(Object& parent)
{
}

void ObjectBehavior::onDistacted(Object& parent)
{
}

Serializable* ObjectBehavior::instantiate(const SerializationInfo& from)
{
	return nullptr;
}

SerializeInstance(Object);

Object::Object(string name) : Base(), events(this)
{
	this->name = name;
}

Object::~Object()
{
	initialized = false;
	unparent();
	clearChild();
	for (int i = 0; i < behaviors.size(); i++) {
		ObjectBehavior* behavior = behaviors[i];
		delete behavior;
	}
	behaviors.clear();
}

void Object::begin()
{
	initialized = true;
	events.onBegin();
	for (int i = 0; i < behaviors.size(); i++) {
		ObjectBehavior* behavior = behaviors[i];
		behavior->begin();
	}
}

void Object::tick(float deltaTime)
{
	events.onTick(deltaTime);
	for (int i = 0; i < behaviors.size(); i++) {
		ObjectBehavior* behavior = behaviors[i];
		behavior->tick(deltaTime);
	}
}

void Object::afterTick()
{
	events.onAfterTick();
	for (int i = 0; i < behaviors.size(); i++) {
		ObjectBehavior* behavior = behaviors[i];
		behavior->afterTick();
	}
}

void Object::prerender(SceneRenderData& sceneData)
{
}

void Object::end()
{
	for (int i = 0; i < behaviors.size(); i++) {
		ObjectBehavior* behavior = behaviors[i];
		behavior->end();
	}
}

void Object::onAttacted(Object& parent)
{
}

void Object::onDistacted(Object& parent)
{
}

bool Object::addBehavior(ObjectBehavior* behavior)
{
	if (std::find(behaviors.begin(), behaviors.end(), behavior) != behaviors.end())
		return false;
	if (behavior->object == this)
		return false;
	behavior->init(this);
	behaviors.push_back(behavior);
	return true;
}

bool Object::removeBehavior(ObjectBehavior* behavior)
{
	auto iter = std::find(behaviors.begin(), behaviors.end(), behavior);
	if (iter == behaviors.end())
		return false;
	behaviors.erase(iter);
	delete behavior;
	return true;
}

int Object::getBehaviorCount() const
{
	return behaviors.size();
}

ObjectBehavior* Object::getBehavior(int index) const
{
	if (index < 0 || index >= behaviors.size())
		return NULL;
	return behaviors[index];
}

ObjectBehavior* Object::getBehavior(const string& name) const
{
	for (int i = 0; i < behaviors.size(); i++) {
		ObjectBehavior* behavior = behaviors[i];
		if (behavior->getName() == name)
			return behavior;
	}
	return NULL;
}

ObjectBehavior* Object::getBehavior(Serialization& type) const
{
	for (int i = 0; i < behaviors.size(); i++) {
		ObjectBehavior* behavior = behaviors[i];
		if (&behavior->getSerialization() == &type)
			return behavior;
	}
	return NULL;
}

Render* Object::getRender()
{
	return NULL;
}

unsigned int Object::getRenders(vector<Render*>& renders)
{
	return 0;
}

#if ENABLE_PHYSICS
PhysicalBody* Object::getPhysicalBody()
{
	return nullptr;
}
PhysicalConstraint* Object::getPhysicalConstraint()
{
	return nullptr;
}

void Object::setupPhysics(PhysicalWorld& physicalWorld)
{
}

void Object::releasePhysics(PhysicalWorld& physicalWorld)
{
}
#endif

Object* Object::getRoot() const
{
	if (parent == NULL)
		return NULL;
	Object* obj = parent;
	while (obj->parent != NULL)
		obj = obj->parent;
	return obj;
}

Object* Object::getSibling() const
{
	if (parent == NULL)
		return NULL;
	int i = siblingIdx + 1;
	if (i < parent->children.size())
		return parent->children[i];
	return NULL;
}

Object* Object::getChild() const
{
	if (children.size() != 0)
		return children[0];
	return NULL;
}

Object* Object::findChild(const string& name) const
{
	for (int i = 0; i < children.size(); i++) {
		Object* obj = children[i];
		if (obj->name == name)
			return obj;
	}
	return NULL;
}

Object* Object::findChildRecursively(const string& name) const
{
	Object* obj = findChild(name);
	if (obj != NULL)
		return obj;
	for (int i = 0; i < children.size(); i++) {
		obj = children[i]->findChildRecursively(name);
		if (obj != NULL)
			return obj;
	}
	return obj;
}

void Object::setParent(Object& parent)
{
	parent.addChild(*this);
}

void Object::unparent()
{
	if (internalNode)
		return;
	if (parent != NULL) {
		if (siblingIdx != -1) {
			auto b = parent->children.erase(parent->children.begin() + siblingIdx);
			for (auto e = parent->children.end(); b != e; b++)
				(*b)->siblingIdx--;
			onDistacted(*parent);
		}
		parent = NULL;
	}
	siblingIdx = -1;
}

void Object::addChild(Object& child)
{
	child.unparent();
	if (child.internalNode)
		return;
	child.parent = this;
	child.siblingIdx = children.size();
	children.push_back(&child);
	child.onAttacted(*this);
}

void Object::clearChild()
{
	for (auto b = children.begin(), e = children.end(); b != e; b++) {
		(*b)->parent = NULL;
		(*b)->siblingIdx = -1;
		(*b)->onDistacted(*this);
	}
	children.clear();
}

void Object::destroy(bool applyToChild)
{
	destroyFlag = applyToChild ? DestroyFlag::All : DestroyFlag::Self;
	for (auto b = children.begin(), e = children.end(); b != e; b++)
		(*b)->destroy(parent == NULL || applyToChild);
}

bool Object::isDestroy()
{
	return destroyFlag != DestroyFlag::None;
}

Object::DestroyFlag Object::getDestroyFlag() const
{
	return destroyFlag;
}

bool Object::isinitialized()
{
	return initialized;
}

bool Object::isInternal()
{
	return internalNode;
}

Serializable* Object::instantiate(const SerializationInfo& from)
{
	Object* obj = new Object(from.name);
	ChildrenInstantiate(Object, from, obj);
	return obj;
}

bool Object::deserialize(const SerializationInfo& from)
{
	if (!Base::deserialize(from))
		return false;
	const SerializationInfo* behaviorInfos = from.get("behaviors");
	if (behaviorInfos != NULL) {
		for (int i = 0; i < behaviors.size(); i++) {
			delete behaviors[i];
		}
		behaviors.clear();
		for (int i = 0; i < behaviorInfos->sublists.size(); i++) {
			const SerializationInfo& info = behaviorInfos->sublists[i];
			if (info.serialization) {
				Serializable* serializable = info.serialization->instantiate(info);
				if (serializable) {
					ObjectBehavior* behavior = dynamic_cast<ObjectBehavior*>(serializable);
					if (behavior) {
						behavior->deserialize(info);
						addBehavior(behavior);
					}
					else {
						delete serializable;
					}
				}
			}
		}
	}
	const SerializationInfo* childrenInfo = from.get("children");
	if (childrenInfo != NULL) {
		for (int i = 0; i < children.size(); i++) {
			const SerializationInfo* info = childrenInfo->get(children[i]->name);
			if (info != NULL)
				children[i]->deserialize(*info);
		}
	}
	return true;
}

bool Object::serialize(SerializationInfo& to)
{
	if (!Base::serialize(to))
		return false;
	to.name = name;
	SerializationInfo* behaviorInfos = to.add("behaviors");
	if (behaviorInfos != NULL) {
		for (int i = 0; i < behaviors.size(); i++) {
			SerializationInfo* info = behaviorInfos->add(to_string(i));
			if (info != NULL) {
				behaviors[i]->serialize(*info);
			}
		}
	}
	SerializationInfo* childernInfo = to.add("children");
	if (childernInfo != NULL) {
		for (int i = 0; i < children.size(); i++) {
			SerializationInfo* info = childernInfo->add(children[i]->name);
			if (info != NULL) {
				children[i]->serialize(*info);
			}
		}
	}
	return true;
}

void Object::addInternalNode(Object& object)
{
	object.internalNode = true;
	object.parent = this;
	object.siblingIdx = children.size();
	children.push_back(&object);
}

ObjectIterator::ObjectIterator(Object* root)
{
	this->root = root;
	curObj = root;
}

bool ObjectIterator::next()
{
	if (delay) {
		delay = false;
		return curObj != NULL;
	}
	if (curObj == NULL)
		return false;
	Object* o = curObj->getChild();
	if (o == NULL) {
		if (curObj == root)
			return false;
		o = curObj->getSibling();
		if (o == NULL) {
			o = curObj->parent;
			while (o != NULL) {
				if (o == root)
					return false;
				Object* s = o->getSibling();
				if (s == NULL)
					o = o->parent;
				else {
					curObj = s;
					return true;
				}
			}
			return false;
		}
		else {
			curObj = o;
			return true;
		}
	}
	else {
		curObj = o;
		return true;
	}
}

Object& ObjectIterator::current()
{
	return *curObj;
}

Object* ObjectIterator::unparentCurrent()
{
	if (curObj->parent != NULL)
		for (auto b = curObj->children.begin(), e = curObj->children.end(); b != e; b++)
			(*b)->setParent(*curObj->parent);
	Object* o = curObj->getSibling(), * bak = curObj;
	if (o == NULL) {
		o = curObj->parent;
		while (o != NULL) {
			if (o == root) {
				o = NULL;
				break;
			}
			Object* s = o->getSibling();
			if (s == NULL)
				o = o->parent;
			else {
				o = s;
				break;
			}
		}
	}
	curObj->unparent();
	curObj = o;
	delay = true;
	return bak;
}

void ObjectIterator::reset(Object* root)
{
	if (root == NULL)
		curObj = this->root;
	else
		curObj = this->root = root;
	delay = false;
}

ObjectConstIterator::ObjectConstIterator(const Object* root)
{
	this->root = root;
	curObj = (Object*)root;
}

bool ObjectConstIterator::next()
{
	if (curObj == NULL)
		return false;
	Object* o = curObj->getChild();
	if (o == NULL) {
		if (curObj == root)
			return false;
		o = curObj->getSibling();
		if (o == NULL) {
			o = curObj->parent;
			while (o != NULL) {
				if (o == root)
					return false;
				Object* s = o->getSibling();
				if (s == NULL)
					o = o->parent;
				else {
					curObj = s;
					return true;
				}
			}
			return false;
		}
		else {
			curObj = o;
			return true;
		}
	}
	else {
		curObj = o;
		return true;
	}
}

Object& ObjectConstIterator::current()
{
	return *curObj;
}

void ObjectConstIterator::reset(const Object* root)
{
	if (root == NULL)
		curObj = (Object*)this->root;
	else
		curObj = (Object*)(this->root = root);
}

void ChildrenInstantiateObject(const SerializationInfo& from, Object* pObj, InstanceRuleEnum rule)
{
	const SerializationInfo* ___child = from.get("children");
	if (___child != NULL) {
		set<string> names;
		if (rule == IR_ExistUniqueName) {
			for (int i = 0; i < pObj->children.size(); i++)
				names.insert(pObj->children[i]->name);
		}
		for (auto b = ___child->sublists.begin(), e = ___child->sublists.end(); b != e; b++) {
			if (b->serialization != NULL) {
				bool skip = false;
				for (int i = 0; i < pObj->children.size(); i++) {
					Object* obj = pObj->children[i];
					if (obj->isInternal() && obj->name == b->name) {
						ChildrenInstantiateObject(*b, obj);
						skip = true;
						break;
					}
				}
				if (rule == IR_Default) {}
				else if (rule == IR_ExistUniqueName) {
					for (int i = 0; i < pObj->children.size(); i++)
						if (pObj->children[i]->name == b->name) {
							skip = true;
							break;
						}
				}
				if (skip)
					continue;
				Serializable* ser = b->serialization->instantiate(*b);
				if (ser != NULL) {
					Object* cobj = dynamic_cast<Object*>(ser);
					if (cobj == NULL) {
						delete ser;
					}
					else
						pObj->addChild(*cobj);
				}
			}
		}
	}
}
