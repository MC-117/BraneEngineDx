#include "PhysicsNode.h"
#include "../../Engine.h"

SerializeInstance(ContactInfoPin);

ContactInfoPin::ContactInfoPin(const string& name) : GraphPin(name)
{
}

bool ContactInfoPin::process(GraphContext& context)
{
    if (isOutput)
        return true;
    ContactInfoPin* input = dynamic_cast<ContactInfoPin*>(getConnectedPin());
    if (input) {
        contact = input->contact;
    }
    return true;
}

Serializable* ContactInfoPin::instantiate(const SerializationInfo& from)
{
    return new ContactInfoPin(from.name);
}

SerializeInstance(BreakContactInfoNode);

BreakContactInfoNode::BreakContactInfoNode()
{
    flag = Flag::Expression;
    displayName = "BreakContactInfo";

    contactPin = new ContactInfoPin("Contact");
    namePin = new StringPin("Name");
    locationPin = new Vector3fPin("Location");
    normalPin = new Vector3fPin("Normal");
    impactPin = new Vector3fPin("Impact");
    transPin = new TransformRefPin("Transform");

    addInput(*contactPin);
    addOutput(*namePin);
    addOutput(*locationPin);
    addOutput(*normalPin);
    addOutput(*impactPin);
    addOutput(*transPin);
}

bool BreakContactInfoNode::process(GraphContext& context)
{
    ContactInfo& contact = contactPin->contact;
    namePin->setValue(contact.objectName);
    locationPin->setValue(contact.location);
    normalPin->setValue(contact.normal);
    impactPin->setValue(contact.impact);
    transPin->setValue(contact.otherObject);
    return true;
}

Serializable* BreakContactInfoNode::instantiate(const SerializationInfo& from)
{
    return new BreakContactInfoNode();
}

SerializeInstance(AddForceNode);

AddForceNode::AddForceNode()
{
    displayName = "AddForce";
    transPin = new TransformRefPin("Transform");
    forcePin = new Vector3fPin("Force");

    addInput(*transPin);
    addInput(*forcePin);
}

bool AddForceNode::process(GraphContext& context)
{
    Transform* trans = transPin->getRef();
    if (trans == NULL)
        return false;
    RigidBody* rigid = trans->rigidBody;
    if (rigid == NULL)
        return false;
    rigid->addForce(forcePin->getValue());
    return true;
}

Serializable* AddForceNode::instantiate(const SerializationInfo& from)
{
    return new AddForceNode();
}

SerializeInstance(AddForceAtLocationNode);

AddForceAtLocationNode::AddForceAtLocationNode()
{
    displayName = "AddForceAtLocation";
    transPin = new TransformRefPin("Transform");
    forcePin = new Vector3fPin("Force");
    locationPin = new Vector3fPin("Location");

    addInput(*transPin);
    addInput(*forcePin);
    addInput(*locationPin);
}

bool AddForceAtLocationNode::process(GraphContext& context)
{
    Transform* trans = transPin->getRef();
    if (trans == NULL)
        return false;
    RigidBody* rigid = trans->rigidBody;
    if (rigid == NULL)
        return false;
    rigid->addForceAtLocation(forcePin->getValue(), locationPin->getValue());
    return true;
}

Serializable* AddForceAtLocationNode::instantiate(const SerializationInfo& from)
{
    return new AddForceAtLocationNode();
}

SerializeInstance(AddImpulseNode);

AddImpulseNode::AddImpulseNode()
{
    displayName = "AddImpulse";
    transPin = new TransformRefPin("Transform");
    impulsePin = new Vector3fPin("Impulse");

    addInput(*transPin);
    addInput(*impulsePin);
}

bool AddImpulseNode::process(GraphContext& context)
{
    Transform* trans = transPin->getRef();
    if (trans == NULL)
        return false;
    RigidBody* rigid = trans->rigidBody;
    if (rigid == NULL)
        return false;
    rigid->addImpulse(impulsePin->getValue());
    return true;
}

Serializable* AddImpulseNode::instantiate(const SerializationInfo& from)
{
    return new AddImpulseNode();
}

SerializeInstance(AddImpulseAtLocationNode);

AddImpulseAtLocationNode::AddImpulseAtLocationNode()
{
    displayName = "AddImpulse";
    transPin = new TransformRefPin("Transform");
    impulsePin = new Vector3fPin("Impulse");
    locationPin = new Vector3fPin("Location");

    addInput(*transPin);
    addInput(*impulsePin);
    addInput(*locationPin);
}

bool AddImpulseAtLocationNode::process(GraphContext& context)
{
    Transform* trans = transPin->getRef();
    if (trans == NULL)
        return false;
    RigidBody* rigid = trans->rigidBody;
    if (rigid == NULL)
        return false;
    rigid->addImpulseAtLocation(impulsePin->getValue(), locationPin->getValue());
    return true;
}

Serializable* AddImpulseAtLocationNode::instantiate(const SerializationInfo& from)
{
    return new AddImpulseAtLocationNode();
}

SerializeInstance(RayCastNode);

RayCastNode::RayCastNode()
{
    displayName = "RayCast";
    startPin = new Vector3fPin("Start");
    endPin = new Vector3fPin("End");
    successPin = new BoolPin("Success");
    contactPin = new ContactInfoPin("Contact");

    addInput(*startPin);
    addInput(*endPin);
    addOutput(*successPin);
    addOutput(*contactPin);
}

bool RayCastNode::process(GraphContext& context)
{
    World* world = Engine::getCurrentWorld();
    if (world == NULL)
        return false;
    successPin->setValue(world->physicalWorld.rayTest(
        startPin->getValue(), endPin->getValue(), contactPin->contact));
    return true;
}

Serializable* RayCastNode::instantiate(const SerializationInfo& from)
{
    return new RayCastNode();
}

SerializeInstance(SphereSweepNode);

SphereSweepNode::SphereSweepNode()
{
    displayName = "SphereSweep";
    radiusPin = new FloatPin("Radius");
    startPin = new Vector3fPin("Start");
    endPin = new Vector3fPin("End");
    successPin = new BoolPin("Success");
    contactPin = new ContactInfoPin("Contact");

    addInput(*radiusPin);
    addInput(*startPin);
    addInput(*endPin);
    addOutput(*successPin);
    addOutput(*contactPin);
}

bool SphereSweepNode::process(GraphContext& context)
{
    World* world = Engine::getCurrentWorld();
    if (world == NULL)
        return false;
    successPin->setValue(world->physicalWorld.sweep(Sphere(radiusPin->getValue()),
        startPin->getValue(), endPin->getValue(), contactPin->contact));
    return true;
}

Serializable* SphereSweepNode::instantiate(const SerializationInfo& from)
{
    return new SphereSweepNode();
}
