#pragma once

#include "TransformNode.h"
#include "../../Physics/ContactInfo.h"

class ContactInfoPin : public GraphPin
{
public:
	Serialize(ContactInfoPin, GraphPin);

	ContactInfo contact;

	ContactInfoPin(const string& name);
	virtual ~ContactInfoPin() = default;

	virtual bool process(GraphContext & context);

	static Serializable* instantiate(const SerializationInfo& from);
};

class BreakContactInfoNode : public GraphNode
{
public:
	Serialize(BreakContactInfoNode, GraphNode);

	BreakContactInfoNode();
	virtual ~BreakContactInfoNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	ContactInfoPin* contactPin = NULL;
	StringPin* namePin = NULL;
	Vector3fPin* locationPin = NULL;
	Vector3fPin* normalPin = NULL;
	Vector3fPin* impactPin = NULL;
	TransformRefPin* transPin = NULL;
};

class AddForceNode : public InOutFlowNode
{
public:
	Serialize(AddForceNode, InOutFlowNode);

	AddForceNode();
	virtual ~AddForceNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	TransformRefPin* transPin = NULL;
	Vector3fPin* forcePin = NULL;
};

class AddForceAtLocationNode : public InOutFlowNode
{
public:
	Serialize(AddForceAtLocationNode, InOutFlowNode);

	AddForceAtLocationNode();
	virtual ~AddForceAtLocationNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	TransformRefPin* transPin = NULL;
	Vector3fPin* forcePin = NULL;
	Vector3fPin* locationPin = NULL;
};

class AddImpulseNode : public InOutFlowNode
{
public:
	Serialize(AddImpulseNode, InOutFlowNode);

	AddImpulseNode();
	virtual ~AddImpulseNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	TransformRefPin* transPin = NULL;
	Vector3fPin* impulsePin = NULL;
};

class AddImpulseAtLocationNode : public InOutFlowNode
{
public:
	Serialize(AddImpulseAtLocationNode, InOutFlowNode);

	AddImpulseAtLocationNode();
	virtual ~AddImpulseAtLocationNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	TransformRefPin* transPin = NULL;
	Vector3fPin* impulsePin = NULL;
	Vector3fPin* locationPin = NULL;
};

class RayCastNode : public InOutFlowNode
{
public:
	Serialize(RayCastNode, InOutFlowNode);

	RayCastNode();
	virtual ~RayCastNode() = default;

	virtual bool process(GraphContext & context);

	static Serializable* instantiate(const SerializationInfo & from);
protected:
	Vector3fPin* startPin = NULL;
	Vector3fPin* endPin = NULL;
	BoolPin* successPin = NULL;
	ContactInfoPin* contactPin = NULL;
};

class SphereSweepNode : public InOutFlowNode
{
public:
	Serialize(SphereSweepNode, InOutFlowNode);

	SphereSweepNode();
	virtual ~SphereSweepNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	FloatPin* radiusPin = NULL;
	Vector3fPin* startPin = NULL;
	Vector3fPin* endPin = NULL;
	BoolPin* successPin = NULL;
	ContactInfoPin* contactPin = NULL;
};