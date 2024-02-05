#pragma once

#include "ValuePin.h"

class MathFunctionNode : public GraphNode
{
public:
	Serialize(MathFunctionNode, GraphNode);

	static Serializable* instantiate(const SerializationInfo & from);
};

class MathOperationNode : public MathFunctionNode
{
public:
	Serialize(MathOperationNode, MathFunctionNode);

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

class ComparisonNode : public MathFunctionNode
{
public:
	Serialize(ComparisonNode, MathFunctionNode);

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

class NotNode : public MathFunctionNode
{
public:
	Serialize(NotNode, MathFunctionNode);

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

class SteppingNode : public MathFunctionNode
{
public:
	Serialize(SteppingNode, MathFunctionNode);

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

class Vector2fMakeNode : public MathFunctionNode
{
public:
	Serialize(Vector2fMakeNode, MathFunctionNode);

	Vector2fMakeNode();
	virtual ~Vector2fMakeNode() = default;

	virtual bool process(GraphContext & context);

	static Serializable* instantiate(const SerializationInfo & from);
protected:
	FloatPin* inputPin[2];
	Vector2fPin* outputPin = NULL;
};

class Vector3fMakeNode : public MathFunctionNode
{
public:
	Serialize(Vector3fMakeNode, MathFunctionNode);

	Vector3fMakeNode();
	virtual ~Vector3fMakeNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	FloatPin* inputPin[3];
	Vector3fPin* outputPin = NULL;
};

class QuaternionfMakeNode : public MathFunctionNode
{
public:
	Serialize(QuaternionfMakeNode, MathFunctionNode);

	QuaternionfMakeNode();
	virtual ~QuaternionfMakeNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	FloatPin* inputPin[4];
	QuaternionfPin* outputPin = NULL;
};

class Vector2fBreakNode : public MathFunctionNode
{
public:
	Serialize(Vector2fBreakNode, MathFunctionNode);

	Vector2fBreakNode();
	virtual ~Vector2fBreakNode() = default;

	virtual bool process(GraphContext& context);

	virtual bool generate(GraphCodeGenerationContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector2fPin* inputPin = NULL;
	FloatPin* outputPin[2];
};

class Vector3fBreakNode : public MathFunctionNode
{
public:
	Serialize(Vector3fBreakNode, MathFunctionNode);

	Vector3fBreakNode();
	virtual ~Vector3fBreakNode() = default;

	virtual bool process(GraphContext& context);

	virtual bool generate(GraphCodeGenerationContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector3fPin* inputPin = NULL;
	FloatPin* outputPin[3];
};

class QuaternionfBreakNode : public MathFunctionNode
{
public:
	Serialize(QuaternionfBreakNode, MathFunctionNode);

	QuaternionfBreakNode();
	virtual ~QuaternionfBreakNode() = default;

	virtual bool process(GraphContext& context);

	virtual bool generate(GraphCodeGenerationContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	QuaternionfPin* inputPin = NULL;
	FloatPin* outputPin[4];
};

class Vector2fNormalizeNode : public MathFunctionNode
{
public:
	Serialize(Vector2fNormalizeNode, MathFunctionNode);

	Vector2fNormalizeNode();
	virtual ~Vector2fNormalizeNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector2fPin* inputPin = NULL;
	Vector2fPin* outputPin = NULL;
};

class Vector3fNormalizeNode : public MathFunctionNode
{
public:
	Serialize(Vector3fNormalizeNode, MathFunctionNode);

	Vector3fNormalizeNode();
	virtual ~Vector3fNormalizeNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector3fPin* inputPin = NULL;
	Vector3fPin* outputPin = NULL;
};

class QuaternionfNormalizeNode : public MathFunctionNode
{
public:
	Serialize(QuaternionfNormalizeNode, MathFunctionNode);

	QuaternionfNormalizeNode();
	virtual ~QuaternionfNormalizeNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	QuaternionfPin* inputPin = NULL;
	QuaternionfPin* outputPin = NULL;
};

class Vector2fScaleNode : public MathFunctionNode
{
public:
	Serialize(Vector2fScaleNode, MathFunctionNode);

	Vector2fScaleNode();
	virtual ~Vector2fScaleNode() = default;

	virtual bool process(GraphContext & context);

	static Serializable* instantiate(const SerializationInfo & from);
protected:
	Vector2fPin* vecPin = NULL;
	FloatPin* scalePin = NULL;
	Vector2fPin* outputPin = NULL;
};

class Vector3fScaleNode : public MathFunctionNode
{
public:
	Serialize(Vector3fScaleNode, MathFunctionNode);

	Vector3fScaleNode();
	virtual ~Vector3fScaleNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector3fPin* vecPin = NULL;
	FloatPin* scalePin = NULL;
	Vector3fPin* outputPin = NULL;
};

class Vector2fDotNode : public MathFunctionNode
{
public:
	Serialize(Vector2fDotNode, MathFunctionNode);

	Vector2fDotNode();
	virtual ~Vector2fDotNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector2fPin* aPin = NULL;
	Vector2fPin* bPin = NULL;
	FloatPin* outputPin = NULL;
};

class Vector3fDotNode : public MathFunctionNode
{
public:
	Serialize(Vector3fDotNode, MathFunctionNode);

	Vector3fDotNode();
	virtual ~Vector3fDotNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector3fPin* aPin = NULL;
	Vector3fPin* bPin = NULL;
	FloatPin* outputPin = NULL;
};

class Vector2fCrossNode : public MathFunctionNode
{
public:
	Serialize(Vector2fCrossNode, MathFunctionNode);

	Vector2fCrossNode();
	virtual ~Vector2fCrossNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector2fPin* aPin = NULL;
	Vector2fPin* bPin = NULL;
	Vector2fPin* outputPin = NULL;
};

class Vector3fCrossNode : public MathFunctionNode
{
public:
	Serialize(Vector3fCrossNode, MathFunctionNode);

	Vector3fCrossNode();
	virtual ~Vector3fCrossNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector3fPin* aPin = NULL;
	Vector3fPin* bPin = NULL;
	Vector3fPin* outputPin = NULL;
};

class QuaternionfFromTwoVectorsNode : public MathFunctionNode
{
public:
	Serialize(QuaternionfFromTwoVectorsNode, MathFunctionNode);

	QuaternionfFromTwoVectorsNode();
	virtual ~QuaternionfFromTwoVectorsNode() = default;

	virtual bool process(GraphContext & context);

	static Serializable* instantiate(const SerializationInfo & from);
protected:
	Vector3fPin* aPin = NULL;
	Vector3fPin* bPin = NULL;
	QuaternionfPin* outputPin = NULL;
};

class EularToQuaternionfNode : public MathFunctionNode
{
public:
	Serialize(EularToQuaternionfNode, MathFunctionNode);

	EularToQuaternionfNode();
	virtual ~EularToQuaternionfNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector3fPin* eularPin = NULL;
	QuaternionfPin* quatPin = NULL;
};

class QuaternionfToEularNode : public MathFunctionNode
{
public:
	Serialize(QuaternionfToEularNode, MathFunctionNode);

	QuaternionfToEularNode();
	virtual ~QuaternionfToEularNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	QuaternionfPin* quatPin = NULL;
	Vector3fPin* eularPin = NULL;
};

class QuaternionfFromAngleAxisNode : public MathFunctionNode
{
public:
	Serialize(QuaternionfFromAngleAxisNode, MathFunctionNode);

	QuaternionfFromAngleAxisNode();
	virtual ~QuaternionfFromAngleAxisNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	FloatPin* anglePin = NULL;
	Vector3fPin* axisPin = NULL;
	QuaternionfPin* quatPin = NULL;
};

class QuaternionfSlerpNode : public MathFunctionNode
{
public:
	Serialize(QuaternionfSlerpNode, MathFunctionNode);

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

class Vector2fProjectOnVectorNode : public MathFunctionNode
{
public:
	Serialize(Vector2fProjectOnVectorNode, MathFunctionNode);

	Vector2fProjectOnVectorNode();
	virtual ~Vector2fProjectOnVectorNode() = default;

	virtual bool process(GraphContext & context);

	static Serializable* instantiate(const SerializationInfo & from);
protected:
	Vector2fPin* vecPin = NULL;
	Vector2fPin* normPin = NULL;
	Vector2fPin* outputPin = NULL;
};

class Vector3fProjectOnVectorNode : public MathFunctionNode
{
public:
	Serialize(Vector3fProjectOnVectorNode, MathFunctionNode);

	Vector3fProjectOnVectorNode();
	virtual ~Vector3fProjectOnVectorNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector3fPin* vecPin = NULL;
	Vector3fPin* normPin = NULL;
	Vector3fPin* outputPin = NULL;
};

class Vector3fSteppingNode : public MathFunctionNode
{
public:
	Serialize(Vector3fSteppingNode, MathFunctionNode);

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

class Vector3fSteppingRotationNode : public MathFunctionNode
{
public:
	Serialize(Vector3fSteppingRotationNode, MathFunctionNode);

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

class AngleToRadianNode : public MathFunctionNode
{
public:
	Serialize(AngleToRadianNode, MathFunctionNode);

	AngleToRadianNode();
	virtual ~AngleToRadianNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	FloatPin* anglePin = NULL;
	FloatPin* radianPin = NULL;
};

class RadianToAngleNode : public MathFunctionNode
{
public:
	Serialize(RadianToAngleNode, MathFunctionNode);

	RadianToAngleNode();
	virtual ~RadianToAngleNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	FloatPin* radianPin = NULL;
	FloatPin* anglePin = NULL;
};

class GetAngleFromTwoVector2fNode : public MathFunctionNode
{
public:
	Serialize(GetAngleFromTwoVector2fNode, MathFunctionNode);

	GetAngleFromTwoVector2fNode();
	virtual ~GetAngleFromTwoVector2fNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	Vector2fPin* aPin = NULL;
	Vector2fPin* bPin = NULL;
	FloatPin* anglePin = NULL;
};

class GetAngleFromTwoVector3fNode : public MathFunctionNode
{
public:
	Serialize(GetAngleFromTwoVector3fNode, MathFunctionNode);

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

class GetAngleFromToVector3fNode : public MathFunctionNode
{
public:
	Serialize(GetAngleFromToVector3fNode, MathFunctionNode);

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
