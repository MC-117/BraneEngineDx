#include "Object.h"

SerializeInstance(Object);

Object::Object(string name) : events(this)
{
	this->name = name;
	Brane::born(typeid(Object).hash_code(), this, name);
}

Object::~Object()
{
	end();
}

void Object::begin()
{
	initialized = true;
	events.onBegin();
}

void Object::tick(float deltaTime)
{
	events.onTick(deltaTime);
}

void Object::afterTick()
{
	events.onAfterTick();
}

void Object::prerender()
{
}

void Object::end()
{
	initialized = false;
	unparent();
	clearChild();
	Brane::vanish(typeid(Object).hash_code(), this);
}

void Object::onAttacted(Object& parent)
{
}

void Object::onDistacted(Object& parent)
{
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
void* Object::getPhysicalBody()
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

Object* Object::findChild(const string& name)
{
	if (this->name == name)
		return this;
	for (int i = 0; i < children.size(); i++) {
		Object* obj = children[i]->findChild(name);
		if (obj != NULL)
			return obj;
	}
	return NULL;
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

void Object::traverse(void(*preFunc)(Object& object), void(*postFunc)(Object& object))
{
	for (auto b = children.begin(), e = children.end(); b != e; b++) {
		preFunc(**b);
		(*b)->traverse(preFunc, postFunc);
		postFunc(**b);
	}
}

void Object::traverse(bool(*preFunc)(Object& object), void(*postFunc)(Object& object, bool))
{
	for (auto b = children.begin(), e = children.end(); b != e; b++) {
		bool run = preFunc(**b);
		if (run)
			(*b)->traverse(preFunc, postFunc);
		postFunc(**b, run);
	}
}

void Object::preTraverse(void(*func)(Object& object))
{
	for (auto b = children.begin(), e = children.end(); b != e; b++) {
		func(**b);
		(*b)->postTraverse(func);
	}
}

void Object::preTraverse(bool(*func)(Object& object))
{
	auto b = children.begin();
	while (b != children.end()) {
		if (func(**b)) {
			Object* root = getRoot();
			for (auto _b = (*b)->children.begin(), _e = (*b)->children.end(); _b != _e; _b++)
				(*_b)->setParent(*root);
			delete* b;
			b = children.erase(b);
		}
		else {
			(*b)->preTraverse(func);
			b++;
		}
	}
}

void Object::preTraverse(bool(*func)(Object& object, float), float arg)
{
	auto b = children.begin();
	while (b != children.end()) {
		if (func(**b, arg)) {
			Object* root = getRoot();
			for (auto _b = (*b)->children.begin(), _e = (*b)->children.end(); _b != _e; _b++)
				(*_b)->setParent(*root);
			b = children.erase(b);
			Brane::vanish(typeid(Object).hash_code(), *b);
		}
		else {
			(*b)->preTraverse(func, arg);
			b++;
		}
	}
}

void Object::postTraverse(void(*func)(Object& object))
{
	for (auto b = children.begin(), e = children.end(); b != e; b++) {
		(*b)->postTraverse(func);
		func(**b);
	}
}

void Object::destroy(bool applyToChild)
{
	tryDestroy = true;
	if (parent == NULL || applyToChild)
		for (auto b = children.begin(), e = children.end(); b != e; b++)
			(*b)->destroy(applyToChild);
	else
		while (!children.empty())
			children[0]->setParent(*parent);
	//clearChild();
}

bool Object::isDestroy()
{
	return tryDestroy;
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
	const SerializationInfo* child = from.get("children");
	if (child != NULL) {
		for (int i = 0; i < children.size(); i++) {
			const SerializationInfo* info = child->get(children[i]->name);
			if (info != NULL)
				children[i]->deserialize(*info);
		}
	}
	return true;
}

bool Object::serialize(SerializationInfo& to)
{
	to.type = "Object";
	to.name = name;
	SerializationInfo* child = to.add("children");
	if (child != NULL)
		for (int i = 0; i < children.size(); i++) {
			SerializationInfo* info = child->add(children[i]->name);
			if (info != NULL) {
				children[i]->serialize(*info);
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
				else if (rule == IR_WorldUniqueName) {
					if (Brane::find(typeid(Object).hash_code(), b->name) != NULL)
						skip = true;
				}
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
