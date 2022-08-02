#include "ObjectNode.h"
#include "ValueNode.h"

SerializeInstance(RefPin);

void RefPin::setToNull()
{
    castFromInternal(NULL);
}

void RefPin::resetToDefault()
{
    setToNull();
}

void RefPin::assign(const ValuePin* other)
{
    const RefPin* pin = dynamic_cast<const RefPin*>(other);
    if (pin) {
        castFrom(pin);
    }
}

bool RefPin::isConnectable(GraphPin* pin) const
{
    RefPin* refPin = dynamic_cast<RefPin*>(pin);
    if (refPin == NULL)
        return false;
    if (refPin->isOutput == isOutput || refPin->node == node)
        return false;
    if (&refPin->getRefSerialization() == &getRefSerialization())
        return true;
    if (isOutput) {
        return getRefSerialization().isChildOf(refPin->getRefSerialization());
    }
    else {
        return refPin->getRefSerialization().isChildOf(getRefSerialization());
    }
}

void RefPin::castFrom(const ValuePin* other)
{
    const RefPin* pin = dynamic_cast<const RefPin*>(other);
        if (pin == NULL)
            return;
    castFromInternal(pin->getBase());
}

void RefPin::setValue(Base* value)
{
    castFromInternal(value);
}

Base* RefPin::getValue() const
{
    return getBase();
}

bool RefPin::process(GraphContext& context)
{
    if (!isOutput) {
        GraphPin* pin = connectedPin;
        if (pin != NULL)
            castFromInternal(((RefPin*)pin)->getBase());
    }
    return true;
}

Serializable* RefPin::instantiate(const SerializationInfo& from)
{
    return nullptr;
}

RefPin::RefPin(const string& name) : ValuePin(name)
{
}

SerializeInstance(RefVariable);

RefVariable::RefVariable(const string& name) : GraphVariable(name)
{
}

Serializable* RefVariable::instantiate(const SerializationInfo& from)
{
    return NULL;
}

IMP_REF_PIN(Base, BaseRefPin, Color(218, 0, 183));
IMP_REF_PIN(Object, ObjectRefPin, Color(218, 0, 183));

IMP_REF_VAR_CLASS(Base, BaseRefVariable, BaseRefPin, Color(218, 0, 183));
IMP_REF_VAR_CLASS(Object, ObjectRefVariable, ObjectRefPin, Color(218, 0, 183));

IMP_MATH_COMPARE_NODE(BaseRefEqualNode);
IMP_MATH_COMPARE_NODE(BaseRefNotEqualNode);

SerializeInstance(ObjectGetNameNode);

ObjectGetNameNode::ObjectGetNameNode()
{
    flag = Flag::Expression;
    displayName = "GetName";
    objectPin = new ObjectRefPin("Object");
    stringPin = new StringPin("Name");
    addInput(*objectPin);
    addOutput(*stringPin);
}

bool ObjectGetNameNode::process(GraphContext& context)
{
    Object* object = dynamic_cast<Object*>(objectPin->getBase());
    if (object)
        stringPin->setValue(object->name);
    else
        stringPin->setValue("Null");
    return true;
}

Serializable* ObjectGetNameNode::instantiate(const SerializationInfo& from)
{
    return new ObjectGetNameNode();
}

SerializeInstance(ObjectGetChildCountNode);

ObjectGetChildCountNode::ObjectGetChildCountNode()
{
    flag = Flag::Expression;
    displayName = "GetChileCount";
    objectPin = new ObjectRefPin("Object");
    intPin = new IntPin("Count");
    addInput(*objectPin);
    addOutput(*intPin);
}

bool ObjectGetChildCountNode::process(GraphContext& context)
{
    Object* object = dynamic_cast<Object*>(objectPin->getBase());
    if (object)
        intPin->setValue(object->children.size());
    else
        intPin->setValue(0);
    return true;
}

Serializable* ObjectGetChildCountNode::instantiate(const SerializationInfo& from)
{
    return new ObjectGetChildCountNode();
}

SerializeInstance(ObjectGetChildNode);

ObjectGetChildNode::ObjectGetChildNode()
{
    flag = Flag::Expression;
    displayName = "GetChile";
    objectPin = new ObjectRefPin("Object");
    indexPin = new IntPin("Index");
    childPin = new ObjectRefPin("Child");
    addInput(*objectPin);
    addInput(*indexPin);
    addOutput(*childPin);
}

bool ObjectGetChildNode::process(GraphContext& context)
{
    Object* object = dynamic_cast<Object*>(objectPin->getBase());
    int index = indexPin->getValue();
    Ref<Object> ref;
    if (object && index < object->children.size())
        ref = object->children[index];
    childPin->setRef(ref);
    return true;
}

Serializable* ObjectGetChildNode::instantiate(const SerializationInfo& from)
{
    return new ObjectGetChildNode();
}
