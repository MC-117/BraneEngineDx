#include "InputNode.h"
#include "../Engine.h"

SerializeInstance(GetMousePositionNode);

GetMousePositionNode::GetMousePositionNode()
{
    flag = Flag::Expression;
    displayName = "GetMousePosition";
    valuePin = new Vector2fPin("Position");
    addOutput(*valuePin);
}

bool GetMousePositionNode::process(GraphContext& context)
{
    Unit2Di pos = Engine::getInput().getMousePos();
    valuePin->setValue({ (float)pos.x, (float)pos.y });
    return true;
}

Serializable* GetMousePositionNode::instantiate(const SerializationInfo& from)
{
    return new GetMousePositionNode();
}

SerializeInstance(GetMouseMoveNode);

GetMouseMoveNode::GetMouseMoveNode()
{
    flag = Flag::Expression;
    displayName = "GetMouseMove";
    valuePin = new Vector2fPin("Move");
    addOutput(*valuePin);
}

bool GetMouseMoveNode::process(GraphContext& context)
{
    Unit2Di move = Engine::getInput().getMouseMove();
    valuePin->setValue({ (float)move.x, (float)move.y });
    return true;
}

Serializable* GetMouseMoveNode::instantiate(const SerializationInfo& from)
{
    return new GetMouseMoveNode();
}

SerializeInstance(GetMouseDownNode);

GetMouseDownNode::GetMouseDownNode()
{
    flag = Flag::Expression;
    displayName = "GetMouseDown";
    indexPin = new IntPin("Index");
    valuePin = new BoolPin("Value");
    addInput(*indexPin);
    addOutput(*valuePin);
}

bool GetMouseDownNode::process(GraphContext& context)
{
    valuePin->setValue(Engine::getInput().getMouseButtonDown((MouseButtonEnum)indexPin->getValue()));
    return true;
}

Serializable* GetMouseDownNode::instantiate(const SerializationInfo& from)
{
    return new GetMouseDownNode();
}

SerializeInstance(GetMouseUpNode);

GetMouseUpNode::GetMouseUpNode()
{
    flag = Flag::Expression;
    displayName = "GetMouseUp";
    indexPin = new IntPin("Index");
    valuePin = new BoolPin("Value");
    addInput(*indexPin);
    addOutput(*valuePin);
}

bool GetMouseUpNode::process(GraphContext& context)
{
    valuePin->setValue(Engine::getInput().getMouseButtonUp((MouseButtonEnum)indexPin->getValue()));
    return true;
}

Serializable* GetMouseUpNode::instantiate(const SerializationInfo& from)
{
    return new GetMouseUpNode();
}

SerializeInstance(GetMousePressNode);

GetMousePressNode::GetMousePressNode()
{
    flag = Flag::Expression;
    displayName = "GetMousePress";
    indexPin = new IntPin("Index");
    valuePin = new BoolPin("Value");
    addInput(*indexPin);
    addOutput(*valuePin);
}

bool GetMousePressNode::process(GraphContext& context)
{
    valuePin->setValue(Engine::getInput().getMouseButtonPress((MouseButtonEnum)indexPin->getValue()));
    return true;
}

Serializable* GetMousePressNode::instantiate(const SerializationInfo& from)
{
    return new GetMousePressNode();
}

SerializeInstance(GetMouseReleaseNode);

GetMouseReleaseNode::GetMouseReleaseNode()
{
    flag = Flag::Expression;
    displayName = "GetMouseRelease";
    indexPin = new IntPin("Index");
    valuePin = new BoolPin("Value");
    addInput(*indexPin);
    addOutput(*valuePin);
}

bool GetMouseReleaseNode::process(GraphContext& context)
{
    valuePin->setValue(Engine::getInput().getMouseButtonRelease((MouseButtonEnum)indexPin->getValue()));
    return true;
}

Serializable* GetMouseReleaseNode::instantiate(const SerializationInfo& from)
{
    return new GetMouseReleaseNode();
}

SerializeInstance(GetKeyDownNode);

GetKeyDownNode::GetKeyDownNode()
{
    flag = Flag::Expression;
    displayName = "GetKeyDown";
    keyCodePin = new KeyCodePin("Key");
    valuePin = new BoolPin("Value");
    addInput(*keyCodePin);
    addOutput(*valuePin);
}

bool GetKeyDownNode::process(GraphContext& context)
{
    valuePin->setValue(Engine::getInput().getKeyDown(keyCodePin->getValue()));
    return true;
}

Serializable* GetKeyDownNode::instantiate(const SerializationInfo& from)
{
    return new GetKeyDownNode;
}

SerializeInstance(GetKeyUpNode);

GetKeyUpNode::GetKeyUpNode()
{
    flag = Flag::Expression;
    displayName = "GetKeyUp";
    keyCodePin = new KeyCodePin("Key");
    valuePin = new BoolPin("Value");
    addInput(*keyCodePin);
    addOutput(*valuePin);
}

bool GetKeyUpNode::process(GraphContext& context)
{
    valuePin->setValue(Engine::getInput().getKeyUp(keyCodePin->getValue()));
    return true;
}

Serializable* GetKeyUpNode::instantiate(const SerializationInfo& from)
{
    return new GetKeyUpNode;
}

SerializeInstance(GetKeyPressNode);

GetKeyPressNode::GetKeyPressNode()
{
    flag = Flag::Expression;
    displayName = "GetKeyPress";
    keyCodePin = new KeyCodePin("Key");
    valuePin = new BoolPin("Value");
    addInput(*keyCodePin);
    addOutput(*valuePin);
}

bool GetKeyPressNode::process(GraphContext& context)
{
    valuePin->setValue(Engine::getInput().getKeyPress(keyCodePin->getValue()));
    return true;
}

Serializable* GetKeyPressNode::instantiate(const SerializationInfo& from)
{
    return new GetKeyPressNode();
}

SerializeInstance(GetKeyReleaseNode);

GetKeyReleaseNode::GetKeyReleaseNode()
{
    flag = Flag::Expression;
    displayName = "GetKeyRelease";
    keyCodePin = new KeyCodePin("Key");
    valuePin = new BoolPin("Value");
    addInput(*keyCodePin);
    addOutput(*valuePin);
}

bool GetKeyReleaseNode::process(GraphContext& context)
{
    valuePin->setValue(Engine::getInput().getKeyRelease(keyCodePin->getValue()));
    return true;
}

Serializable* GetKeyReleaseNode::instantiate(const SerializationInfo& from)
{
    return new GetKeyReleaseNode();
}
