#pragma once

#include "Variable.h"
#include "GraphBaseEditor.h"
#include "../GUI/GUIUtility.h"

class GraphVariableEditor : public GraphBaseEditor
{
public:
	GraphVariableEditor() = default;
	virtual ~GraphVariableEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onInspectGUI(EditorInfo & info);
protected:
	GraphVariable* variable = NULL;
};

#define DEC_VAR_EDITOR(BaseType, VarType, Statement)						\
class VarType##Editor : public GraphVariableEditor							\
{																			\
public:																		\
	VarType##Editor() = default;											\
	virtual ~VarType##Editor() = default;									\
																			\
	EditorInfoMethod();														\
	virtual void setInspectedObject(void* object)							\
	{																		\
		valueVariable = dynamic_cast<VarType*>((Base*)object);				\
		GraphBaseEditor::setInspectedObject(valueVariable);					\
	}																		\
																			\
	virtual void onInspectGUI(EditorInfo& info)								\
	{																		\
		BaseType value = valueVariable->getValue();							\
		if (Statement(valueVariable->getName().c_str(), &value)) {			\
			valueVariable->setValue(value);									\
		}																	\
		BaseType defaultvalue = valueVariable->getDefaultValue();			\
		ImGui::PushID("Default");											\
		ImGui::Text("Default");												\
		if (Statement(valueVariable->getName().c_str(), &defaultvalue)) {	\
			valueVariable->setDefaultValue(defaultvalue);					\
		}																	\
		ImGui::PopID();														\
		if (ImGui::Button("ResetToDefault")) {								\
			valueVariable->resetToDefault();								\
		}																	\
	}																		\
protected:																	\
	VarType* valueVariable = NULL;								   			\
};

#define DEC_OBJECT_VAR_EDITOR(BaseType, VarType, Statement)			\
class VarType##Editor : public GraphVariableEditor					\
{																	\
public:																\
	VarType##Editor() = default;									\
	virtual ~VarType##Editor() = default;							\
																	\
	EditorInfoMethod();												\
	virtual void setInspectedObject(void* object)					\
	{																\
		valueVariable = dynamic_cast<VarType*>((Base*)object);		\
		GraphBaseEditor::setInspectedObject(valueVariable);			\
	}																\
																	\
	virtual void onInspectGUI(EditorInfo& info)						\
	{																\
		BaseType value = valueVariable->getValue();					\
		if (Statement) {											\
			valueVariable->setValue(value);							\
		}															\
		value = valueVariable->getDefaultValue();					\
		ImGui::PushID("Default");									\
		ImGui::Text("Default");										\
		if (Statement) {											\
			valueVariable->setDefaultValue(value);					\
		}															\
		ImGui::PopID();												\
		if (ImGui::Button("ResetToDefault")) {						\
			valueVariable->resetToDefault();						\
		}															\
	}																\
protected:															\
	VarType* valueVariable = NULL;								   	\
};

#define IMP_VAR_EDITOR(VarType) RegistEditor(VarType);

DEC_OBJECT_VAR_EDITOR(float, FloatVariable, ImGui::DragFloat(valueVariable->getName().c_str(), &value, 0.01f));
DEC_VAR_EDITOR(int, IntVariable, ImGui::DragInt);
DEC_VAR_EDITOR(bool, BoolVariable, ImGui::Checkbox);
DEC_VAR_EDITOR(string, StringVariable, ImGui::InputText);

DEC_OBJECT_VAR_EDITOR(Vector2f, Vector2fVariable, ImGui::DragFloat2(valueVariable->getName().c_str(), (float*)&value));
DEC_OBJECT_VAR_EDITOR(Vector3f, Vector3fVariable, ImGui::DragFloat3(valueVariable->getName().c_str(), (float*)&value));
DEC_OBJECT_VAR_EDITOR(Quaternionf, QuaternionfVariable, ImGui::DragFloat4(valueVariable->getName().c_str(), (float*)&value));
DEC_OBJECT_VAR_EDITOR(Color, ColorVariable, ImGui::ColorEdit4(valueVariable->getName().c_str(), (float*)&value));