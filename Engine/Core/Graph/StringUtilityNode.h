#pragma once

#include "FlowNode.h"

class ToStringNode : public GraphNode
{
public:
	Serialize(ToStringNode, GraphNode);

	ToStringNode();

	StringPin* getStringPin();

	virtual void toString();

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	StringPin* stringPin = NULL;
};

#define DEC_VALUE_TO_STRING_NODE(PinType, NodeType)					\
class NodeType : public ToStringNode								\
{																	\
public:																\
	Serialize(NodeType, ToStringNode);								\
																	\
	NodeType()														\
	{																\
		valuePin = new PinType("Value");							\
		addInput(*valuePin);										\
	}																\
																	\
	virtual Color getNodeColor() const								\
	{																\
		return valuePin->getPinColor();								\
	}																\
																	\
	PinType* getValuePin()											\
	{																\
		return valuePin;											\
	}																\
																	\
	virtual void toString()											\
	{																\
		stringPin->setValue(to_string(valuePin->getValue()));		\
	}																\
																	\
	static Serializable* instantiate(const SerializationInfo& from)	\
	{																\
		return new NodeType();										\
																	\
	}																\
protected:															\
	PinType* valuePin = NULL;										\
};

#define DEC_OBJECT_VALUE_TO_STRING_NODE(PinType, NodeType, StrFunc)	\
class NodeType : public ToStringNode								\
{																	\
public:																\
	Serialize(NodeType, ToStringNode);								\
																	\
	NodeType()														\
	{																\
		valuePin = new PinType("Value");							\
		addInput(*valuePin);										\
	}																\
																	\
	virtual Color getNodeColor() const								\
	{																\
		return valuePin->getPinColor();								\
	}																\
																	\
	PinType* getValuePin()											\
	{																\
		return valuePin;											\
	}																\
																	\
	virtual void toString()											\
	{																\
		stringPin->setValue(toStringInternal(valuePin));			\
	}																\
																	\
	static Serializable* instantiate(const SerializationInfo& from)	\
	{																\
		return new NodeType();										\
																	\
	}																\
protected:															\
	PinType* valuePin = NULL;										\
																	\
	string toStringInternal(PinType* pin)							\
	{																\
		StrFunc;													\
	}																\
};

#define IMP_VALUE_TO_STRING_NODE(NodeType) SerializeInstance(NodeType);

DEC_VALUE_TO_STRING_NODE(FloatPin, FloatToStringNode);
DEC_VALUE_TO_STRING_NODE(IntPin, IntToStringNode);
DEC_VALUE_TO_STRING_NODE(BoolPin, BoolToStringNode);

DEC_OBJECT_VALUE_TO_STRING_NODE(Vector2fPin, Vector2fToStringNode,
{
	Vector2f vec = pin->getValue();
	return "(" + to_string(vec.x()) + ", " + to_string(vec.y()) + ")";
});

DEC_OBJECT_VALUE_TO_STRING_NODE(Vector3fPin, Vector3fToStringNode,
{
	Vector3f vec = pin->getValue();
	return "(" + to_string(vec.x()) + ", " + to_string(vec.y()) + ", " + to_string(vec.z()) + ")";
});

DEC_OBJECT_VALUE_TO_STRING_NODE(QuaternionfPin, QuaternionfToStringNode,
{
	Quaternionf vec = pin->getValue();
	return "(" + to_string(vec.x()) + ", " + to_string(vec.y()) + ", " + to_string(vec.z()) + ", " + to_string(vec.w()) + ")";
});

class PrintNode : public InOutFlowNode
{
public:
	Serialize(PrintNode, InOutFlowNode);

	PrintNode();

	StringPin* getStringPin();

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	StringPin* stringPin = NULL;
};

class PrintWarningNode : public PrintNode
{
public:
	Serialize(PrintWarningNode, PrintNode);

	PrintWarningNode();

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
};

class PrintErrorNode : public PrintNode
{
public:
	Serialize(PrintErrorNode, PrintNode);

	PrintErrorNode();

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
};