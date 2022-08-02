#include "ValuePinEditor.h"
#include "../Engine.h"

IMP_VALUE_PIN_EDITOR(FloatPin);
IMP_VALUE_PIN_EDITOR(IntPin);
IMP_VALUE_PIN_EDITOR(BoolPin);
IMP_VALUE_PIN_EDITOR(Vector2fPin);
IMP_VALUE_PIN_EDITOR(Vector3fPin);
IMP_VALUE_PIN_EDITOR(QuaternionfPin);
IMP_VALUE_PIN_EDITOR(StringPin);

IMP_VALUE_PIN_EDITOR(KeyCodePin);

void KeyCodePinEditor::setInspectedObject(void* object)
{
	valuePin = dynamic_cast<KeyCodePin*>((Base*)object);
	GraphPinEditor::setInspectedObject(valuePin);
}

void KeyCodePinEditor::onInspectGUI(EditorInfo& info)
{
	char value = valuePin->getValue();
	string keyName = Engine::input.getKeyName(value);
	if (ImGui::BeginCombo(valuePin->getDisplayName().c_str(), keyName.c_str())) {
		for (int i = 0; i < 128; i++) {
			string keyName = Engine::input.getKeyName(i);
			bool selected = value == i;
			ImGui::PushID(i);
			if (ImGui::Selectable(keyName.c_str(), &selected)) {
				valuePin->setValue(i);
			}
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}
}

void KeyCodePinEditor::onPinGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	if (valuePin->getConnectedPin())
		GraphPinEditor::onPinGUI(info, graphInfo);
	else {
		char value = valuePin->getValue();
		ImGui::TextUnformatted(Engine::input.getKeyName(value).c_str());
	}
}

void KeyCodePinEditor::onContextMenuGUI(EditorInfo& info, GraphInfo& graphInfo)
{
	char value = valuePin->getValue();
	for (int i = 0; i < 128; i++) {
		string keyName = Engine::input.getKeyName(i);
		bool selected = value == i;
		ImGui::PushID(i);
		if (ImGui::Selectable(keyName.c_str(), &selected)) {
			valuePin->setValue(i);
		}
		ImGui::PopID();
	}
}
