#include "MathNode.h"

#include "GraphCodeGeneration.h"

SerializeInstance(MathOperationNode, DEF_ATTR(Namespace, "Math"));

MathOperationNode::MathOperationNode()
{
    flag = Flag::Expression;
}

MathOperationNode::~MathOperationNode()
{
}

ValuePin* MathOperationNode::getOutputValuePin()
{
    return outputPin;
}

void MathOperationNode::initOutputPin(ValuePin* valuePin)
{
    if (valuePin == NULL || outputPin != NULL)
        return;
    this->outputPin = valuePin;
    nodeColor = valuePin->getPinColor();
    valuePin->setName("output");
    valuePin->setDisplayName("");
    addOutput(*valuePin);
}

void MathOperationNode::addInputPin()
{
    if (outputPin == NULL)
        return;
    Serializable* serializable = outputPin->getSerialization().instantiate(SerializationInfo());
    if (serializable == NULL)
        return;
    ValuePin* valuePin = dynamic_cast<ValuePin*>(serializable);
    if (valuePin == NULL) {
        delete serializable;
        return;
    }
    valuePin->setName("input" + to_string(inputs.size()));
    valuePin->setDisplayName("");
    addInput(*valuePin);
}

bool MathOperationNode::process(GraphContext& context)
{
    return false;
}

Serializable* MathOperationNode::instantiate(const SerializationInfo& from)
{
    const SerializationInfo* pinInfos = from.get("outputs");
    if (pinInfos == NULL)
        return NULL;
    const SerializationInfo* pinInfo = pinInfos->get("output");
    if (!pinInfo->serialization->isChildOf(ValuePin::ValuePinSerialization::serialization))
        return NULL;
    Serializable* serializable = pinInfo->serialization->instantiate(from);
    if (serializable == NULL)
        return NULL;
    ValuePin* pin = (ValuePin*)serializable;
    MathOperationNode* node = new MathOperationNode();
    node->initOutputPin(pin);

    const SerializationInfo* inputInfo = from.get("inputs");
    for (int i = 0; i < inputInfo->sublists.size(); i++) {
        node->addInputPin();
    }
    return node;
}

IMP_MATH_OP_NODE(FloatAddtionNode);
IMP_MATH_OP_NODE(IntAddtionNode);
IMP_MATH_OP_NODE(Vector2fAddtionNode);
IMP_MATH_OP_NODE(Vector3fAddtionNode);
IMP_MATH_OP_NODE(StringAddtionNode);

IMP_MATH_OP_NODE(FloatSubtractionNode);
IMP_MATH_OP_NODE(IntSubtractionNode);
IMP_MATH_OP_NODE(Vector2fSubtractionNode);
IMP_MATH_OP_NODE(Vector3fSubtractionNode);

IMP_MATH_OP_NODE(FloatMultiplyNode);
IMP_MATH_OP_NODE(IntMultiplyNode);

IMP_MATH_OP_NODE(FloatDivisionNode);
IMP_MATH_OP_NODE(IntDivisionNode);

IMP_MATH_OP_NODE(BoolAndNode);
IMP_MATH_OP_NODE(BoolOrNode);

IMP_MATH_COMPARE_NODE(FloatLessNode);
IMP_MATH_COMPARE_NODE(IntLessNode);
IMP_MATH_COMPARE_NODE(StringLessNode);

IMP_MATH_COMPARE_NODE(FloatLessEqualNode);
IMP_MATH_COMPARE_NODE(IntLessEqualNode);
IMP_MATH_COMPARE_NODE(StringLessEqualNode);

IMP_MATH_COMPARE_NODE(FloatGreaterNode);
IMP_MATH_COMPARE_NODE(IntGreaterNode);
IMP_MATH_COMPARE_NODE(StringGreaterNode);

IMP_MATH_COMPARE_NODE(FloatGreaterEqualNode);
IMP_MATH_COMPARE_NODE(IntGreaterEqualNode);
IMP_MATH_COMPARE_NODE(StringGreaterEqualNode);

IMP_MATH_COMPARE_NODE(FloatEqualNode);
IMP_MATH_COMPARE_NODE(IntEqualNode);
IMP_MATH_COMPARE_NODE(StringEqualNode);

IMP_MATH_COMPARE_NODE(Vector2fEqualNode);
IMP_MATH_COMPARE_NODE(Vector3fEqualNode);
IMP_MATH_COMPARE_NODE(QuaternionfEqualNode);

IMP_MATH_COMPARE_NODE(FloatNotEqualNode);
IMP_MATH_COMPARE_NODE(IntNotEqualNode);
IMP_MATH_COMPARE_NODE(StringNotEqualNode);

IMP_MATH_COMPARE_NODE(Vector2fNotEqualNode);
IMP_MATH_COMPARE_NODE(Vector3fNotEqualNode);
IMP_MATH_COMPARE_NODE(QuaternionfNotEqualNode);

SerializeInstance(ComparisonNode);

ComparisonNode::ComparisonNode()
{
    flag = Flag::Expression;
    outputPin = new BoolPin("Out");
    addOutput(*outputPin);
}

BoolPin* ComparisonNode::getOutputPin()
{
    return outputPin;
}

Serializable* ComparisonNode::instantiate(const SerializationInfo& from)
{
    return nullptr;
}

SerializeInstance(NotNode);

NotNode::NotNode()
{
    flag = Flag::Expression;
    displayName = "Not";
    inputPin = new BoolPin("In");
    outputPin = new BoolPin("Out");
    addInput(*inputPin);
    addOutput(*outputPin);
}

BoolPin* NotNode::getInputPin()
{
    return inputPin;
}

BoolPin* NotNode::getOutputPin()
{
    return outputPin;
}

bool NotNode::process(GraphContext& context)
{
    if (outputPin == NULL)
        return false;
    outputPin->setValue(!inputPin->getValue());
    return true;
}

Name NotNode::getFunctionName() const
{
    return Code::not_op;
}

Serializable* NotNode::instantiate(const SerializationInfo& from)
{
    return new NotNode();
}

SerializeInstance(SteppingNode);

SteppingNode::SteppingNode()
{
    flag = Flag::Expression;
    displayName = "Stepping";
    srcPin = new FloatPin("Src");
    tarPin = new FloatPin("Tar");
    stepPin = new FloatPin("Step");
    outputPin = new FloatPin("Out");
    addInput(*srcPin);
    addInput(*tarPin);
    addInput(*stepPin);
    addOutput(*outputPin);
}

bool SteppingNode::process(GraphContext& context)
{
    float src = srcPin->getValue();
    float tar = tarPin->getValue();
    float step = abs(stepPin->getValue());
    float diff = tar - src;
    float dis = abs(diff);
    float dir = diff / dis;
    outputPin->setValue(dis > step ? (src + step * dir) : tar);
    return true;
}

Serializable* SteppingNode::instantiate(const SerializationInfo& from)
{
    return new SteppingNode();
}

SerializeInstance(Vector2fMakeNode, DEF_ATTR(CodeFunctionName, Code::Vector2f_t));

Vector2fMakeNode::Vector2fMakeNode()
{
    flag = Flag::Expression;
    displayName = "MakeVector2f";
    inputPin[0] = new FloatPin("x");
    inputPin[1] = new FloatPin("y");
    outputPin = new Vector2fPin("Out");
    addInput(*inputPin[0]);
    addInput(*inputPin[1]);
    addOutput(*outputPin);
}

bool Vector2fMakeNode::process(GraphContext& context)
{
    outputPin->setValue({ inputPin[0]->getValue(), inputPin[1]->getValue() });
    return true;
}

Serializable* Vector2fMakeNode::instantiate(const SerializationInfo& from)
{
    return new Vector2fMakeNode();
}

SerializeInstance(Vector3fMakeNode, DEF_ATTR(CodeFunctionName, Code::Vector3f_t));

Vector3fMakeNode::Vector3fMakeNode()
{
    flag = Flag::Expression;
    displayName = "MakeVector3f";
    inputPin[0] = new FloatPin("x");
    inputPin[1] = new FloatPin("y");
    inputPin[2] = new FloatPin("z");
    outputPin = new Vector3fPin("Out");
    addInput(*inputPin[0]);
    addInput(*inputPin[1]);
    addInput(*inputPin[2]);
    addOutput(*outputPin);
}

bool Vector3fMakeNode::process(GraphContext& context)
{
    outputPin->setValue({ inputPin[0]->getValue(), inputPin[1]->getValue(), inputPin[2]->getValue() });
    return true;
}

Serializable* Vector3fMakeNode::instantiate(const SerializationInfo& from)
{
    return new Vector3fMakeNode();
}

SerializeInstance(QuaternionfMakeNode, DEF_ATTR(CodeFunctionName, Code::Quaternionf_t));

QuaternionfMakeNode::QuaternionfMakeNode()
{
    flag = Flag::Expression;
    displayName = "MakeQuaternionf";
    inputPin[0] = new FloatPin("x");
    inputPin[1] = new FloatPin("y");
    inputPin[2] = new FloatPin("z");
    inputPin[3] = new FloatPin("w");
    outputPin = new QuaternionfPin("Out");
    addInput(*inputPin[0]);
    addInput(*inputPin[1]);
    addInput(*inputPin[2]);
    addInput(*inputPin[3]);
    addOutput(*outputPin);
}

bool QuaternionfMakeNode::process(GraphContext& context)
{
    outputPin->setValue(Quaternionf {
        inputPin[0]->getValue(),
        inputPin[1]->getValue(),
        inputPin[2]->getValue(),
        inputPin[3]->getValue()
    });
    return true;
}

Serializable* QuaternionfMakeNode::instantiate(const SerializationInfo& from)
{
    return new QuaternionfMakeNode();
}

SerializeInstance(Vector2fBreakNode);

Vector2fBreakNode::Vector2fBreakNode()
{
    flag = Flag::Expression;
    displayName = "BreakVector2f";
    inputPin = new Vector2fPin("In");
    outputPin[0] = new FloatPin("x");
    outputPin[1] = new FloatPin("y");
    addInput(*inputPin);
    addOutput(*outputPin[0]);
    addOutput(*outputPin[1]);
}

bool Vector2fBreakNode::process(GraphContext& context)
{
    Vector2f vec = inputPin->getValue();
    outputPin[0]->setValue(vec.x());
    outputPin[1]->setValue(vec.y());
    return true;
}

bool Vector2fBreakNode::generate(GraphCodeGenerationContext& context)
{
    bool ok = true;
    if (outputPin[0]->getConnectedPin())
        ok &= context.getBackend().invoke(
            CodeFunctionInvocation(Name(".x"))
                .param(context.getParameter(inputPin))
            .out(context.getParameter(outputPin[0]).symbol()));
    if (outputPin[1]->getConnectedPin())
        ok &= context.getBackend().invoke(
            CodeFunctionInvocation(Name(".y"))
                .param(context.getParameter(inputPin))
            .out(context.getParameter(outputPin[1]).symbol()));
    return ok;
}

Serializable* Vector2fBreakNode::instantiate(const SerializationInfo& from)
{
    return new Vector2fBreakNode();
}

SerializeInstance(Vector3fBreakNode);

Vector3fBreakNode::Vector3fBreakNode()
{
    flag = Flag::Expression;
    displayName = "BreakVector3f";
    inputPin = new Vector3fPin("In");
    outputPin[0] = new FloatPin("x");
    outputPin[1] = new FloatPin("y");
    outputPin[2] = new FloatPin("z");
    addInput(*inputPin);
    addOutput(*outputPin[0]);
    addOutput(*outputPin[1]);
    addOutput(*outputPin[2]);
}

bool Vector3fBreakNode::process(GraphContext& context)
{
    Vector3f vec = inputPin->getValue();
    outputPin[0]->setValue(vec.x());
    outputPin[1]->setValue(vec.y());
    outputPin[2]->setValue(vec.z());
    return true;
}

bool Vector3fBreakNode::generate(GraphCodeGenerationContext& context)
{
    bool ok = true;
    if (outputPin[0]->getConnectedPin())
        ok &= context.getBackend().invoke(
            CodeFunctionInvocation(Name(".x"))
                .param(context.getParameter(inputPin))
            .out(context.getParameter(outputPin[0]).symbol()));
    if (outputPin[1]->getConnectedPin())
        ok &= context.getBackend().invoke(
            CodeFunctionInvocation(Name(".y"))
                .param(context.getParameter(inputPin))
            .out(context.getParameter(outputPin[1]).symbol()));
    if (outputPin[2]->getConnectedPin())
        ok &= context.getBackend().invoke(
            CodeFunctionInvocation(Name(".z"))
                .param(context.getParameter(inputPin))
            .out(context.getParameter(outputPin[2]).symbol()));
    return ok;
}

Serializable* Vector3fBreakNode::instantiate(const SerializationInfo& from)
{
    return new Vector3fBreakNode();
}

SerializeInstance(QuaternionfBreakNode);

QuaternionfBreakNode::QuaternionfBreakNode()
{
    flag = Flag::Expression;
    displayName = "BreakQuaternionf";
    inputPin = new QuaternionfPin("In");
    outputPin[0] = new FloatPin("x");
    outputPin[1] = new FloatPin("y");
    outputPin[2] = new FloatPin("z");
    outputPin[3] = new FloatPin("w");
    addInput(*inputPin);
    addOutput(*outputPin[0]);
    addOutput(*outputPin[1]);
    addOutput(*outputPin[2]);
    addOutput(*outputPin[3]);
}

bool QuaternionfBreakNode::process(GraphContext& context)
{
    Quaternionf vec = inputPin->getValue();
    outputPin[0]->setValue(vec.x());
    outputPin[1]->setValue(vec.y());
    outputPin[2]->setValue(vec.z());
    outputPin[3]->setValue(vec.w());
    return true;
}

bool QuaternionfBreakNode::generate(GraphCodeGenerationContext& context)
{
    bool ok = true;
    if (outputPin[0]->getConnectedPin())
        ok &= context.getBackend().invoke(
            CodeFunctionInvocation(Name(".x"))
                .param(context.getParameter(inputPin))
            .out(context.getParameter(outputPin[0]).symbol()));
    if (outputPin[1]->getConnectedPin())
        ok &= context.getBackend().invoke(
            CodeFunctionInvocation(Name(".y"))
                .param(context.getParameter(inputPin))
            .out(context.getParameter(outputPin[1]).symbol()));
    if (outputPin[2]->getConnectedPin())
        ok &= context.getBackend().invoke(
            CodeFunctionInvocation(Name(".z"))
                .param(context.getParameter(inputPin))
                .out(context.getParameter(outputPin[2]).symbol()));
    if (outputPin[3]->getConnectedPin())
        ok &= context.getBackend().invoke(
            CodeFunctionInvocation(Name(".w"))
                .param(context.getParameter(inputPin))
            .out(context.getParameter(outputPin[3]).symbol()));
    return ok;
}

Serializable* QuaternionfBreakNode::instantiate(const SerializationInfo& from)
{
    return new QuaternionfBreakNode();
}

SerializeInstance(Vector2fNormalizeNode, DEF_ATTR(CodeFunctionName, "normalize"));

Vector2fNormalizeNode::Vector2fNormalizeNode()
{
    flag = Flag::Expression;
    displayName = "NormalizeVector2f";

    inputPin = new Vector2fPin("Input");
    outputPin = new Vector2fPin("Output");

    addInput(*inputPin);
    addOutput(*outputPin);
}

bool Vector2fNormalizeNode::process(GraphContext& context)
{
    outputPin->setValue(inputPin->getValue().normalize());
    return true;
}

Serializable* Vector2fNormalizeNode::instantiate(const SerializationInfo& from)
{
    return new Vector2fNormalizeNode();
}

SerializeInstance(Vector3fNormalizeNode, DEF_ATTR(CodeFunctionName, "normalize"));

Vector3fNormalizeNode::Vector3fNormalizeNode()
{
    flag = Flag::Expression;
    displayName = "NormalizeVector3f";

    inputPin = new Vector3fPin("Input");
    outputPin = new Vector3fPin("Output");

    addInput(*inputPin);
    addOutput(*outputPin);
}

bool Vector3fNormalizeNode::process(GraphContext& context)
{
    outputPin->setValue(inputPin->getValue().normalize());
    return true;
}

Serializable* Vector3fNormalizeNode::instantiate(const SerializationInfo& from)
{
    return new Vector3fNormalizeNode();
}

SerializeInstance(QuaternionfNormalizeNode, DEF_ATTR(CodeFunctionName, "normalize"));

QuaternionfNormalizeNode::QuaternionfNormalizeNode()
{
    flag = Flag::Expression;
    displayName = "NormalizeQuaternionf";

    inputPin = new QuaternionfPin("Input");
    outputPin = new QuaternionfPin("Output");

    addInput(*inputPin);
    addOutput(*outputPin);
}

bool QuaternionfNormalizeNode::process(GraphContext& context)
{
    outputPin->setValue(inputPin->getValue().normalize());
    return true;
}

Serializable* QuaternionfNormalizeNode::instantiate(const SerializationInfo& from)
{
    return new QuaternionfNormalizeNode();
}

SerializeInstance(Vector2fScaleNode);

Vector2fScaleNode::Vector2fScaleNode()
{
    flag = Flag::Expression;
    displayName = "ScaleVector2f";

    vecPin = new Vector2fPin("Vector2f");
    scalePin = new FloatPin("Scalar");
    outputPin = new Vector2fPin("Output");

    addInput(*vecPin);
    addInput(*scalePin);
    addOutput(*outputPin);
}

bool Vector2fScaleNode::process(GraphContext& context)
{
    outputPin->setValue(vecPin->getValue() * scalePin->getValue());
    return true;
}

Serializable* Vector2fScaleNode::instantiate(const SerializationInfo& from)
{
    return new Vector2fScaleNode();
}

SerializeInstance(Vector3fScaleNode);

Vector3fScaleNode::Vector3fScaleNode()
{
    flag = Flag::Expression;
    displayName = "ScaleVector3f";

    vecPin = new Vector3fPin("Vector3f");
    scalePin = new FloatPin("Scalar");
    outputPin = new Vector3fPin("Output");

    addInput(*vecPin);
    addInput(*scalePin);
    addOutput(*outputPin);
}

bool Vector3fScaleNode::process(GraphContext& context)
{
    outputPin->setValue(vecPin->getValue() * scalePin->getValue());
    return true;
}

Serializable* Vector3fScaleNode::instantiate(const SerializationInfo& from)
{
    return new Vector3fScaleNode();
}

SerializeInstance(Vector2fDotNode, DEF_ATTR(CodeFunctionName, "dot"));

Vector2fDotNode::Vector2fDotNode()
{
    flag = Flag::Expression;
    displayName = "DotVector2f";

    aPin = new Vector2fPin("A");
    bPin = new Vector2fPin("B");
    outputPin = new FloatPin("Output");

    addInput(*aPin);
    addInput(*bPin);
    addOutput(*outputPin);
}

bool Vector2fDotNode::process(GraphContext& context)
{
    outputPin->setValue(aPin->getValue().dot(bPin->getValue()));
    return true;
}

Serializable* Vector2fDotNode::instantiate(const SerializationInfo& from)
{
    return new Vector2fDotNode();
}

SerializeInstance(Vector3fDotNode, DEF_ATTR(CodeFunctionName, "dot"));

Vector3fDotNode::Vector3fDotNode()
{
    flag = Flag::Expression;
    displayName = "DotVector3f";

    aPin = new Vector3fPin("A");
    bPin = new Vector3fPin("B");
    outputPin = new FloatPin("Output");

    addInput(*aPin);
    addInput(*bPin);
    addOutput(*outputPin);
}

bool Vector3fDotNode::process(GraphContext& context)
{
    outputPin->setValue(aPin->getValue().dot(bPin->getValue()));
    return true;
}

Serializable* Vector3fDotNode::instantiate(const SerializationInfo& from)
{
    return new Vector3fDotNode();
}

SerializeInstance(Vector2fCrossNode, DEF_ATTR(CodeFunctionName, "cross"));

Vector2fCrossNode::Vector2fCrossNode()
{
    flag = Flag::Expression;
    displayName = "CrossVector2f";

    aPin = new Vector2fPin("A");
    bPin = new Vector2fPin("B");
    outputPin = new Vector2fPin("Output");

    addInput(*aPin);
    addInput(*bPin);
    addOutput(*outputPin);
}

bool Vector2fCrossNode::process(GraphContext& context)
{
    outputPin->setValue(aPin->getValue().cross(bPin->getValue()));
    return true;
}

Serializable* Vector2fCrossNode::instantiate(const SerializationInfo& from)
{
    return new Vector2fCrossNode();
}

SerializeInstance(Vector3fCrossNode, DEF_ATTR(CodeFunctionName, "cross"));

Vector3fCrossNode::Vector3fCrossNode()
{
    flag = Flag::Expression;
    displayName = "CrossVector3f";

    aPin = new Vector3fPin("A");
    bPin = new Vector3fPin("B");
    outputPin = new Vector3fPin("Output");

    addInput(*aPin);
    addInput(*bPin);
    addOutput(*outputPin);
}

bool Vector3fCrossNode::process(GraphContext& context)
{
    outputPin->setValue(aPin->getValue().cross(bPin->getValue()));
    return true;
}

Serializable* Vector3fCrossNode::instantiate(const SerializationInfo& from)
{
    return new Vector3fCrossNode();
}

SerializeInstance(QuaternionfFromTwoVectorsNode);

QuaternionfFromTwoVectorsNode::QuaternionfFromTwoVectorsNode()
{
    flag = Flag::Expression;
    displayName = "FromTwoVector";

    aPin = new Vector3fPin("A");
    bPin = new Vector3fPin("B");
    outputPin = new QuaternionfPin("Output");

    addInput(*aPin);
    addInput(*bPin);
    addOutput(*outputPin);
}

bool QuaternionfFromTwoVectorsNode::process(GraphContext& context)
{
    outputPin->setValue(Quaternionf::FromTwoVectors(aPin->getValue(), bPin->getValue()));
    return true;
}

Serializable* QuaternionfFromTwoVectorsNode::instantiate(const SerializationInfo& from)
{
    return new QuaternionfFromTwoVectorsNode();
}

SerializeInstance(EularToQuaternionfNode);

EularToQuaternionfNode::EularToQuaternionfNode()
{
    flag = Flag::Expression;
    displayName = "EularToQuaternionf";

    eularPin = new Vector3fPin("Eular");
    quatPin = new QuaternionfPin("Quaternionf");

    addInput(*eularPin);
    addOutput(*quatPin);
}

bool EularToQuaternionfNode::process(GraphContext& context)
{
    quatPin->setValue(Quaternionf::FromEularAngles(eularPin->getValue()));
    return true;
}

Serializable* EularToQuaternionfNode::instantiate(const SerializationInfo& from)
{
    return new EularToQuaternionfNode();
}

SerializeInstance(QuaternionfToEularNode);

QuaternionfToEularNode::QuaternionfToEularNode()
{
    flag = Flag::Expression;
    displayName = "QuaternionfToEular";

    quatPin = new QuaternionfPin("Quaternionf");
    eularPin = new Vector3fPin("Eular");

    addInput(*quatPin);
    addOutput(*eularPin);
}

bool QuaternionfToEularNode::process(GraphContext& context)
{
    eularPin->setValue(quatPin->getValue().toRotationMatrix().eulerAngles());
    return true;
}

Serializable* QuaternionfToEularNode::instantiate(const SerializationInfo& from)
{
    return new QuaternionfToEularNode();
}

SerializeInstance(QuaternionfFromAngleAxisNode);

QuaternionfFromAngleAxisNode::QuaternionfFromAngleAxisNode()
{
    flag = Flag::Expression;
    displayName = "QuaternionfFromAngleAxis";

    anglePin = new FloatPin("Angle");
    axisPin = new Vector3fPin("Axis");
    quatPin = new QuaternionfPin("Quaternionf");

    addInput(*anglePin);
    addInput(*axisPin);
    addOutput(*quatPin);
}

bool QuaternionfFromAngleAxisNode::process(GraphContext& context)
{
    quatPin->setValue(Quaternionf::FromAngleAxis(anglePin->getValue(), axisPin->getValue()));
    return true;
}

Serializable* QuaternionfFromAngleAxisNode::instantiate(const SerializationInfo& from)
{
    return new QuaternionfFromAngleAxisNode();
}

SerializeInstance(QuaternionfSlerpNode);

QuaternionfSlerpNode::QuaternionfSlerpNode()
{
    flag = Flag::Expression;
    displayName = "Slerp";

    aPin = new QuaternionfPin("A");
    bPin = new QuaternionfPin("B");
    weightPin = new FloatPin("Weight");
    outputPin = new QuaternionfPin("Output");

    addInput(*aPin);
    addInput(*bPin);
    addInput(*weightPin);
    addOutput(*outputPin);
}

bool QuaternionfSlerpNode::process(GraphContext& context)
{
    outputPin->setValue(aPin->getValue().slerp(weightPin->getValue(), bPin->getValue()));
    return true;
}

Serializable* QuaternionfSlerpNode::instantiate(const SerializationInfo& from)
{
    return new QuaternionfSlerpNode();
}

SerializeInstance(Vector2fProjectOnVectorNode);

Vector2fProjectOnVectorNode::Vector2fProjectOnVectorNode()
{
    flag = Flag::Expression;
    displayName = "ProjectOnVector";

    vecPin = new Vector2fPin("Vector");
    normPin = new Vector2fPin("Normal");
    outputPin = new Vector2fPin("Output");

    addInput(*vecPin);
    addInput(*normPin);
    addOutput(*outputPin);
}

bool Vector2fProjectOnVectorNode::process(GraphContext& context)
{
    Vector2f vec = vecPin->getValue();
    Vector2f norm = normPin->getValue().normalize();
    float fact = vec.dot(norm);
    outputPin->setValue(vec - norm * fact);
    return true;
}

Serializable* Vector2fProjectOnVectorNode::instantiate(const SerializationInfo& from)
{
    return new Vector2fProjectOnVectorNode();
}

SerializeInstance(Vector3fProjectOnVectorNode);

Vector3fProjectOnVectorNode::Vector3fProjectOnVectorNode()
{
    flag = Flag::Expression;
    displayName = "ProjectOnVector";

    vecPin = new Vector3fPin("Vector");
    normPin = new Vector3fPin("Normal");
    outputPin = new Vector3fPin("Output");

    addInput(*vecPin);
    addInput(*normPin);
    addOutput(*outputPin);
}

bool Vector3fProjectOnVectorNode::process(GraphContext& context)
{
    Vector3f vec = vecPin->getValue();
    Vector3f norm = normPin->getValue().normalize();
    float fact = vec.dot(norm);
    outputPin->setValue(vec - norm * fact);
    return true;
}

Serializable* Vector3fProjectOnVectorNode::instantiate(const SerializationInfo& from)
{
    return new Vector3fProjectOnVectorNode();
}

SerializeInstance(Vector3fSteppingNode);

Vector3fSteppingNode::Vector3fSteppingNode()
{
    flag = Flag::Expression;
    displayName = "Stepping";

    aPin = new Vector3fPin("A");
    bPin = new Vector3fPin("B");
    anglePin = new FloatPin("Angle");
    outputPin = new Vector3fPin("Output");

    addInput(*aPin);
    addInput(*bPin);
    addInput(*anglePin);
    addOutput(*outputPin);
}

bool Vector3fSteppingNode::process(GraphContext& context)
{
    Vector3f a = aPin->getValue().normalize();
    Vector3f b = bPin->getValue().normalize();
    Vector3f n = a.cross(b);
    float dot = a.dot(b);
    float angle = acos(dot);
    float delta = anglePin->getValue();
    delta = delta > angle ? angle : delta;
    outputPin->setValue(Quaternionf::FromAngleAxis(delta, n) * a);
    return true;
}

Serializable* Vector3fSteppingNode::instantiate(const SerializationInfo& from)
{
    return new Vector3fSteppingNode();
}

SerializeInstance(Vector3fSteppingRotationNode);

Vector3fSteppingRotationNode::Vector3fSteppingRotationNode()
{
    flag = Flag::Expression;
    displayName = "Stepping";

    aPin = new Vector3fPin("A");
    bPin = new Vector3fPin("B");
    anglePin = new FloatPin("Angle");
    outputPin = new QuaternionfPin("Output");

    addInput(*aPin);
    addInput(*bPin);
    addInput(*anglePin);
    addOutput(*outputPin);
}

bool Vector3fSteppingRotationNode::process(GraphContext& context)
{
    Vector3f a = aPin->getValue().normalize();
    Vector3f b = bPin->getValue().normalize();
    Vector3f n = a.cross(b);
    float dot = a.dot(b);
    float angle = acos(dot);
    float delta = anglePin->getValue();
    delta = delta > angle ? angle : delta;
    outputPin->setValue(Quaternionf::FromAngleAxis(delta, n));
    return true;
}

Serializable* Vector3fSteppingRotationNode::instantiate(const SerializationInfo& from)
{
    return new Vector3fSteppingRotationNode();
}

SerializeInstance(AngleToRadianNode);

AngleToRadianNode::AngleToRadianNode()
{
    flag = Flag::Expression;
    displayName = "AngleToRadian";

    anglePin = new FloatPin("Angle");
    radianPin = new FloatPin("Radian");

    addInput(*anglePin);
    addOutput(*radianPin);
}

bool AngleToRadianNode::process(GraphContext& context)
{
    radianPin->setValue(anglePin->getValue() / 180.0f * PI);
    return true;
}

Serializable* AngleToRadianNode::instantiate(const SerializationInfo& from)
{
    return new AngleToRadianNode();
}

SerializeInstance(RadianToAngleNode);

RadianToAngleNode::RadianToAngleNode()
{
    flag = Flag::Expression;
    displayName = "RadianToAngle";

    radianPin = new FloatPin("Radian");
    anglePin = new FloatPin("Angle");

    addInput(*radianPin);
    addOutput(*anglePin);
}

bool RadianToAngleNode::process(GraphContext& context)
{
    anglePin->setValue(radianPin->getValue() / PI * 180.0f);
    return true;
}

Serializable* RadianToAngleNode::instantiate(const SerializationInfo& from)
{
    return new RadianToAngleNode();
}

SerializeInstance(GetAngleFromTwoVector2fNode);

GetAngleFromTwoVector2fNode::GetAngleFromTwoVector2fNode()
{
    flag = Flag::Expression;
    displayName = "GetAngleFromTwoVector";

    aPin = new Vector2fPin("A");
    bPin = new Vector2fPin("B");
    anglePin = new FloatPin("Angle");

    addInput(*aPin);
    addInput(*bPin);
    addOutput(*anglePin);
}

bool GetAngleFromTwoVector2fNode::process(GraphContext& context)
{
    float radian = acos(aPin->getValue().normalize().dot(bPin->getValue().normalize()));
    anglePin->setValue(radian / PI * 180);
    return true;
}

Serializable* GetAngleFromTwoVector2fNode::instantiate(const SerializationInfo& from)
{
    return new GetAngleFromTwoVector2fNode();
}

SerializeInstance(GetAngleFromTwoVector3fNode);

GetAngleFromTwoVector3fNode::GetAngleFromTwoVector3fNode()
{
    flag = Flag::Expression;
    displayName = "GetAngleFromTwoVector";

    aPin = new Vector3fPin("A");
    bPin = new Vector3fPin("B");
    anglePin = new FloatPin("Angle");

    addInput(*aPin);
    addInput(*bPin);
    addOutput(*anglePin);
}

bool GetAngleFromTwoVector3fNode::process(GraphContext& context)
{
    float radian = acos(aPin->getValue().normalize().dot(bPin->getValue().normalize()));
    anglePin->setValue(radian / PI * 180);
    return true;
}

Serializable* GetAngleFromTwoVector3fNode::instantiate(const SerializationInfo& from)
{
    return new GetAngleFromTwoVector3fNode();
}

SerializeInstance(GetAngleFromToVector3fNode);

GetAngleFromToVector3fNode::GetAngleFromToVector3fNode()
{
    flag = Flag::Expression;
    displayName = "GetAngleFromToVector";

    aPin = new Vector3fPin("A");
    bPin = new Vector3fPin("B");
    refAxisPin = new Vector3fPin("RefAxis");
    anglePin = new FloatPin("Angle");

    addInput(*aPin);
    addInput(*bPin);
    addInput(*refAxisPin);
    addOutput(*anglePin);
}

bool GetAngleFromToVector3fNode::process(GraphContext& context)
{
    Vector3f a = aPin->getValue().normalize();
    Vector3f b = bPin->getValue().normalize();
    Vector3f refAxis = refAxisPin->getValue();
    Vector3f axis = a.cross(b);
    float sign = axis.dot(refAxis);
    sign = sign == 0 ? 0 : (sign > 0 ? 1 : -1);
    float radian = acos(a.dot(b)) * sign;
    anglePin->setValue(radian / PI * 180);
    return true;
}

Serializable* GetAngleFromToVector3fNode::instantiate(const SerializationInfo& from)
{
    return new GetAngleFromToVector3fNode();
}
