#pragma once

#include "../ObjectNode.h"
#include "../FlowNode.h"

class Transform;

DEC_REF_PIN(Transform, TransformRefPin);
DEC_REF_VAR_CLASS(Transform, TransformRefVariable, TransformRefPin);

class TransformGetWorldPositionNode : public GraphNode
{
public:
	Serialize(TransformGetWorldPositionNode, GraphNode);

	TransformGetWorldPositionNode();
	virtual ~TransformGetWorldPositionNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	TransformRefPin* transformPin = NULL;
	Vector3fPin* valuePin = NULL;
};

class TransformGetLocalPositionNode : public GraphNode
{
public:
	Serialize(TransformGetLocalPositionNode, GraphNode);

	TransformGetLocalPositionNode();
	virtual ~TransformGetLocalPositionNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	TransformRefPin* transformPin = NULL;
	Vector3fPin* valuePin = NULL;
};

class TransformGetWorldEularAngleNode : public GraphNode
{
public:
	Serialize(TransformGetWorldEularAngleNode, GraphNode);

	TransformGetWorldEularAngleNode();
	virtual ~TransformGetWorldEularAngleNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	TransformRefPin* transformPin = NULL;
	Vector3fPin* valuePin = NULL;
};

class TransformGetLocalEularAngleNode : public GraphNode
{
public:
	Serialize(TransformGetLocalEularAngleNode, GraphNode);

	TransformGetLocalEularAngleNode();
	virtual ~TransformGetLocalEularAngleNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	TransformRefPin* transformPin = NULL;
	Vector3fPin* valuePin = NULL;
};

class TransformGetWorldRotationNode : public GraphNode
{
public:
	Serialize(TransformGetWorldRotationNode, GraphNode);

	TransformGetWorldRotationNode();
	virtual ~TransformGetWorldRotationNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	TransformRefPin* transformPin = NULL;
	QuaternionfPin* valuePin = NULL;
};

class TransformGetLocalRotationNode : public GraphNode
{
public:
	Serialize(TransformGetLocalRotationNode, GraphNode);

	TransformGetLocalRotationNode();
	virtual ~TransformGetLocalRotationNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	TransformRefPin* transformPin = NULL;
	QuaternionfPin* valuePin = NULL;
};

class TransformGetForwardNode : public GraphNode
{
public:
	Serialize(TransformGetForwardNode, GraphNode);

	TransformGetForwardNode();
	virtual ~TransformGetForwardNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	TransformRefPin* transformPin = NULL;
	Vector3fPin* valuePin = NULL;
};

class TransformGetRightwardNode : public GraphNode
{
public:
	Serialize(TransformGetRightwardNode, GraphNode);

	TransformGetRightwardNode();
	virtual ~TransformGetRightwardNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	TransformRefPin* transformPin = NULL;
	Vector3fPin* valuePin = NULL;
};

class TransformGetUpwardNode : public GraphNode
{
public:
	Serialize(TransformGetUpwardNode, GraphNode);

	TransformGetUpwardNode();
	virtual ~TransformGetUpwardNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	TransformRefPin* transformPin = NULL;
	Vector3fPin* valuePin = NULL;
};

class TransformSetWorldPositionNode : public InOutFlowNode
{
public:
	Serialize(TransformSetWorldPositionNode, InOutFlowNode);

	TransformSetWorldPositionNode();
	virtual ~TransformSetWorldPositionNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	TransformRefPin* transformPin = NULL;
	Vector3fPin* valuePin = NULL;
};

class TransformSetLocalPositionNode : public InOutFlowNode
{
public:
	Serialize(TransformSetLocalPositionNode, InOutFlowNode);

	TransformSetLocalPositionNode();
	virtual ~TransformSetLocalPositionNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	TransformRefPin* transformPin = NULL;
	Vector3fPin* valuePin = NULL;
};

class TransformSetWorldEularAngleNode : public InOutFlowNode
{
public:
	Serialize(TransformSetWorldEularAngleNode, InOutFlowNode);

	TransformSetWorldEularAngleNode();
	virtual ~TransformSetWorldEularAngleNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	TransformRefPin* transformPin = NULL;
	Vector3fPin* valuePin = NULL;
};

class TransformSetLocalEularAngleNode : public InOutFlowNode
{
public:
	Serialize(TransformSetLocalEularAngleNode, InOutFlowNode);

	TransformSetLocalEularAngleNode();
	virtual ~TransformSetLocalEularAngleNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	TransformRefPin* transformPin = NULL;
	Vector3fPin* valuePin = NULL;
};

class TransformSetWorldRotationNode : public InOutFlowNode
{
public:
	Serialize(TransformSetWorldRotationNode, InOutFlowNode);

	TransformSetWorldRotationNode();
	virtual ~TransformSetWorldRotationNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	TransformRefPin* transformPin = NULL;
	QuaternionfPin* valuePin = NULL;
};

class TransformSetLocalRotationNode : public InOutFlowNode
{
public:
	Serialize(TransformSetLocalRotationNode, InOutFlowNode);

	TransformSetLocalRotationNode();
	virtual ~TransformSetLocalRotationNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	TransformRefPin* transformPin = NULL;
	QuaternionfPin* valuePin = NULL;
};

class TransformTranslateWorldNode : public InOutFlowNode
{
public:
	Serialize(TransformTranslateWorldNode, InOutFlowNode);

	TransformTranslateWorldNode();
	virtual ~TransformTranslateWorldNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	TransformRefPin* transformPin = NULL;
	Vector3fPin* valuePin = NULL;
};

class TransformTranslateLocalNode : public InOutFlowNode
{
public:
	Serialize(TransformTranslateLocalNode, InOutFlowNode);

	TransformTranslateLocalNode();
	virtual ~TransformTranslateLocalNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	TransformRefPin* transformPin = NULL;
	Vector3fPin* valuePin = NULL;
};

class TransformRotateEularWorldNode : public InOutFlowNode
{
public:
	Serialize(TransformRotateEularWorldNode, InOutFlowNode);

	TransformRotateEularWorldNode();
	virtual ~TransformRotateEularWorldNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	TransformRefPin* transformPin = NULL;
	Vector3fPin* valuePin = NULL;
};

class TransformRotateLocalEularNode : public InOutFlowNode
{
public:
	Serialize(TransformRotateLocalEularNode, InOutFlowNode);

	TransformRotateLocalEularNode();
	virtual ~TransformRotateLocalEularNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	TransformRefPin* transformPin = NULL;
	Vector3fPin* valuePin = NULL;
};

class TransformRotateNode : public InOutFlowNode
{
public:
	Serialize(TransformRotateNode, InOutFlowNode);

	TransformRotateNode();
	virtual ~TransformRotateNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	TransformRefPin* transformPin = NULL;
	QuaternionfPin* valuePin = NULL;
};
