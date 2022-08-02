#include "TransformNode.h"
#include "../../Transform.h"

IMP_REF_PIN(Transform, TransformRefPin, Color(218, 0, 183));
IMP_REF_VAR_CLASS(Transform, TransformRefVariable, TransformRefPin, Color(218, 0, 183));

SerializeInstance(TransformGetWorldPositionNode);

TransformGetWorldPositionNode::TransformGetWorldPositionNode()
{
	flag = Flag::Expression;
	displayName = "GetWorldPosition";
	transformPin = new TransformRefPin("Transform");
	valuePin = new Vector3fPin("Position");
	addInput(*transformPin);
	addOutput(*valuePin);
}

bool TransformGetWorldPositionNode::process(GraphContext& context)
{
	Transform* trans = transformPin->getRef();
	if (trans)
		valuePin->setValue(trans->getPosition(WORLD));
	else
		valuePin->setValue(Vector3f::Zero());
	return true;
}

Serializable* TransformGetWorldPositionNode::instantiate(const SerializationInfo& from)
{
	return new TransformGetWorldPositionNode();
}

SerializeInstance(TransformGetLocalPositionNode);

TransformGetLocalPositionNode::TransformGetLocalPositionNode()
{
	flag = Flag::Expression;
	displayName = "GetLocalPosition";
	transformPin = new TransformRefPin("Transform");
	valuePin = new Vector3fPin("Position");
	addInput(*transformPin);
	addOutput(*valuePin);
}

bool TransformGetLocalPositionNode::process(GraphContext& context)
{
	Transform* trans = transformPin->getRef();
	if (trans)
		valuePin->setValue(trans->getPosition(LOCAL));
	else
		valuePin->setValue(Vector3f::Zero());
	return true;
}

Serializable* TransformGetLocalPositionNode::instantiate(const SerializationInfo& from)
{
	return new TransformGetLocalPositionNode();
}

SerializeInstance(TransformGetWorldEularAngleNode);

TransformGetWorldEularAngleNode::TransformGetWorldEularAngleNode()
{
	flag = Flag::Expression;
	displayName = "GetWorldEularAngle";
	transformPin = new TransformRefPin("Transform");
	valuePin = new Vector3fPin("EularAngle");
	addInput(*transformPin);
	addOutput(*valuePin);
}

bool TransformGetWorldEularAngleNode::process(GraphContext& context)
{
	Transform* trans = transformPin->getRef();
	if (trans)
		valuePin->setValue(trans->getEulerAngle(WORLD));
	else
		valuePin->setValue(Vector3f::Zero());
	return true;
}

Serializable* TransformGetWorldEularAngleNode::instantiate(const SerializationInfo& from)
{
	return new TransformGetWorldEularAngleNode();
}

SerializeInstance(TransformGetLocalEularAngleNode);

TransformGetLocalEularAngleNode::TransformGetLocalEularAngleNode()
{
	flag = Flag::Expression;
	displayName = "GetLocalEularAngle";
	transformPin = new TransformRefPin("Transform");
	valuePin = new Vector3fPin("EularAngle");
	addInput(*transformPin);
	addOutput(*valuePin);
}

bool TransformGetLocalEularAngleNode::process(GraphContext& context)
{
	Transform* trans = transformPin->getRef();
	if (trans)
		valuePin->setValue(trans->getEulerAngle(LOCAL));
	else
		valuePin->setValue(Vector3f::Zero());
	return true;
}

Serializable* TransformGetLocalEularAngleNode::instantiate(const SerializationInfo& from)
{
	return new TransformGetLocalEularAngleNode();
}

SerializeInstance(TransformGetWorldRotationNode);

TransformGetWorldRotationNode::TransformGetWorldRotationNode()
{
	flag = Flag::Expression;
	displayName = "GetWorldRotation";
	transformPin = new TransformRefPin("Transform");
	valuePin = new QuaternionfPin("Rotation");
	addInput(*transformPin);
	addOutput(*valuePin);
}

bool TransformGetWorldRotationNode::process(GraphContext& context)
{
	Transform* trans = transformPin->getRef();
	if (trans)
		valuePin->setValue(trans->getRotation(WORLD));
	else
		valuePin->setValue(Quaternionf::Identity());
	return true;
}

Serializable* TransformGetWorldRotationNode::instantiate(const SerializationInfo& from)
{
	return new TransformGetWorldRotationNode();
}

SerializeInstance(TransformGetLocalRotationNode);

TransformGetLocalRotationNode::TransformGetLocalRotationNode()
{
	flag = Flag::Expression;
	displayName = "GetLocalRotation";
	transformPin = new TransformRefPin("Transform");
	valuePin = new QuaternionfPin("Rotation");
	addInput(*transformPin);
	addOutput(*valuePin);
}

bool TransformGetLocalRotationNode::process(GraphContext& context)
{
	Transform* trans = transformPin->getRef();
	if (trans)
		valuePin->setValue(trans->getRotation(LOCAL));
	else
		valuePin->setValue(Quaternionf::Identity());
	return true;
}

Serializable* TransformGetLocalRotationNode::instantiate(const SerializationInfo& from)
{
	return new TransformGetLocalRotationNode();
}

SerializeInstance(TransformGetForwardNode);

TransformGetForwardNode::TransformGetForwardNode()
{
	flag = Flag::Expression;
	displayName = "GetForward";
	transformPin = new TransformRefPin("Transform");
	valuePin = new Vector3fPin("Forward");
	addInput(*transformPin);
	addOutput(*valuePin);
}

bool TransformGetForwardNode::process(GraphContext& context)
{
	Transform* trans = transformPin->getRef();
	if (trans)
		valuePin->setValue(trans->getForward(WORLD));
	else
		valuePin->setValue(Vector3f::Zero());
	return true;
}

Serializable* TransformGetForwardNode::instantiate(const SerializationInfo& from)
{
	return new TransformGetForwardNode();
}

SerializeInstance(TransformGetRightwardNode);

TransformGetRightwardNode::TransformGetRightwardNode()
{
	flag = Flag::Expression;
	displayName = "GetRightward";
	transformPin = new TransformRefPin("Transform");
	valuePin = new Vector3fPin("Rightward");
	addInput(*transformPin);
	addOutput(*valuePin);
}

bool TransformGetRightwardNode::process(GraphContext& context)
{
	Transform* trans = transformPin->getRef();
	if (trans)
		valuePin->setValue(trans->getRightward(WORLD));
	else
		valuePin->setValue(Vector3f::Zero());
	return true;
}

Serializable* TransformGetRightwardNode::instantiate(const SerializationInfo& from)
{
	return new TransformGetRightwardNode();
}

SerializeInstance(TransformGetUpwardNode);

TransformGetUpwardNode::TransformGetUpwardNode()
{
	flag = Flag::Expression;
	displayName = "GetUpward";
	transformPin = new TransformRefPin("Transform");
	valuePin = new Vector3fPin("Upward");
	addInput(*transformPin);
	addOutput(*valuePin);
}

bool TransformGetUpwardNode::process(GraphContext& context)
{
	Transform* trans = transformPin->getRef();
	if (trans)
		valuePin->setValue(trans->getUpward(WORLD));
	else
		valuePin->setValue(Vector3f::Zero());
	return true;
}

Serializable* TransformGetUpwardNode::instantiate(const SerializationInfo& from)
{
	return new TransformGetUpwardNode();
}

SerializeInstance(TransformSetWorldPositionNode);

TransformSetWorldPositionNode::TransformSetWorldPositionNode()
{
	displayName = "SetWorldPosition";
	transformPin = new TransformRefPin("Transform");
	valuePin = new Vector3fPin("Position");
	addInput(*transformPin);
	addInput(*valuePin);
}

bool TransformSetWorldPositionNode::process(GraphContext& context)
{
	Transform* trans = transformPin->getRef();
	if (trans)
		trans->setPosition(valuePin->getValue(), WORLD);
	return true;
}

Serializable* TransformSetWorldPositionNode::instantiate(const SerializationInfo& from)
{
	return new TransformSetWorldPositionNode();
}

SerializeInstance(TransformSetLocalPositionNode);

TransformSetLocalPositionNode::TransformSetLocalPositionNode()
{
	displayName = "SetLocalPosition";
	transformPin = new TransformRefPin("Transform");
	valuePin = new Vector3fPin("Position");
	addInput(*transformPin);
	addInput(*valuePin);
}

bool TransformSetLocalPositionNode::process(GraphContext& context)
{
	Transform* trans = transformPin->getRef();
	if (trans)
		trans->setPosition(valuePin->getValue(), LOCAL);
	return true;
}

Serializable* TransformSetLocalPositionNode::instantiate(const SerializationInfo& from)
{
	return new TransformSetLocalPositionNode();
}

SerializeInstance(TransformSetWorldEularAngleNode);

TransformSetWorldEularAngleNode::TransformSetWorldEularAngleNode()
{
	displayName = "SetWorldEularAngle";
	transformPin = new TransformRefPin("Transform");
	valuePin = new Vector3fPin("EularAngle");
	addInput(*transformPin);
	addInput(*valuePin);
}

bool TransformSetWorldEularAngleNode::process(GraphContext& context)
{
	Transform* trans = transformPin->getRef();
	if (trans)
		trans->setRotation(valuePin->getValue(), WORLD);
	return true;
}

Serializable* TransformSetWorldEularAngleNode::instantiate(const SerializationInfo& from)
{
	return new TransformSetWorldEularAngleNode();
}

SerializeInstance(TransformSetLocalEularAngleNode);

TransformSetLocalEularAngleNode::TransformSetLocalEularAngleNode()
{
	displayName = "SetLocalEularAngle";
	transformPin = new TransformRefPin("Transform");
	valuePin = new Vector3fPin("EularAngle");
	addInput(*transformPin);
	addInput(*valuePin);
}

bool TransformSetLocalEularAngleNode::process(GraphContext& context)
{
	Transform* trans = transformPin->getRef();
	if (trans)
		trans->setRotation(valuePin->getValue(), LOCAL);
	return true;
}

Serializable* TransformSetLocalEularAngleNode::instantiate(const SerializationInfo& from)
{
	return new TransformSetLocalEularAngleNode();
}

SerializeInstance(TransformSetWorldRotationNode);

TransformSetWorldRotationNode::TransformSetWorldRotationNode()
{
	displayName = "SetWorldRotation";
	transformPin = new TransformRefPin("Transform");
	valuePin = new QuaternionfPin("Rotation");
	addInput(*transformPin);
	addInput(*valuePin);
}

bool TransformSetWorldRotationNode::process(GraphContext& context)
{
	Transform* trans = transformPin->getRef();
	if (trans)
		trans->setRotation(valuePin->getValue(), WORLD);
	return true;
}

Serializable* TransformSetWorldRotationNode::instantiate(const SerializationInfo& from)
{
	return new TransformSetWorldRotationNode();
}

SerializeInstance(TransformSetLocalRotationNode);

TransformSetLocalRotationNode::TransformSetLocalRotationNode()
{
	displayName = "SetLocalRotation";
	transformPin = new TransformRefPin("Transform");
	valuePin = new QuaternionfPin("Rotation");
	addInput(*transformPin);
	addInput(*valuePin);
}

bool TransformSetLocalRotationNode::process(GraphContext& context)
{
	Transform* trans = transformPin->getRef();
	if (trans)
		trans->setRotation(valuePin->getValue(), LOCAL);
	return true;
}

Serializable* TransformSetLocalRotationNode::instantiate(const SerializationInfo& from)
{
	return new TransformSetLocalRotationNode();
}

SerializeInstance(TransformTranslateWorldNode);

TransformTranslateWorldNode::TransformTranslateWorldNode()
{
	displayName = "TranslateWorld";
	transformPin = new TransformRefPin("Transform");
	valuePin = new Vector3fPin("Transition");
	addInput(*transformPin);
	addInput(*valuePin);
}

bool TransformTranslateWorldNode::process(GraphContext& context)
{
	Transform* trans = transformPin->getRef();
	if (trans)
		trans->translate(valuePin->getValue(), WORLD);
	return true;
}

Serializable* TransformTranslateWorldNode::instantiate(const SerializationInfo& from)
{
	return new TransformTranslateWorldNode();
}

SerializeInstance(TransformTranslateLocalNode);

TransformTranslateLocalNode::TransformTranslateLocalNode()
{
	displayName = "TranslateLocal";
	transformPin = new TransformRefPin("Transform");
	valuePin = new Vector3fPin("Transition");
	addInput(*transformPin);
	addInput(*valuePin);
}

bool TransformTranslateLocalNode::process(GraphContext& context)
{
	Transform* trans = transformPin->getRef();
	if (trans)
		trans->translate(valuePin->getValue(), LOCAL);
	return true;
}

Serializable* TransformTranslateLocalNode::instantiate(const SerializationInfo& from)
{
	return new TransformTranslateLocalNode();
}

SerializeInstance(TransformRotateEularWorldNode);

TransformRotateEularWorldNode::TransformRotateEularWorldNode()
{
	displayName = "RotateEularWorld";
	transformPin = new TransformRefPin("Transform");
	valuePin = new Vector3fPin("Eular");
	addInput(*transformPin);
	addInput(*valuePin);
}

bool TransformRotateEularWorldNode::process(GraphContext& context)
{
	Transform* trans = transformPin->getRef();
	if (trans)
		trans->rotate(valuePin->getValue(), WORLD);
	return true;
}

Serializable* TransformRotateEularWorldNode::instantiate(const SerializationInfo& from)
{
	return new TransformRotateEularWorldNode();
}

SerializeInstance(TransformRotateLocalEularNode);

TransformRotateLocalEularNode::TransformRotateLocalEularNode()
{
	displayName = "RotateLocalEular";
	transformPin = new TransformRefPin("Transform");
	valuePin = new Vector3fPin("Eular");
	addInput(*transformPin);
	addInput(*valuePin);
}

bool TransformRotateLocalEularNode::process(GraphContext& context)
{
	Transform* trans = transformPin->getRef();
	if (trans)
		trans->rotate(valuePin->getValue(), LOCAL);
	return true;
}

Serializable* TransformRotateLocalEularNode::instantiate(const SerializationInfo& from)
{
	return new TransformRotateLocalEularNode();
}

SerializeInstance(TransformRotateNode);

TransformRotateNode::TransformRotateNode()
{
	displayName = "Rotate";
	transformPin = new TransformRefPin("Transform");
	valuePin = new QuaternionfPin("Rotation");
	addInput(*transformPin);
	addInput(*valuePin);
}

bool TransformRotateNode::process(GraphContext& context)
{
	Transform* trans = transformPin->getRef();
	if (trans)
		trans->rotate(valuePin->getValue());
	return true;
}

Serializable* TransformRotateNode::instantiate(const SerializationInfo& from)
{
	return new TransformRotateNode();
}
