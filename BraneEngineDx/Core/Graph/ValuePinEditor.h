#pragma once

#include "ValuePin.h"
#include "GraphPinEditor.h"
#include "../GUI/GUIUtility.h"

#define DEC_VALUE_PIN_EDITOR(BaseType, PinType, Statement)				\
class PinType##Editor : public GraphPinEditor							\
{																		\
public:																	\
	PinType##Editor() = default;										\
	virtual ~PinType##Editor() = default;								\
																		\
	EditorInfoMethod();													\
	virtual void setInspectedObject(void* object)						\
	{																	\
		valuePin = dynamic_cast<PinType*>((Base*)object);				\
		GraphPinEditor::setInspectedObject(valuePin);					\
	}																	\
																		\
	virtual void onInspectGUI(EditorInfo& info)							\
	{																	\
		BaseType value = valuePin->getValue();							\
		if (Statement(valuePin->getDisplayName().c_str(), &value)) {	\
			valuePin->setValue(value);									\
		}																\
	}																	\
																		\
	virtual void onPinGUI(EditorInfo& info, GraphInfo& graphInfo)		\
	{																	\
		if (valuePin->getConnectedPin())								\
			GraphPinEditor::onPinGUI(info, graphInfo);					\
		else {															\
			BaseType value = valuePin->getValue();						\
			ImGui::SetNextItemWidth(100);								\
			if (Statement(valuePin->getDisplayName().c_str(), &value))	\
				valuePin->setValue(value);								\
		}																\
	}																	\
protected:																\
	PinType* valuePin = NULL;											\
};

#define DEC_OBJECT_VALUE_PIN_EDITOR(BaseType, PinType, Statement)		\
class PinType##Editor : public GraphPinEditor							\
{																		\
public:																	\
	PinType##Editor() = default;										\
	virtual ~PinType##Editor() = default;								\
																		\
	EditorInfoMethod();													\
	virtual void setInspectedObject(void* object)						\
	{																	\
		valuePin = dynamic_cast<PinType*>((Base*)object);				\
		GraphPinEditor::setInspectedObject(valuePin);					\
	}																	\
																		\
	virtual void onInspectGUI(EditorInfo& info)							\
	{																	\
		BaseType value = valuePin->getValue();							\
		if (Statement) {												\
			valuePin->setValue(value);									\
		}																\
	}																	\
																		\
	virtual void onPinGUI(EditorInfo& info, GraphInfo& graphInfo)		\
	{																	\
		if (valuePin->getConnectedPin())								\
			GraphPinEditor::onPinGUI(info, graphInfo);					\
		else {															\
			BaseType value = valuePin->getValue();						\
			ImGui::SetNextItemWidth(100);								\
			if (Statement)												\
				valuePin->setValue(value);								\
		}																\
	}																	\
protected:																\
	PinType* valuePin = NULL;											\
};

#define IMP_VALUE_PIN_EDITOR(EditorClass) RegistEditor(EditorClass);

DEC_OBJECT_VALUE_PIN_EDITOR(float, FloatPin, ImGui::DragFloat(valuePin->getDisplayName().c_str(), &value, 0.01f));
DEC_VALUE_PIN_EDITOR(int, IntPin, ImGui::DragInt);
DEC_VALUE_PIN_EDITOR(bool, BoolPin, ImGui::Checkbox);
DEC_OBJECT_VALUE_PIN_EDITOR(Vector2f, Vector2fPin, ImGui::DragFloat2(valuePin->getDisplayName().c_str(), value.data()));
DEC_OBJECT_VALUE_PIN_EDITOR(Vector3f, Vector3fPin, ImGui::DragFloat3(valuePin->getDisplayName().c_str(), value.data()));
DEC_OBJECT_VALUE_PIN_EDITOR(Quaternionf, QuaternionfPin, ImGui::DragFloat4(valuePin->getDisplayName().c_str(), value.data()));
DEC_VALUE_PIN_EDITOR(string, StringPin, ImGui::InputText);

class KeyCodePinEditor : public GraphPinEditor
{
public:
	KeyCodePinEditor() = default;
	virtual ~KeyCodePinEditor() = default;

	EditorInfoMethod();
	virtual void setInspectedObject(void* object);

	virtual void onInspectGUI(EditorInfo& info);
	virtual void onPinGUI(EditorInfo& info, GraphInfo& graphInfo);
	virtual void onContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo);
protected:
	KeyCodePin* valuePin = NULL;
};