#include "CharacterNode.h"
#include "../../Character.h"

IMP_REF_PIN(Character, CharacterRefPin, Color(218, 0, 183));
IMP_REF_VAR_CLASS(Character, CharacterRefVariable, CharacterRefPin, Color(218, 0, 183));

SerializeInstance(CharacterIsFlyNode);

CharacterIsFlyNode::CharacterIsFlyNode()
{
    flag = Flag::Expression;
    displayName = "IsFly";
    characterPin = new CharacterRefPin("Character");
    isFlyPin = new BoolPin("IsFly");
    addInput(*characterPin);
    addOutput(*isFlyPin);
}

bool CharacterIsFlyNode::process(GraphContext& context)
{
    Character* character = characterPin->getRef();
    bool isFly = false;
    if (character)
        isFly = character->isFly();
    isFlyPin->setValue(isFly);
    return true;
}

Serializable* CharacterIsFlyNode::instantiate(const SerializationInfo& from)
{
    return new CharacterIsFlyNode();
}

SerializeInstance(CharacterMoveNode);

CharacterMoveNode::CharacterMoveNode()
{
    displayName = "CharacterMove";
    characterPin = new CharacterRefPin("Character");
    movePin = new Vector3fPin("MoveVector");
    addInput(*characterPin);
    addInput(*movePin);
}

bool CharacterMoveNode::process(GraphContext& context)
{
    Character* character = characterPin->getRef();
    if (character)
        character->move(movePin->getValue());
    return true;
}

Serializable* CharacterMoveNode::instantiate(const SerializationInfo& from)
{
    return new CharacterMoveNode();
}

SerializeInstance(CharacterJumpNode);

CharacterJumpNode::CharacterJumpNode()
{
    displayName = "CharacterJump";
    characterPin = new CharacterRefPin("Character");
    impulsePin = new FloatPin("Impulse");
    addInput(*characterPin);
    addInput(*impulsePin);
}

bool CharacterJumpNode::process(GraphContext& context)
{
    Character* character = characterPin->getRef();
    if (character)
        character->jump(impulsePin->getValue());
    return true;
}

Serializable* CharacterJumpNode::instantiate(const SerializationInfo& from)
{
    return new CharacterJumpNode();
}
