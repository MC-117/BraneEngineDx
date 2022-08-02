#pragma once

#include "ActorNode.h"

class Character;

DEC_REF_PIN(Character, CharacterRefPin);
DEC_REF_VAR_CLASS(Character, CharacterRefVariable, CharacterRefPin);

class CharacterIsFlyNode : public GraphNode
{
public:
	Serialize(CharacterIsFlyNode, GraphNode);

	CharacterIsFlyNode();
	virtual ~CharacterIsFlyNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	CharacterRefPin* characterPin = NULL;
	BoolPin* isFlyPin = NULL;
};

class CharacterMoveNode : public InOutFlowNode
{
public:
	Serialize(CharacterMoveNode, InOutFlowNode);

	CharacterMoveNode();
	virtual ~CharacterMoveNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	CharacterRefPin* characterPin = NULL;
	Vector3fPin* movePin = NULL;
};

class CharacterJumpNode : public InOutFlowNode
{
public:
	Serialize(CharacterJumpNode, InOutFlowNode);

	CharacterJumpNode();
	virtual ~CharacterJumpNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	CharacterRefPin* characterPin = NULL;
	FloatPin* impulsePin = NULL;
};