#pragma once

#include "ValuePin.h"

class MathOperationNode : public GraphNode
{
public:
	Serialize(MathOperationNode, GraphNode);

	MathOperationNode();
	virtual ~MathOperationNode();

	ValuePin* getOutputValuePin();

	void initOutputPin(ValuePin* valuePin);
	virtual void addInputPin();

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo & from);
protected:
	ValuePin* outputPin = NULL;
};

#define DEC_MATH_OP_NODE(PinType, NodeType, Op, OpName) \
class NodeType : public MathOperationNode \
{ \
public: \
	Serialize(NodeType, MathOperationNode); \
 \
	NodeType() \
	{ \
		displayName = #Op; \
		initOutputPin(new PinType("output")); \
		addInputPin(); \
		addInputPin(); \
	} \
 \
	virtual bool process(GraphContext& context) \
	{ \
		if (outputPin == NULL) \
			return false; \
		PinType* output = (PinType*)outputPin; \
		output->setValue(((PinType*)inputs[0])->getValue()); \
		for (int i = 1; i < inputs.size(); i++) \
		{ \
			PinType* input = (PinType*)inputs[i]; \
			output->setValue(output->getValue() Op input->getValue()); \
		} \
		return true; \
	} \
 \
	virtual Name getFunctionName() const \
	{ \
		static const Name funcName = OpName; \
		return funcName; \
	} \
 \
	static Serializable* instantiate(const SerializationInfo& from) \
	{ \
		NodeType* node = new NodeType(); \
		const SerializationInfo* inputInfo = from.get("inputs"); \
		if (inputInfo) \
			for (int i = 2; i < inputInfo->sublists.size(); i++) \
				node->addInputPin(); \
		return node; \
	} \
};

#define IMP_MATH_OP_NODE(NodeType) SerializeInstance(NodeType);

DEC_MATH_OP_NODE(FloatPin, FloatAddtionNode, +, Code::add_op);
DEC_MATH_OP_NODE(IntPin, IntAddtionNode, +, Code::add_op);
DEC_MATH_OP_NODE(Vector2fPin, Vector2fAddtionNode, +, Code::add_op);
DEC_MATH_OP_NODE(Vector3fPin, Vector3fAddtionNode, +, Code::add_op);
DEC_MATH_OP_NODE(StringPin, StringAddtionNode, +, Code::add_op);

DEC_MATH_OP_NODE(FloatPin, FloatSubtractionNode, -, Code::sub_op);
DEC_MATH_OP_NODE(IntPin, IntSubtractionNode, -, Code::sub_op);
DEC_MATH_OP_NODE(Vector2fPin, Vector2fSubtractionNode, -, Code::sub_op);
DEC_MATH_OP_NODE(Vector3fPin, Vector3fSubtractionNode, -, Code::sub_op);

DEC_MATH_OP_NODE(FloatPin, FloatMultiplyNode, *, Code::mul_op);
DEC_MATH_OP_NODE(IntPin, IntMultiplyNode, *, Code::mul_op);

DEC_MATH_OP_NODE(FloatPin, FloatDivisionNode, /, Code::div_op);
DEC_MATH_OP_NODE(IntPin, IntDivisionNode, /, Code::div_op);

DEC_MATH_OP_NODE(BoolPin, BoolAndNode, &, Code::and_op);
DEC_MATH_OP_NODE(BoolPin, BoolOrNode, |, Code::or_op);

class ComparisonNode : public GraphNode
{
public:
	Serialize(ComparisonNode, GraphNode);

	ComparisonNode();
	virtual ~ComparisonNode() = default;

	BoolPin* getOutputPin();

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	BoolPin* outputPin = NULL;
};

#define DEC_MATH_COMPARE_NODE(PinType, NodeType, Op, OpName) \
class NodeType : public ComparisonNode \
{ \
public: \
	Serialize(NodeType, ComparisonNode); \
 \
	NodeType() : ComparisonNode() \
	{ \
		displayName = #Op; \
		aPin = new PinType("A"); \
		bPin = new PinType("B"); \
		addInput(*aPin); \
		addInput(*bPin); \
	} \
 \
	virtual bool process(GraphContext& context) \
	{ \
		if (outputPin == NULL) \
			return false; \
		outputPin->setValue(aPin->getValue() Op bPin->getValue()); \
		return true; \
	} \
\
	virtual Name getFunctionName() const \
	{ \
		static const Name funcName = OpName; \
		return funcName; \
	} \
\
	static Serializable* instantiate(const SerializationInfo& from) \
	{ \
		return new NodeType(); \
	} \
protected: \
	PinType* aPin = NULL; \
	PinType* bPin = NULL; \
};

#define IMP_MATH_COMPARE_NODE(NodeType) SerializeInstance(NodeType);

DEC_MATH_COMPARE_NODE(FloatPin, FloatLessNode, <, Code::les_op);
DEC_MATH_COMPARE_NODE(IntPin, IntLessNode, <, Code::les_op);
DEC_MATH_COMPARE_NODE(StringPin, StringLessNode, <, Code::les_op);

DEC_MATH_COMPARE_NODE(FloatPin, FloatLessEqualNode, <=, Code::lesEq_op);
DEC_MATH_COMPARE_NODE(IntPin, IntLessEqualNode, <=, Code::lesEq_op);
DEC_MATH_COMPARE_NODE(StringPin, StringLessEqualNode, <=, Code::lesEq_op);

DEC_MATH_COMPARE_NODE(FloatPin, FloatGreaterNode, >, Code::gre_op);
DEC_MATH_COMPARE_NODE(IntPin, IntGreaterNode, >, Code::gre_op);
DEC_MATH_COMPARE_NODE(StringPin, StringGreaterNode, >, Code::gre_op);

DEC_MATH_COMPARE_NODE(FloatPin, FloatGreaterEqualNode, >=, Code::greEq_op);
DEC_MATH_COMPARE_NODE(IntPin, IntGreaterEqualNode, >=, Code::greEq_op);
DEC_MATH_COMPARE_NODE(StringPin, StringGreaterEqualNode, >=, Code::greEq_op);

DEC_MATH_COMPARE_NODE(FloatPin, FloatEqualNode, ==, Code::eq_op);
DEC_MATH_COMPARE_NODE(IntPin, IntEqualNode, ==, Code::eq_op);
DEC_MATH_COMPARE_NODE(BoolPin, BoolEqualNode, ==, Code::eq_op);
DEC_MATH_COMPARE_NODE(StringPin, StringEqualNode, ==, Code::eq_op);

DEC_MATH_COMPARE_NODE(Vector2fPin, Vector2fEqualNode, ==, Code::eq_op);
DEC_MATH_COMPARE_NODE(Vector3fPin, Vector3fEqualNode, ==, Code::eq_op);
DEC_MATH_COMPARE_NODE(QuaternionfPin, QuaternionfEqualNode, ==, Code::eq_op);

DEC_MATH_COMPARE_NODE(FloatPin, FloatNotEqualNode, !=, Code::notEq_op);
DEC_MATH_COMPARE_NODE(IntPin, IntNotEqualNode, !=, Code::notEq_op);
DEC_MATH_COMPARE_NODE(BoolPin, BoolNotEqualNode, !=, Code::notEq_op);
DEC_MATH_COMPARE_NODE(StringPin, StringNotEqualNode, !=, Code::notEq_op);

DEC_MATH_COMPARE_NODE(Vector2fPin, Vector2fNotEqualNode, !=, Code::notEq_op);
DEC_MATH_COMPARE_NODE(Vector3fPin, Vector3fNotEqualNode, !=, Code::notEq_op);
DEC_MATH_COMPARE_NODE(QuaternionfPin, QuaternionfNotEqualNode, !=, Code::notEq_op);

class NotNode : public GraphNode
{
public:
	Serialize(NotNode, GraphNode);

	NotNode();
	virtual ~NotNode() = default;

	BoolPin* getInputPin();
	BoolPin* getOutputPin();

	virtual bool process(GraphContext& context);

	virtual Name getFunctionName() const;

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	BoolPin* inputPin = NULL;
	BoolPin* outputPin = NULL;
};

class SteppingNode : public GraphNode
{
public:
	Serialize(SteppingNode, GraphNode);

	SteppingNode();
	virtual ~SteppingNode() = default;

	virtual bool process(GraphContext & context);

	static Serializable* instantiate(const SerializationInfo & from);
protected:
	FloatPin* srcPin = NULL;
	FloatPin* tarPin = NULL;
	FloatPin* stepPin = NULL;
	FloatPin* outputPin = NULL;
};

class Vector2fMakeNode : public GraphNode
{
public:
	Serialize(Vector2fMakeNode, GraphNode);

	Vector2fMakeNode();
	virtual ~Vector2fMakeNode() = default;

	virtual bool process(GraphContext & context);

	static Serializable* instantiate(const SerializationInfo & from);
protected:
	FloatPin* inputPin[2];
	Vector2fPin* outputPin = NULL;
};

class Vector3fMakeNode : public GraphNode
{
public:
	Serialize(Vector3fMakeNode, GraphNode);

	Vector3fMakeNode();
	virtual ~Vector3fMakeNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	FloatPin* inputPin[3];
	Vector3fPin* outputPin = NULL;
};

class QuaternionfMakeNode : public GraphNode
{
public:
	Serialize(QuaternionfMakeNode, GraphNode);

	QuaternionfMakeNode();
	virtual ~QuaternionfMakeNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	FloatPin* inputPin[4];
	QuaternionfPin* outputPin = NULL;
};

class Vector2fBreakNode : public GraphNode
{
public:
	Serialize(Vector2fBreakNode, GraphNode);

	Vector2fBreakNode();
	virtual ~Vector2fBreakNode() = default;

	virtual bool process(GraphContext& context);

	virtual bool generate(GraphCodeGenerationContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector2fPin* inputPin = NULL;
	FloatPin* outputPin[2];
};

class Vector3fBreakNode : public GraphNode
{
public:
	Serialize(Vector3fBreakNode, GraphNode);

	Vector3fBreakNode();
	virtual ~Vector3fBreakNode() = default;

	virtual bool process(GraphContext& context);

	virtual bool generate(GraphCodeGenerationContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector3fPin* inputPin = NULL;
	FloatPin* outputPin[3];
};

class QuaternionfBreakNode : public GraphNode
{
public:
	Serialize(QuaternionfBreakNode, GraphNode);

	QuaternionfBreakNode();
	virtual ~QuaternionfBreakNode() = default;

	virtual bool process(GraphContext& context);

	virtual bool generate(GraphCodeGenerationContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	QuaternionfPin* inputPin = NULL;
	FloatPin* outputPin[4];
};

class Vector2fNormalizeNode : public GraphNode
{
public:
	Serialize(Vector2fNormalizeNode, GraphNode);

	Vector2fNormalizeNode();
	virtual ~Vector2fNormalizeNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector2fPin* inputPin = NULL;
	Vector2fPin* outputPin = NULL;
};

class Vector3fNormalizeNode : public GraphNode
{
public:
	Serialize(Vector3fNormalizeNode, GraphNode);

	Vector3fNormalizeNode();
	virtual ~Vector3fNormalizeNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector3fPin* inputPin = NULL;
	Vector3fPin* outputPin = NULL;
};

class QuaternionfNormalizeNode : public GraphNode
{
public:
	Serialize(QuaternionfNormalizeNode, GraphNode);

	QuaternionfNormalizeNode();
	virtual ~QuaternionfNormalizeNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	QuaternionfPin* inputPin = NULL;
	QuaternionfPin* outputPin = NULL;
};

class Vector2fScaleNode : public GraphNode
{
public:
	Serialize(Vector2fScaleNode, GraphNode);

	Vector2fScaleNode();
	virtual ~Vector2fScaleNode() = default;

	virtual bool process(GraphContext & context);

	static Serializable* instantiate(const SerializationInfo & from);
protected:
	Vector2fPin* vecPin = NULL;
	FloatPin* scalePin = NULL;
	Vector2fPin* outputPin = NULL;
};

class Vector3fScaleNode : public GraphNode
{
public:
	Serialize(Vector3fScaleNode, GraphNode);

	Vector3fScaleNode();
	virtual ~Vector3fScaleNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector3fPin* vecPin = NULL;
	FloatPin* scalePin = NULL;
	Vector3fPin* outputPin = NULL;
};

class Vector2fDotNode : public GraphNode
{
public:
	Serialize(Vector2fDotNode, GraphNode);

	Vector2fDotNode();
	virtual ~Vector2fDotNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector2fPin* aPin = NULL;
	Vector2fPin* bPin = NULL;
	FloatPin* outputPin = NULL;
};

class Vector3fDotNode : public GraphNode
{
public:
	Serialize(Vector3fDotNode, GraphNode);

	Vector3fDotNode();
	virtual ~Vector3fDotNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector3fPin* aPin = NULL;
	Vector3fPin* bPin = NULL;
	FloatPin* outputPin = NULL;
};

class Vector2fCrossNode : public GraphNode
{
public:
	Serialize(Vector2fCrossNode, GraphNode);

	Vector2fCrossNode();
	virtual ~Vector2fCrossNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector2fPin* aPin = NULL;
	Vector2fPin* bPin = NULL;
	Vector2fPin* outputPin = NULL;
};

class Vector3fCrossNode : public GraphNode
{
public:
	Serialize(Vector3fCrossNode, GraphNode);

	Vector3fCrossNode();
	virtual ~Vector3fCrossNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector3fPin* aPin = NULL;
	Vector3fPin* bPin = NULL;
	Vector3fPin* outputPin = NULL;
};

class QuaternionfFromTwoVectorsNode : public GraphNode
{
public:
	Serialize(QuaternionfFromTwoVectorsNode, GraphNode);

	QuaternionfFromTwoVectorsNode();
	virtual ~QuaternionfFromTwoVectorsNode() = default;

	virtual bool process(GraphContext & context);

	static Serializable* instantiate(const SerializationInfo & from);
protected:
	Vector3fPin* aPin = NULL;
	Vector3fPin* bPin = NULL;
	QuaternionfPin* outputPin = NULL;
};

class EularToQuaternionfNode : public GraphNode
{
public:
	Serialize(EularToQuaternionfNode, GraphNode);

	EularToQuaternionfNode();
	virtual ~EularToQuaternionfNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector3fPin* eularPin = NULL;
	QuaternionfPin* quatPin = NULL;
};

class QuaternionfToEularNode : public GraphNode
{
public:
	Serialize(QuaternionfToEularNode, GraphNode);

	QuaternionfToEularNode();
	virtual ~QuaternionfToEularNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	QuaternionfPin* quatPin = NULL;
	Vector3fPin* eularPin = NULL;
};

class QuaternionfFromAngleAxisNode : public GraphNode
{
public:
	Serialize(QuaternionfFromAngleAxisNode, GraphNode);

	QuaternionfFromAngleAxisNode();
	virtual ~QuaternionfFromAngleAxisNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	FloatPin* anglePin = NULL;
	Vector3fPin* axisPin = NULL;
	QuaternionfPin* quatPin = NULL;
};

class QuaternionfSlerpNode : public GraphNode
{
public:
	Serialize(QuaternionfSlerpNode, GraphNode);

	QuaternionfSlerpNode();
	virtual ~QuaternionfSlerpNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	QuaternionfPin* aPin = NULL;
	QuaternionfPin* bPin = NULL;
	FloatPin* weightPin = NULL;
	QuaternionfPin* outputPin = NULL;
};

class Vector2fProjectOnVectorNode : public GraphNode
{
public:
	Serialize(Vector2fProjectOnVectorNode, GraphNode);

	Vector2fProjectOnVectorNode();
	virtual ~Vector2fProjectOnVectorNode() = default;

	virtual bool process(GraphContext & context);

	static Serializable* instantiate(const SerializationInfo & from);
protected:
	Vector2fPin* vecPin = NULL;
	Vector2fPin* normPin = NULL;
	Vector2fPin* outputPin = NULL;
};

class Vector3fProjectOnVectorNode : public GraphNode
{
public:
	Serialize(Vector3fProjectOnVectorNode, GraphNode);

	Vector3fProjectOnVectorNode();
	virtual ~Vector3fProjectOnVectorNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector3fPin* vecPin = NULL;
	Vector3fPin* normPin = NULL;
	Vector3fPin* outputPin = NULL;
};

class Vector3fSteppingNode : public GraphNode
{
public:
	Serialize(Vector3fSteppingNode, GraphNode);

	Vector3fSteppingNode();
	virtual ~Vector3fSteppingNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector3fPin* aPin = NULL;
	Vector3fPin* bPin = NULL;
	FloatPin* anglePin = NULL;
	Vector3fPin* outputPin = NULL;
};

class Vector3fSteppingRotationNode : public GraphNode
{
public:
	Serialize(Vector3fSteppingRotationNode, GraphNode);

	Vector3fSteppingRotationNode();
	virtual ~Vector3fSteppingRotationNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector3fPin* aPin = NULL;
	Vector3fPin* bPin = NULL;
	FloatPin* anglePin = NULL;
	QuaternionfPin* outputPin = NULL;
};

class AngleToRadianNode : public GraphNode
{
public:
	Serialize(AngleToRadianNode, GraphNode);

	AngleToRadianNode();
	virtual ~AngleToRadianNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	FloatPin* anglePin = NULL;
	FloatPin* radianPin = NULL;
};

class RadianToAngleNode : public GraphNode
{
public:
	Serialize(RadianToAngleNode, GraphNode);

	RadianToAngleNode();
	virtual ~RadianToAngleNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	FloatPin* radianPin = NULL;
	FloatPin* anglePin = NULL;
};

class GetAngleFromTwoVector2fNode : public GraphNode
{
public:
	Serialize(GetAngleFromTwoVector2fNode, GraphNode);

	GetAngleFromTwoVector2fNode();
	virtual ~GetAngleFromTwoVector2fNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector2fPin* aPin = NULL;
	Vector2fPin* bPin = NULL;
	FloatPin* anglePin = NULL;
};

class GetAngleFromTwoVector3fNode : public GraphNode
{
public:
	Serialize(GetAngleFromTwoVector3fNode, GraphNode);

	GetAngleFromTwoVector3fNode();
	virtual ~GetAngleFromTwoVector3fNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector3fPin* aPin = NULL;
	Vector3fPin* bPin = NULL;
	Vector3fPin* refAxisPin = NULL;
	FloatPin* anglePin = NULL;
};

class GetAngleFromToVector3fNode : public GraphNode
{
public:
	Serialize(GetAngleFromToVector3fNode, GraphNode);

	GetAngleFromToVector3fNode();
	virtual ~GetAngleFromToVector3fNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector3fPin* aPin = NULL;
	Vector3fPin* bPin = NULL;
	Vector3fPin* refAxisPin = NULL;
	FloatPin* anglePin = NULL;
};
