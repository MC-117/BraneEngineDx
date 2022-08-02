#include "AnimationBaseEditor.h"

RegistEditor(AnimationBase);

void AnimationBaseEditor::setInspectedObject(void* object)
{
	animationBase = (AnimationBase*)object;
}

void AnimationBaseEditor::onGUI(EditorInfo& info)
{
}
