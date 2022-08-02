#pragma once

#include "MathNode.h"
#include "FlowNode.h"

class GetMousePositionNode : public GraphNode
{
public:
	Serialize(GetMousePositionNode, GraphNode);

	GetMousePositionNode();
	virtual ~GetMousePositionNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector2fPin* valuePin = NULL;
};

class GetMouseMoveNode : public GraphNode
{
public:
	Serialize(GetMouseMoveNode, GraphNode);

	GetMouseMoveNode();
	virtual ~GetMouseMoveNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector2fPin* valuePin = NULL;
};

class GetMouseDownNode : public GraphNode
{
public:
	Serialize(GetMouseDownNode, GraphNode);

	GetMouseDownNode();
	virtual ~GetMouseDownNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	IntPin* indexPin = NULL;
	BoolPin* valuePin = NULL;
};

class GetMouseUpNode : public GraphNode
{
public:
	Serialize(GetMouseUpNode, GraphNode);

	GetMouseUpNode();
	virtual ~GetMouseUpNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	IntPin* indexPin = NULL;
	BoolPin* valuePin = NULL;
};

class GetMousePressNode : public GraphNode
{
public:
	Serialize(GetMousePressNode, GraphNode);

	GetMousePressNode();
	virtual ~GetMousePressNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	IntPin* indexPin = NULL;
	BoolPin* valuePin = NULL;
};

class GetMouseReleaseNode : public GraphNode
{
public:
	Serialize(GetMouseReleaseNode, GraphNode);

	GetMouseReleaseNode();
	virtual ~GetMouseReleaseNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	IntPin* indexPin = NULL;
	BoolPin* valuePin = NULL;
};

class GetKeyDownNode : public GraphNode
{
public:
	Serialize(GetKeyDownNode, GraphNode);

	GetKeyDownNode();
	virtual ~GetKeyDownNode() = default;

	virtual bool process(GraphContext & context);

	static Serializable* instantiate(const SerializationInfo & from);
protected:
	KeyCodePin* keyCodePin = NULL;
	BoolPin* valuePin = NULL;
};

class GetKeyUpNode : public GraphNode
{
public:
	Serialize(GetKeyUpNode, GraphNode);

	GetKeyUpNode();
	virtual ~GetKeyUpNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	KeyCodePin* keyCodePin = NULL;
	BoolPin* valuePin = NULL;
};

class GetKeyPressNode : public GraphNode
{
public:
	Serialize(GetKeyPressNode, GraphNode);

	GetKeyPressNode();
	virtual ~GetKeyPressNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	KeyCodePin* keyCodePin = NULL;
	BoolPin* valuePin = NULL;
};

class GetKeyReleaseNode : public GraphNode
{
public:
	Serialize(GetKeyReleaseNode, GraphNode);

	GetKeyReleaseNode();
	virtual ~GetKeyReleaseNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	KeyCodePin* keyCodePin = NULL;
	BoolPin* valuePin = NULL;
};