#include "AnimationStateEditor.h"

RegistEditor(AnimationTransition);

void AnimationTransitionEditor::setInspectedObject(void* object)
{
	animationTransition = dynamic_cast<AnimationTransition*>((Base*)object);
	StateGraphTransitionEditor::setInspectedObject(animationTransition);
}

void AnimationTransitionEditor::onContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	float duration = animationTransition->getDuration();
	if (ImGui::DragFloat("Duration", &duration, 0.01)) {
		animationTransition->setDuration(duration);
	}
}

RegistEditor(AnimationState);

void AnimationStateEditor::setInspectedObject(void* object)
{
	animationState = dynamic_cast<AnimationState*>((Base*)object);
	StateNodeEditor::setInspectedObject(animationState);
}

void AnimationStateEditor::onDoubleClicked(EditorInfo& info, GraphInfo& graphInfo)
{
	if (animationState)
		graphInfo.openGraph(animationState->getGraph());
}
