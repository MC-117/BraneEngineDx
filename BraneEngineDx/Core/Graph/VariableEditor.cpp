#include "VariableEditor.h"

RegistEditor(GraphVariable);

void GraphVariableEditor::setInspectedObject(void* object)
{
	variable = dynamic_cast<GraphVariable*>((Base*)object);
	GraphBaseEditor::setInspectedObject(variable);
}

void GraphVariableEditor::onInspectGUI(EditorInfo& info)
{
}

IMP_VAR_EDITOR(FloatVariable);
IMP_VAR_EDITOR(IntVariable);
IMP_VAR_EDITOR(BoolVariable);
IMP_VAR_EDITOR(StringVariable);

IMP_VAR_EDITOR(Vector2fVariable);
IMP_VAR_EDITOR(Vector3fVariable);
IMP_VAR_EDITOR(QuaternionfVariable);
IMP_VAR_EDITOR(ColorVariable);
