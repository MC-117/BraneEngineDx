#include "ValueNode.h"

SerializeInstance(CastToNode);

CastToNode::CastToNode()
{
    flag = Flag::Expression;
    displayName = "CastTo";
}

ValuePin* CastToNode::getFromPin() const
{
    return fromPin;
}

ValuePin* CastToNode::getToPin() const
{
    return toPin;
}

void CastToNode::setFromPin(ValuePin* pin)
{
    if (fromPin) {
        delete fromPin;
        inputs.clear();
    }
    fromPin = pin;
    if (pin) {
        pin->setName("In");
        addInput(*pin);
    }
}

void CastToNode::setToPin(ValuePin* pin)
{
    if (toPin) {
        delete toPin;
        outputs.clear();
    }
    toPin = pin;
    if (pin) {
        pin->setName("Out");
        addOutput(*pin);
    }
}

bool CastToNode::process(GraphContext& context)
{
    if (fromPin == NULL || toPin == NULL)
        return false;
    toPin->castFrom(fromPin);
    return true;
}

bool CastToNode::generate(GraphCodeGenerationContext& context)
{
    return ValueCasterManager::get().generate(context, fromPin, toPin);
}

Serializable* CastToNode::instantiate(const SerializationInfo& from)
{
    CastToNode* node = new CastToNode();
    const SerializationInfo* inputInfos = from.get("inputs");
    if (inputInfos) {
        const SerializationInfo* valuePinInfo = inputInfos->get("In");
        if (valuePinInfo) {
            Serializable* serializable = valuePinInfo->serialization->instantiate(*valuePinInfo);
            if (serializable) {
                ValuePin* pin = dynamic_cast<ValuePin*>(serializable);
                if (!pin) {
                    delete serializable;
                }
                node->setFromPin(pin);
            }
        }
    }
    const SerializationInfo* outputInfos = from.get("outputs");
    if (outputInfos) {
        const SerializationInfo* valuePinInfo = outputInfos->get("Out");
        if (valuePinInfo) {
            Serializable* serializable = valuePinInfo->serialization->instantiate(*valuePinInfo);
            if (serializable) {
                ValuePin* pin = dynamic_cast<ValuePin*>(serializable);
                if (!pin) {
                    delete serializable;
                }
                node->setToPin(pin);
            }
        }
    }
    return node;
}
